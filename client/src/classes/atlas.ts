import { ref } from "vue"
import { DeviceInfo } from "./device"
import { AtlasParams } from "./parameter"
import { AtlasService } from "./service"
import { SPProfileManager } from "./sp-profile"
import { AtlasResult } from "./result"

// サービス
const ATLAS_SERVICE = "32150000-9a86-43ac-b15f-200ed1b7a72a"

//-----------------------------------------------------------------------------
// インスタンス for Singleton
//-----------------------------------------------------------------------------
// Bluetooth制御
const device  = ref<BluetoothDevice | null>(null)
const service = ref<AtlasService | null>(null)

// デバイス制御
const devInfo = ref(new DeviceInfo)
const params = ref(new AtlasParams)
const result = ref(new AtlasResult)
const profManager = ref(new SPProfileManager)

// フラグ
const isConnected = ref(false)
const isGattBusy = ref(false)

//-----------------------------------------------------------------------------
// コンポーザブル
//-----------------------------------------------------------------------------
export function useATLAS() {
    // 生データ
    let rawData: Uint8Array | null = null
    let rawDataOffset = 0
    let rawDataSize = 0
    let rawDataProcessing = false

    // 排他制御
    async function runExclusive<T>(
        task: () => Promise<T>
    ): Promise<T> {
        if (isGattBusy.value) {
            throw new Error('BLE GATT busy')
        }

        isGattBusy.value = true
        try {
            return await task()
        }
        finally {
            isGattBusy.value = false
        }
    }

    // デバイスの要求
    async function requestDevice() {
        return runExclusive(async () => {
            // スキャン実行
            console.log("finding device...")
            device.value = await navigator.bluetooth.requestDevice({
                filters: [{
                    // サービス名は固定
                    services: [ATLAS_SERVICE],
                }]
            })
            console.log("finding device...done")

            // デバイスが切断されたときのコールバックを登録
            device.value.addEventListener(
                'gattserverdisconnected',
                () => {
                    console.log(`device is disconnected.`);
                    isConnected.value = false
                    service.value = null
                    devInfo.value.format = 1
                }
            );
        })
    }

    // 接続
    async function connect() {
        return runExclusive(async () => {
            if (!device.value?.gatt) {
                throw new Error('GATT not available')
            }

            // 接続実行
            console.log("connecting...")
            const server = await device.value.gatt?.connect()
            service.value = new AtlasService(
                await server.getPrimaryService(ATLAS_SERVICE)
            )
            isConnected.value = true
            console.log("connecting...done")
        })
    }

    // 切断
    function disconnect() {
        return runExclusive(async () => {
            if (device.value?.gatt?.connected) {
                console.log("disconnecting...")
                device.value.gatt.disconnect()
                console.log("disconnecting...done")
            }
        })
    }

    // デバイス情報の取得
    async function readDeviceInfo() {
        return runExclusive(async () => {
            if (!service.value) return

            console.log("reading device info...")

            // 1.2.0 以降
            const data = await service.value.readDeviceInfo()
            if (data) {
                devInfo.value.deserialize(data)
                console.log(` --> version ${devInfo.value.versionStr()}`)
            }
            // 1.0.x
            else if (await service.value.isUnder110()) {
                // 何もしない
                console.log(" --> version 1.0.0")
            }
            // 1.1.0
            else {
                const data = await service.value.readParams()
                if (data) {
                    devInfo.value.deserialize11x(data)
                }
                console.log(" --> version 1.1.0")
            }
        })
    }

    // パラメータの取得
    async function readParams() {
        return runExclusive(async () => {
            if (!service.value) return

            console.log("reading params...")
            const data = await service.value.readParams()
            if (!data) return

            // 1.x
            if (devInfo.value.version < 0x2000) {
                params.value.deserialize1x(data)
            }
            // 2.0.0 以降
            else {
                params.value.deserialize(data)
            }
        })
    }

    // パラメータの書き込み
    async function writeParams() {
        return runExclusive(async () => {
            if (!service.value) return

            console.log("writing params...")
            await service.value.writeParams(
                new Uint8Array(
                    devInfo.value.version < 0x2000
                        ? params.value.serialize1x()  // 1.x
                        : params.value.serialize()    // 2.0.0 以降
                )
            )

            // Ver.1.0.0のみ本体ROMへパラメータを記録させる
            if (devInfo.value.version === 0x1000) {
                console.log("storing params in flash memory...")
                await service.value.writeRom()
            }
        })
    }

    // 解析結果の読み込み
    async function readResult() {
        return runExclusive(async () => {
            if (!service.value) return

            // 1.x
            if (devInfo.value.version < 0x2000) {
                // ヘッダ情報の取得
                console.log("reading basic statistics...")
                const dataHeader = await service.value.readHeader()
                if (!dataHeader) return

                // ヒストグラムデータの取得
                const dataHists = new Array<DataView<ArrayBufferLike>>(3)
                for (let i = 0; i < 3; i += 1) {
                    console.log(`reading shoot-power distribution...${i+1}/3`)
                    const data = await service.value.readResult()
                    if (!data) return
                        
                    dataHists[i] = data
                }

                // デシリアライズ
                result.value.deserialize1x(dataHeader, dataHists)
            }
            // 2.0.0 以降
            else {
                console.log("reading statistics...")
                const data = await service.value.readResult()
                if (!data) return

                result.value.deserialize(data)
            }
        })
    }

    // 統計データの初期化
    async function clearResult() {
        return runExclusive(async () => {
            if (!service.value) return

            console.log("clearing statistics data...")

            // 1.x
            if (devInfo.value.version < 0x2000) {
                await service.value.clearResult1x()
            }
            // 2.0.0 以降
            else {
                await service.value.clearResult()
            }
        })
    }

    // 生データの取得
    async function readRawData() {
        return runExclusive(async () => {
            if (!service.value) return
            console.log("starting raw data notification...")

            // パケット数の取得
            const data = await service.value.readRawDataSize()
            if (!data) return
            rawDataSize = data.getUint32(0, true)

            // バッファの確保
            rawData = new Uint8Array(rawDataSize)
            rawDataOffset = 0

            // 購読開始
            const success = await service.value.subscribeRawData(onNotifyRawData)
            if (!success) return

            // データ転送依頼
            await service.value.startReceiveRawData()
        })
    }

    // 生データが来たときのイベントハンドラ
    async function onNotifyRawData(evt: Event) {
        if (!rawData || rawDataProcessing) return

        rawDataProcessing = true

        try {
            // データの取得
            const dv = (evt.target as BluetoothRemoteGATTCharacteristic).value
            if (!dv) return

            // ペイロード
            const data = new Uint8Array(dv.buffer)

            // データの格納
            rawData.set(data, rawDataOffset)
            rawDataOffset += data.length

            // 読み込み完了
            if (rawDataOffset >= rawDataSize) {
                // 購読の解除
                await service.value!.unsubscribeRawData(onNotifyRawData)
                console.log("raw data received")
                profManager.value.set(new Uint16Array(rawData.buffer))
            }
        }
        finally {
            rawDataProcessing = false
        }
    }

    // マニュアル射出
    async function launchManually() {
        return runExclusive(async () => {
            if (!service.value) return

            console.log("launching beyblade...")
            await service.value.launchManually()
        })
    }

    // オートモードへの切り替え（スイッチレスデバイスのみ）
    async function switchToAutoMode() {
        return runExclusive(async () => {
            if (!service.value) return

            console.log("switching to auto mode...")
            await service.value.switchToAutoMode()
        })
    }

    return {
        // データ
        devInfo,
        params,
        result,
        profManager,
        // フラグ
        isConnected,
        isGattBusy,
        // 操作
        requestDevice,
        connect,
        disconnect,
        readDeviceInfo,
        readParams,
        writeParams,
        readResult,
        readRawData,
        clearResult,
        launchManually,
        switchToAutoMode
    }
}

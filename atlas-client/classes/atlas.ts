import { ref } from "vue"
import { DeviceInfo } from "./device"
import { AtlasParams } from "./parameter"
import { AtlasStats } from "./stats"
import { AtlasService } from "./service"
import { Histogram } from "./histogram"

// サービス
const ATLAS_SERVICE = "32150000-9a86-43ac-b15f-200ed1b7a72a"

export function useBLE() {
    // Bluetooth制御
    const device  = ref<BluetoothDevice | null>(null)
    const service = ref<AtlasService | null>(null)

    // デバイス制御
    const devInfo = ref(new DeviceInfo)
    const params = ref(new AtlasParams)
    const stats = ref(new AtlasStats)
    const histogram = ref(new Histogram)

    // フラグ
    const isConnected = ref(false)
    const isGattBusy = ref(false)

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
            const server = await device.value?.gatt?.connect()
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

            // 1.2.0以降
            const data = await service.value.readDeviceInfo()
            if (data) {
                console.log(" --> version 1.2.x or more")
                devInfo.value.deserialize(data)
            }
            // 1.0.x
            else if (await service.value.isUnder110()) {
                // 何もしない
                console.log(" --> version 1.0.x")
            }
            // 1.1.x
            else {
                console.log(" --> version 1.1.x")
                const data = await service.value.readParams()
                if (data) {
                    devInfo.value.deserialize11x(data)
                }
            }
        })
    }

    // パラメータの取得
    async function readParams() {
        return runExclusive(async () => {
            if (!service.value) return

            console.log("reading params...")
            const data = await service.value.readParams()
            if (data) {
                params.value.deserialize(data)
            }
        })
    }

    // パラメータの書き込み
    async function writeParams() {
        return runExclusive(async () => {
            if (!service.value) return

            console.log("writing params...")
            await service.value.writeParams(params.value.serialize())

            // Ver.1.0.0のみ本体ROMへパラメータを記録させる
            // Ver.1.1.0以降のバージョンでは、
            // "32150001-9a86-43ac-b15f-200ed1b7a72a" の中で記録される
            if (devInfo.value.version === 0x1000) {
                console.log("storing params in flash memory...")
                await service.value.writeRom()
            }
        })
    }

    // 統計データの読み込み
    async function readStatistics() {
        return runExclusive(async () => {
            if (!service.value) return

            // ヘッダ情報の取得
            console.log("reading basic statistics...")
            const data = await service.value.readHeader()
            if (data) {
                stats.value.deserialize(data)
            }

            // ヒストグラムデータの取得
            const hists = new Array<DataView<ArrayBufferLike>>(3)
            for (let i = 0; i < 3; i += 1) {
                console.log(`reading shoot-power distribution...${i+1}/3`)
                const data = await service.value.readHistogram()
                if (!data) {
                    return
                }
                hists[i] = data
            }
            histogram.value.deserialize(stats.value, hists)
        })
    }

    // 統計データの初期化
    async function clearStatistics() {
        return runExclusive(async () => {
            if (!service.value) return

            console.log("clearing statistics data...")
            await service.value.clearStatistics()
        })
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
        stats,
        histogram,
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
        readStatistics,
        clearStatistics,
        launchManually,
        switchToAutoMode
    }
}

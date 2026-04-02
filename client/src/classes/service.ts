import {
    readCharacteristic,
    tryGetCharacteristic,
    writeCharacteristic
} from "../controls/ble"

//-----------------------------------------------------------------------------
// キャラクタリスティック
//-----------------------------------------------------------------------------

// パラメータを読み書きを行うキャラクタリスティック
//
// - Ver.1.2.0未満：デバイス情報を含む
export const ATLAS_CHR_PARAMS = "32150001-9a86-43ac-b15f-200ed1b7a72a"

// パラメータをROMに書き込むかどうかを設定するためのキャラクタリスティック
// 
// - ver.1.1.0未満：本体ROMへパラメータを記録させる
// - ver.1.1.0以降：廃止（常に書き込むようにした）
export const ATLAS_CHR_WR_ROM = "32150010-9a86-43ac-b15f-200ed1b7a72a"

// マニュアル射出を行うキャラクタリスティック
export const ATLAS_CHR_SHOOT = "32150020-9a86-43ac-b15f-200ed1b7a72a"

// ヘッダーデータの読み出しを行うキャラクタリスティック
//
// - ver.2.0.0以降：廃止（ATLAS_CHR_DATAのREADに統合）
export const ATLAS_CHR_HEADER = "32150030-9a86-43ac-b15f-200ed1b7a72a"

// 統計データの取得を行うキャラクタリスティック
export const ATLAS_CHR_DATA = "32150031-9a86-43ac-b15f-200ed1b7a72a"

// コントローラ内のデータの初期化を行うキャラクタリスティック
//
// - ver.2.0.0以降：廃止（ATLAS_CHR_DATAのWRITEに統合）
export const ATLAS_CHR_CLEAR = "32150040-9a86-43ac-b15f-200ed1b7a72a"

// コントローラを計測モード(A)に切り替えるキャラクタリスティック
export const ATLAS_CHR_SWITCH = "32150050-9a86-43ac-b15f-200ed1b7a72a"

// デバイス情報の読み取りを行うキャラクタリスティック
//
// - Ver.1.2.0未満：未実装
export const ATLAS_CHR_DEVINFO = "32150060-9a86-43ac-b15f-200ed1b7a72a"

// 生データ取得制御を行うキャラクタリスティック
export const ATLAS_CHR_RAW_CTRL = "32150070-9a86-43ac-b15f-200ed1b7a72a"

// 生データの取得を行うキャラクタリスティック
export const ATLAS_CHR_RAW_DATA = "32150071-9a86-43ac-b15f-200ed1b7a72a"

//-----------------------------------------------------------------------------
// サービス操作
//-----------------------------------------------------------------------------

export class AtlasService {

    private charDataRaw: BluetoothRemoteGATTCharacteristic | null = null

    constructor(
        private _service: BluetoothRemoteGATTService
    ) {}

    // デバイス情報を取得する
    async readDeviceInfo(): Promise<DataView<ArrayBufferLike> | null> {
        return await readCharacteristic(
            this._service,
            ATLAS_CHR_DEVINFO
        )
    }

    // パラメータを読み出す
    async readParams(): Promise<DataView<ArrayBufferLike> | null> {
        return await readCharacteristic(
            this._service,
            ATLAS_CHR_PARAMS
        )
    }

    // パラメータを書き込む
    async writeParams(value: Uint8Array<ArrayBuffer>): Promise<boolean> {
        return await writeCharacteristic(
            this._service,
            ATLAS_CHR_PARAMS,
            value
        ) 
    }

    // マニュアルシュートを行う
    async launchManually(): Promise<boolean> {
        return await writeCharacteristic(
            this._service,
            ATLAS_CHR_SHOOT,
            new Uint8Array([1])
        )
    }

    // データヘッダーを読み出す（2.0.0未満）
    async readHeader(): Promise<DataView<ArrayBufferLike> | null> {
        return await readCharacteristic(
            this._service,
            ATLAS_CHR_HEADER
        )
    }

    // 結果を読み出す
    async readResult(): Promise<DataView<ArrayBufferLike> | null> {
        return await readCharacteristic(
            this._service,
            ATLAS_CHR_DATA
        )
    }

    // 結果をクリアする（2.0.0以降）
    async clearResult(): Promise<boolean> {
        return await writeCharacteristic(
            this._service,
            ATLAS_CHR_DATA,
            new Uint8Array([1])
        ) 
    }

    // 結果をクリアする（2.0.0未満）
    async clearResult1x(): Promise<boolean> {
        return await writeCharacteristic(
            this._service,
            ATLAS_CHR_CLEAR,
            new Uint8Array([1])
        ) 
    }

    // コントローラを計測モード(A)に切り替える
    async switchToAutoMode(): Promise<boolean> {
        return await writeCharacteristic(
            this._service,
            ATLAS_CHR_SWITCH,
            new Uint8Array([1])
        )
    }

    // 生データのサイズを取得する
    async readRawDataSize(): Promise<DataView<ArrayBufferLike> | null> {
        return await readCharacteristic(
            this._service,
            ATLAS_CHR_RAW_CTRL
        )
    }

    // 生データ送信を依頼する
    async startReceiveRawData(): Promise<boolean> {
        return await writeCharacteristic(
            this._service,
            ATLAS_CHR_RAW_CTRL,
            new Uint8Array([1])
        )
    }

    // 生データを読み出すための購読を開始
    async subscribeRawData(
        handler: (event: Event) => void
    ): Promise<boolean> {
        try {
            // キャラクタリスティックの取得
            this.charDataRaw = await this._service.getCharacteristic(
                ATLAS_CHR_RAW_DATA
            )

            // 購読の開始
            await this.charDataRaw.startNotifications()

            // イベントハンドラの登録
            this.charDataRaw.addEventListener(
                "characteristicvaluechanged",
                handler
            )

            return true
        }
        catch (error) {
            console.log(error);
        }
        return false
    }

    // 生データを読み出すための購読を停止
    async unsubscribeRawData(
        handler: (event: Event) => void
    ) {
        try {
            if (!this.charDataRaw) return

            // イベントハンドラの削除
            this.charDataRaw.removeEventListener(
                "characteristicvaluechanged",
                handler
            )

            // 購読の停止
            await this.charDataRaw.stopNotifications()

            this.charDataRaw = null
        }
        catch (error) {
            console.log(error);
        }
    }

    // バージョンが1.1.0未満かどうか
    async isUnder110(): Promise<boolean> {
        // パラメータをROMに書き込むかどうかを設定するためのキャラクタリスティックが存在するかで判断
        // 1.1.0以前に実装されていて、現在は廃止している
        return await tryGetCharacteristic(
            this._service,
            ATLAS_CHR_WR_ROM
        ) ? true : false
    }

    // パラメータをROMに書き込むかどうかを設定（1.1.0未満）
    async writeRom(): Promise<boolean> {
        return await writeCharacteristic(
            this._service,
            ATLAS_CHR_WR_ROM,
            new Uint8Array([1])
        )
    }
}

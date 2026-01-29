import {
    tryGetCharacteristic,
    readCharacteristic,
    writeCharacteristic
} from "./ble"

// パラメータを読み書きを行うキャラクタリスティック
// Ver.1.2未満には、デバイス情報を取得するキャラクタリスティックがない。
// したがって、必要な情報はパラメータから取得する。
export const ATLAS_CHR_SET     = "32150001-9a86-43ac-b15f-200ed1b7a72a"

// パラメータをROMに書き込むかどうかを設定するためのキャラクタリスティック
// ver.1.1.0以前に実装されていて、現在は廃止
// Ver.1.0.0のみ本体ROMへパラメータを記録させる
// Ver.1.1.0以降のバージョンでは、"32150001-9a86-43ac-b15f-200ed1b7a72a" の中で記録される
export const ATLAS_CHR_WR_ROM  = "32150010-9a86-43ac-b15f-200ed1b7a72a"

// マニュアル射出を行うキャラクタリスティック
export const ATLAS_CHR_SHOOT   = "32150020-9a86-43ac-b15f-200ed1b7a72a"

// ヘッダーデータの読み出しを行うキャラクタリスティック
export const ATLAS_CHR_HEADER  = "32150030-9a86-43ac-b15f-200ed1b7a72a"

// ヒストグラムデータの取得を行うキャラクタリスティック
export const ATLAS_CHR_DATA    = "32150031-9a86-43ac-b15f-200ed1b7a72a"

// コントローラ内のデータの初期化を行うキャラクタリスティック
export const ATLAS_CHR_CLEAR   = "32150040-9a86-43ac-b15f-200ed1b7a72a"

// コントローラを計測モード(A)に切り替えるキャラクタリスティック
export const ATLAS_CHR_SWITCH  = "32150050-9a86-43ac-b15f-200ed1b7a72a"

// デバイス情報の読み取りを行うキャラクタリスティック
export const ATLAS_CHR_DEVINFO = "32150060-9a86-43ac-b15f-200ed1b7a72a"

/*
    サービス操作
*/
export class AtlasService {
    constructor(
        private _service: BluetoothRemoteGATTService
    ) {}

    // ATLASのデバイス情報を取得する
    // 1.2.0以降のみ有効
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
            ATLAS_CHR_SET
        )
    }

    // パラメータを書き込む
    async writeParams(value: Uint8Array<ArrayBuffer>): Promise<boolean> {
        return await writeCharacteristic(
            this._service,
            ATLAS_CHR_SET,
            value
        ) 
    }

    // データヘッダーを読み出す
    async readHeader(): Promise<DataView<ArrayBufferLike> | null> {
        return await readCharacteristic(
            this._service,
            ATLAS_CHR_HEADER
        )
    }

    // データ本体を読み出す
    async readHistogram(): Promise<DataView<ArrayBufferLike> | null> {
        return await readCharacteristic(
            this._service,
            ATLAS_CHR_DATA
        )
    }

    // データをクリアする
    async clearStatistics(): Promise<boolean> {
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

    // マニュアルシュートを行う
    async launchManually(): Promise<boolean> {
        return await writeCharacteristic(
            this._service,
            ATLAS_CHR_SHOOT,
            new Uint8Array([1])
        )
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

    async writeRom(): Promise<boolean> {
        return await writeCharacteristic(
            this._service,
            ATLAS_CHR_WR_ROM,
            new Uint8Array([1])
        )
    }

}

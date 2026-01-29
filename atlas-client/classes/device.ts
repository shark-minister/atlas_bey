//-----------------------------------------------------------------
// デバイス実装情報
//-----------------------------------------------------------------
export class DeviceInfo {
    /*
        バージョン（16ビット整数）
        - 0xF000: メジャーバージョン
        - 0x0F00: マイナーバージョン
        - 0x00FF: リビジョン
    */
    version: number = 0x1000
    /*
        利用形態。SP計測器としての実装か否か
        - 0: 電動ランチャー制御器
        - 1: SP測定器
    */
    format: number = 1
    /*
        モード切替のスイッチタイプ
        - 0: スイッチなし
        - 1: スライドスイッチ
        - 2: タクトスイッチ
    */
    switchType: number = 1
    // 使用するモーター数
    numElrs: number = 1
    // モーター1の最大回転数
    elr1MaxRpm: number = 0
    // モーター2の最大回転数
    elr2MaxRpm: number = 0

    useSoftwareSwitch(): boolean {
        return this.switchType === 0 || this.switchType === 2
    }

    isElrController(): boolean {
        return this.format === 0
    }

    deserialize(data: DataView<ArrayBufferLike>): void {
        // バージョン取得（リトルエンディアン）
        this.version = data.getUint16(0, true)

        // コンディション取得（リトルエンディアン）
        const cond = data.getUint16(2, true)

        /*
            実装フォーマット（マスク：0000 0011） 
            - `0`: 電動ランチャー制御器
            - `1`: SP測定器
            - `2`: 予備
            - `3`: 予備
        */
        this.format = (cond & 0b11)

        /*
            モード切替のスイッチタイプ
            - `0`: スイッチなし
            - `1`: スライドスイッチ
            - `2`: タクトスイッチ
        */
        this.switchType = (this.version < 0x1300)
            // 1.3.0未満は、マスク 0000 0100
            // - フラグが立っていたらスイッチレスなので、0
            // - フラグがなかったらスライドスイッチなので、1
            ? ((cond & 0b100) > 0 ? 0 : 1)
            // 1.3.0以降は、マスク 0001 1100
            // 該当の3ビットの値をそのまま読み込む
            : (cond & 0b11100)

        /*
            電動ランチャーは2台構成か否か（マスク：0001 1000） 
            - `0`: モータは1台構成
            - `1`: モータは2台構成
            - `2`: 予備
            - `3`: 予備
        */
        this.numElrs = ((cond & 0b11000) >> 3) + 1

        // モーターの最大回転数
        this.elr1MaxRpm = data.getUint8(4)
        this.elr2MaxRpm = data.getUint8(5)
    }

    deserialize11x(data: DataView<ArrayBufferLike>): void {
        // バージョンは固定
        this.version = 0x1100
        // 先頭の1バイトを取得
        const byte = data.getUint8(0)
        // 0000 0010
        this.format = (byte & 0b10) > 0 ? 1 : 0
        // 0000 0100  フラグが立っていたらスイッチレス
        this.switchType = (byte & 0b100) > 0 ? 0 : 1
        // 0001 0000
        this.numElrs = (byte & 0b10000) > 0 ? 2 : 1
    }
}

// // ver.1.2.0以降で用いるデバイス情報取得
// export function decodeDeviceInfo(
//     data: DataView<ArrayBufferLike>
// ): DeviceInfo {
//     const devInfo = new DeviceInfo
//     // バージョン取得（リトルエンディアン）
//     devInfo.version = data.getUint16(0, true)

//     // コンディション取得（リトルエンディアン）
//     const cond = data.getUint16(2, true)

//     /*
//         実装フォーマット（マスク：0000 0011） 
//         - `0`: 電動ランチャー制御器
//         - `1`: SP測定器
//         - `2`: 予備
//         - `3`: 予備
//     */
//     devInfo.format = (cond & 0b11)

//     /*
//         モード切替のスイッチタイプ
//         - `0`: スイッチなし
//         - `1`: スライドスイッチ
//         - `2`: タクトスイッチ
//     */
//     devInfo.switchType = (devInfo.version < 0x1300)
//         // 1.3.0未満は、マスク 0000 0100
//         // - フラグが立っていたらスイッチレスなので、0
//         // - フラグがなかったらスライドスイッチなので、1
//         ? ((cond & 0b100) > 0 ? 0 : 1)
//         // 1.3.0以降は、マスク 0001 1100
//         // 該当の3ビットの値をそのまま読み込む
//         : (cond & 0b11100)

//     /*
//         電動ランチャーは2台構成か否か（マスク：0001 1000） 
//         - `0`: モータは1台構成
//         - `1`: モータは2台構成
//         - `2`: 予備
//         - `3`: 予備
//     */
//     devInfo.numElrs = ((cond & 0b11000) >> 3) + 1

//     // モーターの最大回転数
//     devInfo.elr1MaxRpm = data.getUint8(4)
//     devInfo.elr2MaxRpm = data.getUint8(5)

//     return devInfo
// }

// // ver.1.1.xで用いるデバイス情報取得
// export function decodeDeviceInfo11x(
//     data: DataView<ArrayBufferLike>
// ): DeviceInfo {
//     const devInfo = new DeviceInfo

//     // 先頭の1バイトを取得
//     const byte = data.getUint8(0)
//     // 0000 0010
//     devInfo.format = (byte & 0b10) > 0 ? 1 : 0
//     // 0000 0100  フラグが立っていたらスイッチレス
//     devInfo.switchType = (byte & 0b100) > 0 ? 0 : 1
//     // 0001 0000
//     devInfo.numElrs = (byte & 0b10000) > 0 ? 2 : 1
//     // バージョンは固定
//     devInfo.version = 0x1100

//     return devInfo
// }

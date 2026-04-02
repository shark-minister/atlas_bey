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

    // バージョン文字列
    versionStr(): string {
        return `${(this.version >> 12) & 0xF}.${(this.version >> 8) & 0xF}.${this.version & 0xFF}`
    }

    // ソフトウェアスイッチを使うかどうか
    useSoftwareSwitch(): boolean {
        return this.switchType === 0 || this.switchType === 2
    }

    // 電動ランチャー制御が有効かどうか
    isElrController(): boolean {
        return this.format === 0
    }

    // デシリアライズ
    deserialize(data: DataView<ArrayBufferLike>): void {
        // バージョン取得（リトルエンディアン）
        this.version = data.getUint16(0, true)

        // コンディション取得（リトルエンディアン）
        const cond = data.getUint16(2, true)

        //  実装フォーマット（マスク：0000 0011） 
        this.format = (cond & 0b11)

        // 1.3.0未満
        if (this.version < 0x1300) {
            // スイッチレスかどうかのフラグ（マスク：0000 0100） 
            // - 0: スライドスイッチ --> switchType = 1
            // - 1: スイッチレス --> switchType = 0
            this.switchType =  ((cond & 0b100) > 0 ? 0 : 1)

            // モーター数（マスク：0011 0000） 
            this.numElrs = ((cond & 0b11000) >> 3) + 1
        }
        // 1.3.0以降
        else {
            // モード切替のスイッチタイプ（マスク：0001 1100）
            this.switchType = (cond & 0b11100) >> 2

            // モーター数（マスク：0110 0000） 
            this.numElrs = ((cond & 0b110000) >> 4) + 1
        }
    }

    // デシリアライズ（Ver.1.2未満にはバージョン情報がない）
    deserialize11x(data: DataView<ArrayBufferLike>): void {
        // バージョンは固定
        this.version = 0x1100
        
        // 先頭の1バイト
        const firstByte = data.getUint8(0)

        // 0000 0010
        this.format = (firstByte & 0b10) > 0 ? 1 : 0

        // 0000 0100  フラグが立っていたらスイッチレス
        this.switchType = (firstByte & 0b100) > 0 ? 0 : 1

        // 0001 0000
        this.numElrs = (firstByte & 0b10000) > 0 ? 2 : 1
    }
}

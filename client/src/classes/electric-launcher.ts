/*
    電動ランチャーの設定を保持するクラス
*/
export class ElectricLauncher {
    // マニュアルモードで利用するか否か
    enabledManual: boolean = false

    // 右回転か否か。falseなら左回転
    isRotRight: boolean = true

    // シュートパワー
    sp: number = 10000

    serializeTo(data: DataView<ArrayBuffer>) {
        data.setUint8(
            0,
            (this.enabledManual ? 1 : 0) | (this.isRotRight ? 0 : 0b1000)
        )
        data.setUint8(1, this.sp / 100)
    }

    deserialize(data: DataView<ArrayBufferLike>) {
        // フラグ
        const flag = data.getUint8(0)
        this.enabledManual = ((flag & 0b0001) === 1)
        this.isRotRight = ((flag & 0b10000) === 0)

        // 電動ランチャー1のSP値 [rpm]
        // 値は 1/100 で送られてくる
        this.sp = data.getUint8(1) * 100
    }
}

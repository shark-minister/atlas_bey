import { ElectricLauncher } from "./electric-launcher"

export class AtlasParams {
    /*
        オートモードで利用する電動ランチャー
        - False: ELR 1
        - True:  ELR 2
    */
    elr2Auto: boolean = false

    // オートモードにて、表示するSP値の種類
    // - True:  BBPの値をメインに表示する
    // - False: 真のSP値をメインに表示する
    isBbpSpMain: boolean = false

    // 射出猶予 [ms]
    latency: number = 2000

    // 射出遅延 [ms]
    delay: number = 0

    // 電動ランチャー1
    elr1 = new ElectricLauncher()

    // 電動ランチャー2
    elr2 = new ElectricLauncher()

    // デシリアライズ
    deserialize(data: DataView<ArrayBufferLike>): void {
        // 先頭の2バイトを取得
        const firstByte = data.getUint16(0, true)

        // 0000 0000 0000 0001
        this.elr2Auto = (firstByte & 0b1) > 0

        // 0000 0000 0000 0110
        this.isBbpSpMain = (firstByte & 0b0110) > 0

        // 射出猶予時間 [ms]
        // 値は 1/10 で送られてくる
        this.latency = data.getUint8(2) * 10

        // ベイ射出遅延時間 [ms]
        // 値は 1/2 で送られてくる
        this.delay = data.getUint8(3) * 2

        // 電動ランチャー1
        this.elr1.deserialize(
            new DataView(data.buffer, data.byteOffset + 4, 2)
        )

        // 電動ランチャー2
        this.elr2.deserialize(
            new DataView(data.buffer, data.byteOffset + 6, 2)
        )
    }

    deserialize1x(data: DataView<ArrayBufferLike>): void {
        // 先頭の1バイトを取得
        const firstByte = data.getUint8(0)

        // 0000 0001
        this.elr2Auto = (firstByte & 0b1) > 0

        // 0000 1000
        this.isBbpSpMain = (firstByte & 0b1000) > 0

        // 射出猶予時間 [ms]
        // 値は 1/10 で送られてくる
        this.latency = data.getUint8(1) * 10

        // ベイ射出遅延時間 [ms]
        // 値は 1/2 で送られてくる
        this.delay = data.getUint8(2) * 2

        // 電動ランチャー1
        this.elr1.deserialize(
            new DataView(data.buffer, data.byteOffset + 3, 2)
        )

        // 電動ランチャー2
        this.elr2.deserialize(
            new DataView(data.buffer, data.byteOffset + 5, 2)
        )
    }

    serialize(): ArrayBuffer {
        const buffer = new ArrayBuffer(8)
        const data = new DataView(buffer)

        data.setUint16(
            0,
            (this.elr2Auto ? 0b1 : 0) | (this.isBbpSpMain ? 0b10 : 0),
            true
        )
        data.setUint8(2, this.latency / 10)
        data.setUint8(3, this.delay / 2)
        this.elr1.serializeTo(new DataView(buffer, 2, 4))
        this.elr2.serializeTo(new DataView(buffer, 2, 6))

        return buffer
    }

    serialize1x(): ArrayBuffer {
        const buffer = new ArrayBuffer(7)
        const data = new DataView(buffer)

        data.setUint8(
            0,
            (this.elr2Auto ? 0b1 : 0) | (this.isBbpSpMain ? 0b1000 : 0)
        )
        data.setUint8(1, this.latency / 10)
        data.setUint8(2, this.delay / 2)
        this.elr1.serializeTo(new DataView(buffer, 2, 3))
        this.elr2.serializeTo(new DataView(buffer, 2, 5))

        return buffer
    }
}

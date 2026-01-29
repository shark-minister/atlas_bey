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
    latency: number = 2000
    delay: number = 0
    elr1 = new ElectricLauncher()
    elr2 = new ElectricLauncher()

    deserialize(data: DataView<ArrayBufferLike>): void {
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

        // 電動ランチャー1の設定
        this.elr1.deserializeFlag(data.getUint8(3))

        // 電動ランチャー1のSP値 [rpm]
        // 値は 1/100 で送られてくる
        this.elr1.sp = data.getUint8(4) * 100

        // 電動ランチャー2の設定
        this.elr1.deserializeFlag(data.getUint8(5))

        // 値は 1/100 で送られてくる
        // 電動ランチャー2のSP値 [rpm]
        this.elr2.sp = data.getUint8(6) * 100
    }

    serialize(): Uint8Array<ArrayBuffer> {
        return new Uint8Array([
            (this.elr2Auto ? 0b1 : 0) | (this.isBbpSpMain ? 0b1000 : 0),
            this.latency / 10,
            this.delay / 2,
            this.elr1.serializeFlag(),
            this.elr1.sp / 100,
            this.elr2.serializeFlag(),
            this.elr2.sp / 100
        ])
    }
}

// // パラメータの解析
// export function decodeParams(
//     data: DataView<ArrayBufferLike>
// ): AtlasParams {
//     const params = new AtlasParams

//     // 先頭の1バイトを取得
//     const firstByte = data.getUint8(0)

//     // 0000 0001
//     params.elr2Auto = (firstByte & 0b1) > 0

//     // 0000 1000
//     params.isBbpSpMain = (firstByte & 0b1000) > 0

//     // 射出猶予時間 [ms]
//     // 値は 1/10 で送られてくる
//     params.latency = data.getUint8(1) * 10

//     // ベイ射出遅延時間 [ms]
//     // 値は 1/2 で送られてくる
//     params.delay = data.getUint8(2) * 2

//     // 電動ランチャー1の設定
//     params.elr1.deserializeFlag(data.getUint8(3))

//     // 電動ランチャー1のSP値 [rpm]
//     // 値は 1/100 で送られてくる
//     params.elr1.sp = data.getUint8(4) * 100

//     // 電動ランチャー2の設定
//     params.elr1.deserializeFlag(data.getUint8(5))

//     // 値は 1/100 で送られてくる
//     // 電動ランチャー2のSP値 [rpm]
//     params.elr2.sp = data.getUint8(6) * 100

//     return params
// }

// // パラメータのエンコード
// export function encodeParams(params: AtlasParams): Uint8Array<ArrayBuffer> {
//     return new Uint8Array([
//         (params.elr2Auto ? 0b1 : 0) | (params.isBbpSpMain ? 0b1000 : 0),
//         params.latency / 10,
//         params.delay / 2,
//         params.elr1.serializeFlag(),
//         params.elr1.sp / 100,
//         params.elr2.serializeFlag(),
//         params.elr2.sp / 100
//     ])
// }

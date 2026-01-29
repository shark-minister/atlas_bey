import { AtlasStats } from "./stats"

export class Histogram {
    label: Array<string> = []
    data: Array<number> = []

    reset(numBins: number): void {
        this.label = new Array(numBins)
        this.data = new Array(numBins)
    }

    deserialize(
        stats: AtlasStats,
        data: Array<DataView<ArrayBufferLike>>
    ): void {
        if (stats.total > 0 && data.length === 3) {
            // 確保
            this.reset(stats.histEnd - stats.histBegin + 1)

            for (let i = stats.histBegin; i <= stats.histEnd; i += 1) {
                const block = Math.floor(i / 20)
                const index = i - (block * 20)
                this.data[i - stats.histBegin] = data[block]!.getUint8(index)
                this.label[i - stats.histBegin] = String(4000 + i * 200)
            }
        }
    }
}

// // ヒストグラムデータのデコード
// export function decodeHistogram(
//     stats: AtlasStats,
//     data: Array<DataView<ArrayBufferLike>>
// ): Histogram | null {
//     // ヒストグラムデータの解析
//     if (stats.total > 0 && data.length === 3) {
//         // ヒストグラム
//         const hist = new Histogram(stats.histEnd - stats.histBegin + 1)

//         for (let i = stats.histBegin; i <= stats.histEnd; i += 1) {
//             const block = Math.floor(i / 20)
//             const index = i - (block * 20)
//             hist.data[i - stats.histBegin] = data[block]!.getUint8(index)
//             hist.label[i - stats.histBegin] = String(4000 + i * 200)
//         }

//         return hist
//     }
//     return null
// }

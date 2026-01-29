export class AtlasStats {
    total: number = 0
    maxSp: number = 0
    minSp: number = 0
    avgSp: number = 0
    stdSp: number = 0
    histBegin: number = 0
    histEnd: number = 0

    deserialize(data: DataView<ArrayBufferLike>): void {
        // ヘッダ情報の解析（すべてリトルエンディアン）
        this.total = data.getUint16(0, true)
        this.maxSp = data.getUint16(2, true)
        this.minSp = data.getUint16(4, true)
        this.avgSp = data.getUint16(6, true)
        this.stdSp = data.getUint16(8, true)
        this.histBegin = data.getUint8(10)
        this.histEnd = data.getUint8(11)
    }
}

// // ヘッダ情報のデコード
// export function decodeStats(
//     data: DataView<ArrayBufferLike>
// ): AtlasStats {
//     const stats = new AtlasStats

//     // ヘッダ情報の解析（すべてリトルエンディアン）
//     stats.total = data.getUint16(0, true)
//     stats.maxSp = data.getUint16(2, true)
//     stats.minSp = data.getUint16(4, true)
//     stats.avgSp = data.getUint16(6, true)
//     stats.stdSp = data.getUint16(8, true)
//     stats.histBegin = data.getUint8(10)
//     stats.histEnd = data.getUint8(11)

//     return stats
// }

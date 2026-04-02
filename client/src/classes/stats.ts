import { Histogram } from "./histogram"
import Encoding from 'encoding-japanese'

export class AtlasStats {
    // SP統計データ
    total: number = 0
    maxSP: number = 0
    minSP: number = 0
    meanSP: number = 0
    stdevSP: number = 0
    latestSP: number = 0
    hist = new Histogram

    deserialize(data: DataView<ArrayBufferLike>): void {
        // 値はすべてリトルエンディアン
        // SP統計情報
        this.total = data.getUint16(0, true)
        this.maxSP = data.getUint16(2, true)
        this.minSP = data.getUint16(4, true)
        this.meanSP = data.getUint16(6, true)
        this.stdevSP = data.getUint16(8, true)
        this.latestSP = data.getUint16(10, true)

        // ヒストグラム
        this.hist.deserialize(
            new DataView(
                data.buffer,
                data.byteOffset + 12,
                88
            )
        )
    }

    deserialize1x(
        dataHeader: DataView<ArrayBufferLike>,
        dataHists: Array<DataView<ArrayBufferLike>>
    ): void {
        this.total = dataHeader.getUint16(0, true)
        this.maxSP = dataHeader.getUint16(2, true)
        this.minSP = dataHeader.getUint16(4, true)
        this.meanSP = dataHeader.getUint16(6, true)
        this.stdevSP = dataHeader.getUint16(8, true)
        this.latestSP = dataHeader.getUint16(10, true)

        // ヒストグラム
        this.hist.deserialize1x(
            dataHeader.getUint8(10),
            dataHeader.getUint8(11),
            dataHists
        )
    }

    downloadFile(filename: string): void {
        if (this.hist.data.length === 0) return

        const sp = this.hist.data
        const lines = this.hist.label.map((label, index) => {
            return `${label},${sp[index]}`
        })

        // Shift JISへの変換
        const uint8Array = new Uint8Array(
            Encoding.convert(
                lines.join('\r\n'),
                {
                    to: 'SJIS',
                    from: 'UNICODE',
                    type: 'arraybuffer'
                }
            )
        )
        const blob = new Blob(
            [ uint8Array ],
            { type: 'text/csv' }
        )
        const link = document.createElement('a')
        link.href = URL.createObjectURL(blob)
        link.download = filename
        link.click()
        link.remove()
    }
}

export class Histogram {
    minIndex: number = 0
    maxIndex: number = 79
    binWidth: number = 200
    minSP: number = 2000
    maxSP: number = 18000
    label: Array<string> = []
    data: Array<number> = []

    deserialize(data: DataView<ArrayBufferLike>): void {
        // ヘッダ情報
        this.minIndex = data.getUint8(0)
        if (this.minIndex == 0xFF) {
            this.minIndex = 0
        }
        this.maxIndex = data.getUint8(1)
        this.binWidth = data.getUint16(2, true)
        this.minSP    = data.getUint16(4, true)

        // 確保
        const N = this.maxIndex - this.minIndex + 1
        this.label = new Array(N)
        this.data = new Array(N)
        const offset = this.minIndex + 8
        for (let i = 0; i < N; i += 1) {
            this.data[i] = data.getUint8(offset + i)
            this.label[i] = String(
                this.minSP + (i + this.minIndex) * this.binWidth
            )
        }
    }

    deserialize1x(
        minIndex: number,
        maxIndex: number,
        data: Array<DataView<ArrayBufferLike>>
    ): void {
        this.minIndex = minIndex
        this.maxIndex = maxIndex
        if (data.length === 3) {
            // 確保
            const N = maxIndex - minIndex + 1
            this.label = new Array(N)
            this.data = new Array(N)
            // 再配置
            for (let i = minIndex; i <= maxIndex; i += 1) {
                const h = Math.floor(i / 20)
                this.data[i - minIndex] = data[h]!.getUint8(i - (h * 20))
                this.label[i - minIndex] = String(4000 + i * 200)
            }
        }
    }
}

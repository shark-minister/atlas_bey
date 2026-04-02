import { AtlasStats } from "./stats"

export class AtlasResult {
    // SP統計データ
    statsOrig = new AtlasStats
    statsEval = new AtlasStats

    deserialize(data: DataView<ArrayBufferLike>): void {
        this.statsOrig.deserialize(data)
        this.statsEval.deserialize(
            new DataView(
                data.buffer,
                data.byteOffset + 112,
                112
            )
        )
    }

    deserialize1x(
        dataHeader: DataView<ArrayBufferLike>,
        dataHists: Array<DataView<ArrayBufferLike>>
    ): void {
        this.statsOrig.deserialize1x(dataHeader, dataHists)
        this.statsEval.deserialize1x(dataHeader, dataHists)
    }

}

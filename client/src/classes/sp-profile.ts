export interface Point {
    x: number,
    y: number
}

class RegParam {
  a: number = 0
  b: number = 0
}

export function leastSq(points: Array<Point>): RegParam {
  const n = points.length;
  const sumX = points.reduce((s, c) => s + c.x, 0)
  const sumY = points.reduce((s, c) => s + c.y, 0)
  const sumX2 = points.reduce((s, c) => s + c.x * c.x, 0)
  const sumXY = points.reduce((s, c) => s + c.x * c.y, 0)
  const d = (n * sumX2 - Math.pow(sumX, 2))
  const a = (n * sumXY - sumX * sumY) / d
  const b = (sumX2 * sumY - sumXY * sumX) / d
  return {
    a, b
  }
}

export class SPProfile {
    entryNumber: number = 0
    origSP: number = 0
    evalSP: number = 0
    regParam1 = new RegParam
    regParam2 = new RegParam
    points1 = new Array<Point>
    points2 = new Array<Point>
    accelPoints = new Array<Point>
    accels = new Array<number>
    myEvalSP: number = 0

    constructor(data: Uint16Array) {
        this.entryNumber = data[0]!
        this.origSP = data[1]!
        this.evalSP = data[2]!

        const points = new Array<Point>

        // 経過時間
        let elapsedTime: number = 0
        // ピークに到達したか
        let peakReached = false
        // 解析
        for (let i = 3; i < 35; i += 1) {
            // ランチャー1回転の間に、センサー（8μs間隔）で反射を計測した回数
            const nRefs = data[i]!

            // `0`はオーバーフロー？ 無視して次に進むことにする
            if (nRefs == 0) continue

            // ランチャー1回転に掛かった時間 [ms]
            // 1反射あたり8μsかかっているので、一回転あたりの時間は
            // nRefs*8 [μs] =  nRefs*8/1000 [ms] = nRefs/125 [ms]
            const dt = nRefs / 125

            // ランチャーの回転数（シュートパワー）[rpm], 60000 は ms->min の変換
            const sp = ~~(60000 / dt)

            // その回転が終了したときの、ランチャー引き始めからの時間t [ms]
            if (i > 3) {
                elapsedTime += ~~dt
            }

            points.push({
                x: elapsedTime,
                y: sp
            })
        }
        
        // for (let i = 1; i < this.points1.length; i += 1) {
        //     this.accelPoints.push({
        //         x: this.points1[i]!.x,
        //         y: (this.points1[i]!.y - this.points1[i-1]!.y) /
        //            (this.points1[i]!.x - this.points1[i-1]!.x)
        //     })
        // }

        const MAX_PEAK_LENGTH = 12

        // プロファイルのデータ点数が7点に満たない場合は、計算しない
        if (points.length < 7) {
            this.myEvalSP = this.origSP
        }
        else {
            // 初期化
            let maxSP = 0      // プロファイル上の最大SP
            let length = points.length > 15 ? 15 : points.length  // 13回転目以降は見ない
            let maxIndex = 0   /// length - 1

            // 4回転目から値をチェックする
            let peakIndex = 0
            for (let i = 4; i < length; ++i) {
                /*
                    チェックするポイント

                    P4'  i-4  4回転前（変数名 *_m4）
                    P2'  i-2  2回転前（変数名 *_m2）
                    P1'  i-1  1回転前（変数名 *_1）
                    P0   i    基準点（変数名 *_0）
                    P1   i+1  1回転前（変数名 *_p1）
                    P2   i+2  2回転前（変数名 *_p2）
                */

                // ピーク位置の更新
                peakIndex = i;

                // P0, P1' のSP値の取得
                const sp_0  = points[i]!.y
                const sp_m1 = points[i-1]!.y
                
                // 最大SP値の更新
                if (sp_0 > maxSP && i < MAX_PEAK_LENGTH) {
                    maxSP = sp_0
                    maxIndex = i       ///
                }

                // P1'→P0 で、減少に転じている場合
                if (sp_m1 > sp_0) {
                    let flag: boolean = false
                    const p1 = points[i+1]!
                    if ((i+2) < length) {
                        // さらに P1' → P0 → P1 → P2 と減少していくケース
                        // つまり P1' がピークトップとなっている
                        flag = (sp_0 > p1.y) && (p1.y > points[i+2]!.y)
                    }
                    else if ((i+1) < length) {
                        flag = (sp_0 > p1.y)
                    }

                    // 異常値の検査
                    if (flag) {
                        const t_m2  = points[i-2]!.x   // P2'の時間
                        const t_m4  = points[i-4]!.x   // P4'の時間
                        const sp_m2 = points[i-2]!.y   // P2'のSP値
                        const sp_m4 = points[i-4]!.y   // P4'のSP値

                        // P2' と P4' 間の傾き a の計算
                        const a = (sp_m2 - sp_m4) / (t_m2 - t_m4)

                        // P2'から傾き a で延長したときの、ピーク位置 P1' における期待SP値の計算
                        // 念のため、4%の安全係数を掛けておく
                        const extSP = 1.04 * ( a * (points[i-1]!.x - t_m2) + sp_m2)

                        // 期待値を超える SP が P1' で記録されている場合は、異常値の可能性が高い
                        if ((extSP < sp_m1) && (i >= 12)) {
                            // P2' をピークトップに切替
                            this.myEvalSP = sp_m2
                            peakIndex = i - 2
                        }
                        else {
                            // そうでないなら、P1' がそのままピークトップ
                            this.myEvalSP = sp_m1
                            peakIndex = i - 1
                        }
                    }
                    // P1, P2 のいずれかで再び増加している場合
                    // ストリングランチャー使用時に、射出の紐巻き戻り加速によるピークが記録
                    // その値が P1' のSP値より高くても使用しない
                    // バトルパスでは多くの場合に記録されてしまうダミーのSP値である
                    else {
                        // P1'がピークトップ
                        this.myEvalSP = sp_m1;
                        peakIndex = i - 1;
                    }
                    // 解析終了
                    break;
                }
            }

            if (peakIndex >= MAX_PEAK_LENGTH) {
                this.myEvalSP = maxSP
                peakIndex = maxIndex
            }
            else if (this.myEvalSP > this.origSP) {
                this.myEvalSP = this.origSP
                //peakIndex = this->_findIndex(evalSP, SP, size);
            }
        }

        for (const p of points) {
            // ピーク検知
            if (!peakReached && p.y === this.myEvalSP) {
                peakReached = true
                // 両方に格納
                this.points1.push({
                    x: p.x,
                    y: p.y
                })
                this.points2.push({
                    x: p.x,
                    y: p.y
                })
            }
            else {
                (peakReached ? this.points2 : this.points1).push({
                    x: p.x,
                    y: p.y
                })
            }
        }

        // 加速度解析
        this.regParam1 = leastSq(this.points1.slice(1, -4))
        this.regParam2 = leastSq(this.points1.slice(-4))
    }
}

export class SPProfileManager {
    rawData: Uint16Array | null = null
    profiles = new Array<SPProfile>

    set(data: Uint16Array): void {
        this.rawData = data
        this.profiles = new Array<SPProfile>(data.buffer.byteLength / 70)
        for (let i = 0; i < this.profiles.length; ++i) {
            this.profiles[i] = new SPProfile(
                new Uint16Array(
                    data.buffer,
                    data.byteOffset + i * 70
                )
            )
        }
    }

    numShoots(): number {
        return this.profiles.length
    }

    downloadFile(filename: string): void {
        if (!this.rawData) return
        
        const blob = new Blob([
            this.rawData.slice(
                this.rawData.byteOffset,
                this.rawData.byteOffset + this.rawData.byteLength
            )
        ])
        const link = document.createElement('a')
        link.href = URL.createObjectURL(blob)
        link.download = filename
        link.click()
        link.remove()
    }

    getProfile(index: number): SPProfile | null {
        return this.profiles[index] ?? null
    }
}
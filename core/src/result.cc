/*
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
*/
#include "result.hh"

#include <Arduino.h>

namespace atlas {
//-----------------------------------------------------------------------------

static double calcAcc(
    const std::uint16_t* t,
    const std::uint16_t* sp,
    std::uint16_t iBegin,
    std::uint16_t iEnd
) {
    const std::uint32_t N = iEnd - iBegin;
    std::uint32_t sumX = 0;
    std::uint32_t sumY = 0;
    std::uint32_t sumXX = 0;
    std::uint32_t sumXY = 0;

    for (std::uint16_t i = iBegin; i < iEnd; ++i) {
        sumX += t[i];
        sumY += sp[i];
        sumXX += t[i] * t[i];
        sumXY += t[i] * sp[i];
    }

    return static_cast<double>(N*sumXY - sumX*sumY) / (N*sumXX - sumX*sumX);
}

void Result::initialize() noexcept
{
    this->statsOrig.initialize();
    this->statsEval.initialize();
    this->clear();
}

void Result::clear() noexcept
{
    // SP統計データ
    this->statsOrig.clear();
    this->statsEval.clear();
}

void Result::update(
    std::uint16_t origSP,
    const std::uint16_t* rawProf,
    std::uint16_t& acc1,
    std::uint16_t& acc2
)
{
    std::uint16_t evalSP = 0;

    //-------------------------------------------------------------------------
    // プロファイルのデコード
    //-------------------------------------------------------------------------
    std::uint16_t T[32];
    std::uint16_t SP[32];
    std::uint16_t size;
    // 経過時間
    std::uint16_t elapsedTime = 0;

    // 解析
    for (int i = 0; i < 32; i += 1) {
        // ランチャー1回転の間に、センサー（8μs間隔）で反射を計測した回数
        auto nRefs = rawProf[i];

        // `0`はオーバーフロー？ 無視して次に進むことにする
        if (nRefs == 0) continue;

        // ランチャー1回転に掛かった時間 [ms]
        // 1反射あたり8μsかかっているので、一回転あたりの時間は
        // nRefs*8 [μs] =  nRefs*8/1000 [ms] = nRefs/125 [ms]
        auto dt = static_cast<double>(nRefs) / 125;

        // ランチャーの回転数（シュートパワー）[rpm], 60000 は ms->min の変換
        auto sp = static_cast<std::uint16_t>(60000 / dt);

        // その回転が終了したときの、ランチャー引き始めからの時間t [ms]
        elapsedTime += static_cast<std::uint16_t>(dt);

        // 格納
        T[size] = elapsedTime;
        SP[size] = sp;
        size += 1;
    }

    //-------------------------------------------------------------------------
    // 真のSP値の割り出し
    //-------------------------------------------------------------------------

    constexpr std::uint32_t MAX_PEAK_LENGTH = 12;

    // プロファイルのデータ点数が7点に満たない場合は、計算しない
    if (size < 7) {
        evalSP = origSP;
    }
    else {
        // 初期化
        std::uint16_t maxSP = 0;   // プロファイル上の最大SP
        std::uint32_t length = size > 15 ? 15 : size;
        std::uint32_t maxIndex = 0;   /// length - 1

        // 4回転目から値をチェックする
        std::uint16_t peakIndex = 0;
        for (std::uint32_t i = 4; i < length; ++i) {
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
            auto sp_0  = SP[i];
            auto sp_m1 = SP[i-1];
            
            // 最大SP値の更新
            if (sp_0 > maxSP && i < MAX_PEAK_LENGTH) {
                maxSP = sp_0;
                maxIndex = i;       ///
            }

            // P1'→P0 で、減少に転じている場合
            if (sp_m1 > sp_0) {
                bool flag = false;
                if ((i+2) < length) {
                    // さらに P1' → P0 → P1 → P2 と減少していくケース
                    // つまり P1' がピークトップとなっている
                    flag = (sp_0 > SP[i+1]) && (SP[i+1] > SP[i+2]);
                }
                else if ((i+1) < length) {
                    flag = (sp_0 > SP[i+1]);
                }

                // 異常値の検査
                if (flag) {
                    auto t_m2  = T[i-2];    // P2'の時間
                    auto t_m4  = T[i-4];    // P4'の時間
                    auto sp_m2 = SP[i-2];   // P2'のSP値
                    auto sp_m4 = SP[i-4];   // P4'のSP値

                    // P2' と P4' 間の傾き a の計算
                    auto a = static_cast<double>(sp_m2 - sp_m4) / (t_m2 - t_m4);

                    // P2'から傾き a で延長したときの、ピーク位置 P1' における期待SP値の計算
                    // 念のため、4%の安全係数を掛けておく
                    std::uint16_t extSP = 1.04 * ( a * (T[i-1] - t_m2) + sp_m2);

                    // 期待値を超える SP が P1' で記録されている場合は、異常値の可能性が高い
                    if ((extSP < sp_m1) && (i >= MAX_PEAK_LENGTH)) {
                        // P2' をピークトップに切替
                        evalSP = sp_m2;
                        peakIndex = i - 2;
                    }
                    else {
                        // そうでないなら、P1' がそのままピークトップ
                        evalSP = sp_m1;
                        peakIndex = i - 1;
                    }
                }
                // P1, P2 のいずれかで再び増加している場合
                // ストリングランチャー使用時に、射出の紐巻き戻り加速によるピークが記録
                // その値が P1' のSP値より高くても使用しない
                // バトルパスでは多くの場合に記録されてしまうダミーのSP値である
                else {
                    // P1'がピークトップ
                    evalSP = sp_m1;
                    peakIndex = i - 1;
                }
                // 解析終了
                break;
            }
        }

        if (peakIndex == (size - 1) || peakIndex >= MAX_PEAK_LENGTH) {
            evalSP = maxSP;
        }
        else if (evalSP > origSP) {
            evalSP = origSP;
        }

        //---------------------------------------------------------------------
        // 加速度データの計算
        //---------------------------------------------------------------------
        double a1 = calcAcc(T, SP, 1, peakIndex - 3);
        double a2 = calcAcc(T, SP, peakIndex - 3, peakIndex + 1);
        acc1 = a1 >= 0 ? static_cast<std::uint16_t>(a1) : 0;
        acc2 = a1 >= 0 ? static_cast<std::uint16_t>(a2) : 0;
    }

    //-------------------------------------------------------------------------
    // 統計計算
    //-------------------------------------------------------------------------
    this->statsOrig.update(origSP);
    this->statsEval.update(evalSP);
}

//-----------------------------------------------------------------------------
}
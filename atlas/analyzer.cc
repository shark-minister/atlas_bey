/*
    ATLAS - AuTomatic LAuncher System
    Copyright (C) 2025  Shark Minister
    All right reserved.
*/
#include "analyzer.hh"

// C++標準ライブラリ
#include <cmath>

// Arduino
#include <Arduino.h>

namespace atlas
{
//-----------------------------------------------------------------------------

constexpr int ACC_CALC_MAX = 8;

//! シュートの期待SP値を返す
std::uint16_t BBPAnalyzer::exp_sp() const noexcept
{
    return (_accel > 0) ? std::sqrt(24 * 60000 * _accel) : 0;
}

BBPState BBPAnalyzer::analyze(const BBPData& data)
{
    // ヘッダの取得
    auto hdr = data.header();

    /*
        ■ 内容
        ベイブレードの着脱イベント

        ■ ヘッダ
        HEADER_ATTACH_DETACH (A0)
        
        ■ 構成
        ------------------------------------------------------------
         オフセット  幅    内容
        ------------------------------------------------------------
            0       1    A0 (header)
            1       1    ? つねに3A
            2       1    -
            3       1    04(14): ベイ装着, 00(10): ベイ射出
            4       1    ? 取り付け、リリースで値が違う
            5       2    -
            7       2    最大シュートパワー
            9       2    シュート数（シュートカウンター）
           11       6    ベイバトルパスのユニークID
        ------------------------------------------------------------
    */
    if (hdr == HEADER_ATTACH_DETACH)
    {
        /*
            0x00 -> 0x04: ベイがランチャーに取り付けられた（イベントなし）
            0x00 -> 0x10: ベイがない状態でダブルクリックされた（電動ランチャーを有効にする）
            0x04 -> 0x00: ベイがランチャーから取り外された（イベントなし）
            0x10 -> 0x14: 電動ランチャーが有効な状態で、ベイが取り付けられた（射出命令発動）
            0x10 -> 0x00: 電動ランチャーが無効になった（イベントなし）
            0x14 -> 0x10: 射出命令発動中に、ベイが取り外された（射出キャンセル）
        */
        std::uint8_t state_bey = data.at(3);
        BBPState result = static_cast<BBPState>((_prev_state_bey << 8) | state_bey);
        _prev_state_bey = state_bey;
        return result;
    }

    // データの記録
    _data_map[hdr] = data;

    // データの終了
    if (hdr == HEADER_DATA_END)
    {
        // 解析を開始する
        return this->_analyze_impl() ? BBPState::FINISHED : BBPState::ERROR;       
    }

    // それ以外
    return BBPState::NONE;
}

bool BBPAnalyzer::_analyze_impl()
{
    /*
        ■ 内容
        チェックサム値の取得

        ■ ヘッダ
        HEADER_CHECKSUM (B7)
        
        ■ 構成
        ------------------------------------------------------------
         オフセット  幅    内容
        ------------------------------------------------------------
            0       1    B7 (header)
            1      15    -
           16       1    B0-B6のチェックサム値
        ------------------------------------------------------------
    */
    auto checksum = _data_map[HEADER_CHECKSUM].at(16);

    /*
        ■ 内容
        シュートパワーリスト（のうち、最新のSP）の取得

        ■ ヘッダ
        HEADER_LIST_FIRST (B0) - HEADER_LIST_LAST (B6)
        
        ■ 構成
        ------------------------------------------------------------
         オフセット  幅    内容
        ------------------------------------------------------------
            0       1    B0   B1   B2   B3   B4   B5   B6  (header)
            1       2    #1   #9  #17  #25  #33  #41  #49
            3       2    #2  #10  #18  #26  #34  #42  #50
            5       2    #3  #11  #19  #27  #35  #43    -
            7       2    #4  #12  #20  #28  #36  #44   *1
            9       2    #5  #13  #21  #29  #37  #45   *2
           11       2    #6  #14  #22  #30  #38  #46   *3
           13       2    #7  #15  #23  #31  #39  #47    -
           15       2    #8  #16  #24  #32  #40  #48    -
        ------------------------------------------------------------
         #1-#50: シュートパワーリスト
         *1: 最大シュートパワー
         *2: シュート数（シュートカウンター）
         *3: シュート数（シュートパワーリスト）
    */
    // 合計値の計算
    std::uint32_t sum = 0;
    for (auto h = HEADER_LIST_FIRST; h <= HEADER_LIST_LAST; ++h)
    {
        auto& data = _data_map[h];
        for (int i = 1; i < BBPData::LENGTH; ++i)
        {
            sum += data.at(i);
        }
    }
    // チェックサム
    if ((sum & 0xFF) != checksum)
    {
        Serial.println(checksum, HEX);
        Serial.println(sum & 0xFF, HEX);
        this->_data_map.clear();
        // エラー
        return false;
    }
    // SPリストのシュート数
    {
        // シュート数（シュートパワーリスト）の取得
        auto n = _data_map[HEADER_LIST_LAST].at(11);
        // 最新SPの格納位置の計算。上の表を参照。
        // ((n - 1) >> 3) + HEADER_LIST_FIRST: 最新SPがどのデータ列にあるか
        // (n & 7) * 2 - 1: 最新SPがデータ列のどの位置にあるか
        _sp = _data_map[((n-1)>>3)+HEADER_LIST_FIRST].uint16((n&7)*2-1);
    }

    /*
        ■ 内容
        シュートパワープロファイルの取得

        ■ ヘッダ
        HEADER_PROF_FIRST (70) - HEADER_PROF_LAST (73)
        
        ■ 構成
        ------------------------------------------------------------
         オフセット  幅    内容
        ------------------------------------------------------------
            0       1    70   71   72   73  (header)
            1       2    #1   #9  #17  #25
            3       2    #2  #10  #18  #26
            5       2    #3  #11  #19  #27
            7       2    #4  #12  #20  #28
            9       2    #5  #13  #21  #29
           11       2    #6  #14  #22  #30
           13       2    #7  #15  #23  #31
           15       2    #8  #16  #24  #32
        ------------------------------------------------------------
    */
    // 経過時間
    std::uint32_t t = 0;
    int prof_size = 0;
    std::uint32_t sum_xy = 0;
    std::uint32_t sum_x = 0;
    std::uint32_t sum_y = 0;
    std::uint32_t sum_x2 = 0;
    // _prof_size = 0;
    for (auto h = HEADER_PROF_FIRST; h <= HEADER_PROF_LAST; ++h)
    {
        auto& data = _data_map[h];
        for (int i = 1; i < BBPData::LENGTH; i += 2)
        {
            // ランチャー1回転の間に、センサー（8μs間隔）で反射を計測した回数
            auto n_refs = data.uint16(i);
            // `0`はオーバーフロー？ 無視して次に進むことにする
            if (n_refs == 0)
            {
                continue;
            }
            // ランチャー1回転に掛かった時間 [ms]
            // 1反射あたり8μsかかっているので、一回転あたりの時間は
            // n_refs*8 [μs] =  n_refs*8/1000 [ms] = n_refs/125 [ms]
            auto dt = static_cast<double>(n_refs) / 125;
            // ランチャーの回転数（シュートパワー）[rpm], 60000 は ms->min の変換
            auto sp = static_cast<std::uint16_t>(60000 / dt);
            // その回転が終了したときの、ランチャー引き始めからの時間t [ms]
            t += static_cast<std::uint16_t>(dt);

            // 加速度を解析
            // 横軸：時間、縦軸：SPで、最小二乗法
            if (prof_size <= ACC_CALC_MAX)
            {
                sum_x  += t;
                sum_y  += sp;
                sum_xy += t * sp;
                sum_x2 += t * t;

                if (prof_size == ACC_CALC_MAX)
                {
                    _accel = ((ACC_CALC_MAX+1) * sum_xy - sum_x * sum_y) /
                             ((ACC_CALC_MAX+1) * sum_x2 - sum_x * sum_x); 
                }
            }
            prof_size += 1;
        }
    }

    this->_data_map.clear();
    return true;
}

//-----------------------------------------------------------------------------
} // namespace atlas

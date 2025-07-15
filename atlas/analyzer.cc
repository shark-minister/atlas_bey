/*
    ATLAS - AuTomatic LAuncher System
    Copyright (C) 2025  Shark Minister
    All right reserved.
*/
#include "analyzer.hh"

// C++標準ライブラリ
//#include <array>
//#include <cmath>    // std::sqrt()

namespace atlas
{
//-----------------------------------------------------------------------------

//constexpr int ACC_CALC_MAX = 8;

/*
//! シュートの期待SP値を返す
std::uint16_t BBPAnalyzer::exp_sp() const noexcept
{
    return (_accel > 0) ? std::sqrt(24 * 60000 * _accel) : 0;
}
*/

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

    // データの終了 ==> 解析の開始
    if (hdr == HEADER_DATA_END)
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
            this->_data_map.clear();
            // エラー
            return BBPState::ERROR;
        }
        // SPリストのシュート数
        {
            // シュート数（シュートパワーリスト）の取得
            auto n = _data_map[HEADER_LIST_LAST].at(11);
            // 最新SPの格納位置の計算。上の表を参照。
            // ((n - 1) >> 3) + HEADER_LIST_FIRST: 最新SPがどのデータ列にあるか
            // (n & 7) * 2 - 1: 最新SPがデータ列のどの位置にあるか
            _bbp_sp = _data_map[((n-1)>>3)+HEADER_LIST_FIRST].uint16((n&7)*2-1);
            _true_sp = 0;
            _max_sp = 0;
        }

        return BBPState::FINISHED;    
    }

    // それ以外
    return BBPState::NONE;
}

void BBPAnalyzer::calc_true_sp()
{
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
    std::uint16_t t = 0;
    // プロファイル
    std::uint16_t prof_t[32];
    std::uint16_t prof_sp[32];
    //std::array<std::uint16_t, 32> prof_t;
    //std::array<std::uint16_t, 32> prof_sp;
    // 実効サイズ
    std::uint32_t size = 0;
    // 最大SP値
    _max_sp = 0;

    // プロファイルが収められているデータを走査
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
            // 格納
            prof_t[size] = t;
            prof_sp[size] = sp;
            size += 1;
            // 最大SP値
            if (_max_sp < sp)
            {
                _max_sp = sp;
            }
        }
    }
    
    // プロファイルのデータ点数が5点に満たない場合は、計算しない
    if (size < 5)
    {
        return;
    }

    std::uint16_t max_sp = 0;  // プロファイル上の最大SP
    std::uint16_t peak_t = 0;  // ピーク位置
    std::uint32_t length = size > 13 ? 13 : size;  // 13回転目以降は見ない
    // 4回転目から値をチェックする
    for (std::uint32_t i = 4; i < length; ++i)
    {
        /*
            チェックするSP
            sp_m4  i-4  4回転前のSP
            sp_m2  i-2  2回転前のSP
            sp_m1  i-1  1回転前のSP
            sp_0   i    基準となるSP
            sp_p1  i+1  1回転後のSP
            sp_p2  i+2  2回転後のSP
            sp_p4  i+4  4回転後のSP
        */
        // 基準となるSPの取得
        auto sp_0 = prof_sp[i];

        // 最大SP値の更新
        if (sp_0 > max_sp)
        {
            max_sp = sp_0;
        }
        // ピーク位置の更新
        peak_t = i;

        // 1つ前のSP値の取得
        auto sp_m1 = prof_sp[i-1];

        // 減少に転じている場合
        if (sp_m1 > sp_0)
        {
            // 1, 2つ先のSP値を取得
            auto sp_p1 = prof_sp[i+1];
            auto sp_p2 = prof_sp[i+2];
            // さらに減少していくケース
            // つまりi-1がピークトップとなっている
            if (sp_0 > sp_p1 && sp_p1 > sp_p2)
            {
                auto t_m2  = prof_t[i-2];   // 2つ前の時間
                auto t_m4  = prof_t[i-4];   // 4つ前の時間
                auto sp_m2 = prof_sp[i-2];  // 2つ前のSP
                auto sp_m4 = prof_sp[i-4];  // 4つ前のSP
                // 傾きの計算
                auto a = static_cast<double>(sp_m2-sp_m4) / (t_m2-t_m4);
                std::uint16_t ext_sp = a*(prof_t[i-1]-t_m2) + sp_m2;
                // 真のSP値
                if (ext_sp < sp_m1)
                {
                    _true_sp = sp_m2;
                    peak_t = i - 2;
                }
                else
                {
                    _true_sp = sp_m1;
                    peak_t = i - 1;
                }
            }
            else
            {
                _true_sp = sp_m1;
                peak_t = i - 1;
            }
            break;
        }
    }

    if (peak_t == (size -1))
    {
        _true_sp = max_sp;
    }
    else if (peak_t < 4)
    {
        _true_sp = 0;
    }
    else if (_true_sp > _bbp_sp)
    {
        _true_sp = _bbp_sp;
    }
/*
    // 加速度計算
    int pf_size = 0;
    std::uint32_t sum_xy = 0;
    std::uint32_t sum_x = 0;
    std::uint32_t sum_y = 0;
    std::uint32_t sum_x2 = 0;

    for (std::uint32_t i = 0; i < pf_size; ++i)
    {
        // 加速度を解析
        // 横軸：時間、縦軸：SPで、最小二乗法
        if (i <= ACC_CALC_MAX)
        {
            auto t = prof[i].first;
            auto sp = prof[i].second;
            
            sum_x  += t;
            sum_y  += sp;
            sum_xy += t * sp;
            sum_x2 += t * t;

            if (i == ACC_CALC_MAX)
            {
                _accel = ((ACC_CALC_MAX+1) * sum_xy - sum_x * sum_y) /
                         ((ACC_CALC_MAX+1) * sum_x2 - sum_x * sum_x); 
            }
        }
    }
*/
}

void BBPAnalyzer::clear()
{
    this->_data_map.clear();
}

//-----------------------------------------------------------------------------
} // namespace atlas

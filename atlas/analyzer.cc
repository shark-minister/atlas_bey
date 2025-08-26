/*
    ATLAS - AuTomatic LAuncher System
    Copyright (C) 2025  Shark Minister
    All right reserved.
*/
#include "analyzer.hh"

namespace atlas
{
//-----------------------------------------------------------------------------

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

void BBPAnalyzer::analyze_profile()
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
    std::uint16_t et = 0;
    // プロファイル（最大32点）
    std::uint16_t prof_et[32];
    std::uint16_t prof_sp[32];
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
            et += static_cast<std::uint16_t>(dt);
            // 格納
            prof_et[size] = et;
            prof_sp[size] = sp;

            size += 1;
            // 最大SP値
            if (_max_sp < sp)
            {
                _max_sp = sp;
            }
        }
    }
    
    // プロファイルのデータ点数が7点に満たない場合は、計算しない
    if (size < 7)
    {
        _true_sp = _bbp_sp;
        return;
    }

    std::uint16_t max_sp = 0;  // プロファイル上の最大SP
    std::uint16_t peak_et = 0;  // ピーク位置
    std::uint32_t length = size > 14 ? 14 : size;  // 13回転目以降は見ない
    std::uint32_t max_index = length - 1;
    // 4回転目から値をチェックする
    for (std::uint32_t i = 4; i < length; ++i)
    {
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
        peak_et = i;

        // P0, P1'のSP値の取得
        auto sp_0 = prof_sp[i];
        auto sp_m1 = prof_sp[i-1];
        
        // 最大SP値の更新
        if (sp_0 > max_sp)
        {
            max_sp = sp_0;
        }

        // P1'→P0で、減少に転じている場合
        if (sp_m1 > sp_0)
        {
            bool flag = false;
            if ((i+2) <= max_index)
            {
                // さらにP1'→P0→P1→P2と減少していくケース
                // つまりP1'がピークトップとなっている
                flag = (sp_0 > prof_sp[i+1] && prof_sp[i+1] > prof_sp[i+2]);
            }
            else if ((i+1) <= max_index)
            {
                flag = (sp_0 >prof_sp[i+1]);
            }

            // 異常値の検査
            if (flag)
            {
                auto et_m2 = prof_et[i-2];   // P2'の時間
                auto et_m4 = prof_et[i-4];   // P4'の時間
                auto sp_m2 = prof_sp[i-2];   // P2'のSP値
                auto sp_m4 = prof_sp[i-4];   // P4'のSP値
                // P2'とP4'間の傾き a の計算
                auto a = static_cast<double>(sp_m2 - sp_m4) / (et_m2 - et_m4);
                // P2'から傾き a で延長したときの、ピーク位置 P1'における期待SP値の計算
                // 念のため、4%の安全係数を掛けておく
                std::uint16_t ext_sp = 1.04 * ( a * (prof_et[i-1] - et_m2) + sp_m2);
                // 期待値を超えるSPがP1'で記録されている場合は、異常値の可能性が高い
                if (ext_sp < sp_m1)
                {
                    // P2'をピークトップに切替
                    _true_sp = sp_m2;
                    peak_et = i - 2;
                }
                else
                {
                    // そうでないなら、P1'がそのままピークトップ
                    _true_sp = sp_m1;
                    peak_et = i - 1;
                }
            }
            // P1, P2のいずれかで再び増加している場合
            // ストリングランチャー使用時に、射出の紐巻き戻り加速によるピークが記録
            // その値がP1'のSP値より高くても使用しない
            // バトルパスでは多くの場合に記録されてしまうダミーのSP値である
            else
            {
                // P1'がピークトップ
                _true_sp = sp_m1;
                peak_et = i - 1;
            }
            // 解析終了
            break;
        }
    }

    if (peak_et == (size -1))
    {
        _true_sp = max_sp;
    }
    else if (peak_et < 4 || _true_sp > _bbp_sp)
    {
        _true_sp = _bbp_sp;
    }
}

void BBPAnalyzer::clear()
{
    this->_data_map.clear();
}

//-----------------------------------------------------------------------------
} // namespace atlas

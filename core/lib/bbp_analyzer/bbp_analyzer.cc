/*
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
*/
#include "bbp_analyzer.hh"

// C++標準ライブラリ
#include <cstring>  // std::memcpy

namespace shark {
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
            3       1    04(14): ベイ装着, 00(10): ベイ射出 ★★★
            4       1    ? 取り付け、リリースで値が違う
            5       2    -
            7       2    最大シュートパワー
            9       2    シュート数（シュートカウンター）
           11       6    ベイバトルパスのユニークID
        ------------------------------------------------------------
    */
    if (hdr == HEADER_ATTACH_DETACH) {
        /*
            BBPがダブルクリックされた
             - 0x00 -> 0x10: フラグオフ状態 → フラグオン状態
             - 0x10 -> 0x00: フラグオン状態 → フラグオフ状態
            ベイがランチャーに取り付けられた
             - 0x00 -> 0x04: フラグオフ状態
             - 0x10 -> 0x14: フラグオン状態
            ベイがランチャーから取り外された
             - 0x04 -> 0x00: フラグオフ状態
             - 0x14 -> 0x10: フラグオン状態
        */
        std::uint8_t stateBey = data.at(3);
        BBPState result = static_cast<BBPState>((_prevStateBey << 8) | stateBey);
        _prevStateBey = stateBey;
        return result;
    }

    // データの記録
    _dataMap[hdr] = data;

    // データの終了 ==> 解析の開始
    if (hdr == HEADER_DATA_END) {
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
        auto checksum = _dataMap[HEADER_CHECKSUM].at(16);

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
        for (auto h = HEADER_LIST_FIRST; h <= HEADER_LIST_LAST; ++h) {
            auto& data = _dataMap[h];
            for (int i = 1; i < BBPData::LENGTH; ++i) {
                sum += data.at(i);
            }
        }
        // チェックサム
        if ((sum & 0xFF) != checksum) {
            this->_dataMap.clear();
            // エラー
            return shark::BBPState::ERROR;
        }

        // シュート数（シュートパワーリスト）の取得
        auto n = _dataMap[HEADER_LIST_LAST].at(11);
        
        // 最新SPの格納位置の計算。上の表を参照。
        // ((n - 1) >> 3) + HEADER_LIST_FIRST: 最新SPがどのデータ列にあるか
        // (n & 7) * 2 - 1: 最新SPがデータ列のどの位置にあるか
        _sp = _dataMap[((n-1)>>3)+HEADER_LIST_FIRST].uint16((n&7)*2-1);

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
        // プロファイルが収められているデータを走査
        for (auto h = HEADER_PROF_FIRST; h <= HEADER_PROF_LAST; ++h) {
            // 16バイト分をコピー
            std::memcpy(
                reinterpret_cast<std::uint8_t*>(_raw) + (h-HEADER_PROF_FIRST)*16,
                _dataMap[h].data()+1,
                16
            );
        }

        return shark::BBPState::FINISHED;    
    }

    // それ以外
    return shark::BBPState::NONE;
}

void BBPAnalyzer::clear()
{
    this->_dataMap.clear();
}

//-----------------------------------------------------------------------------
} // namespace shark

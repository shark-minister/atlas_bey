/*
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
*/
#ifndef SHARK_MINISTER_BBP_ANALYZER_HH
#define SHARK_MINISTER_BBP_ANALYZER_HH

// C++標準ライブラリ
#include <cstdint>  // std::uint8_t, std::uint16_t, std::uint32_t
#include <map>      // std::map

// Atlas
#include "bbp_data.hh"
#include "bbp_state.hh"

namespace shark {
//-----------------------------------------------------------------------------

//! ベイバトルパス（BBP）からのデータを解析するクラス
class BBPAnalyzer
{
public:
    /*!
        @brief  BBPからのデータの解析を行う
        @param[in]  data  17bitのデータ
        @return  解析の状況を返す
    */
    BBPState analyze(const BBPData& data);

    //! バトルパスに記録されたシュートパワー値を返す
    inline std::uint16_t sp() const noexcept {
        return _sp;
    }

    //! 生データの取得
    inline const std::uint16_t* raw() const noexcept {
        return _raw;
    }

    //! 解析データのクリア
    void clear();

private:
    /*
        ベイバトルパス(BBP)からのnotifyデータのヘッダー一覧。

        - A0 (160): BBPがベイブレードのマウントを検知した
        - B0 (176): SP一覧のうち、1-8番目のSP値
        - B1 (177): SP一覧のうち、9-16番目のSP値
        - B2 (178): SP一覧のうち、17-24番目のSP値
        - B3 (179): SP一覧のうち、25-32番目のSP値
        - B4 (180): SP一覧のうち、33-40番目のSP値
        - B5 (181): SP一覧のうち、41-48番目のSP値
        - B6 (182): SP一覧のうち、49, 50番目のSP値、BBPに保存されているシュート数
        - B7 (183): チェックサム
        - 70 (112): SPプロファイルのうち、チャンネル1-8
        - 71 (113): SPプロファイルのうち、チャンネル9-16
        - 72 (114): SPプロファイルのうち、チャンネル17-24
        - 73 (115): SPプロファイルのうち、チャンネル25-32。また、最終行に相当
    */
    static constexpr std::uint8_t HEADER_ATTACH_DETACH = 0xA0;
    static constexpr std::uint8_t HEADER_LIST_FIRST    = 0xB0;
    static constexpr std::uint8_t HEADER_LIST_LAST     = 0xB6;
    static constexpr std::uint8_t HEADER_CHECKSUM      = 0xB7;
    static constexpr std::uint8_t HEADER_PROF_FIRST    = 0x70;
    static constexpr std::uint8_t HEADER_PROF_LAST     = 0x73;
    static constexpr std::uint8_t HEADER_DATA_END      = 0x73;

    //! ベイバトルパスからのデータ一式を格納するコンテナ
    std::map<std::uint8_t, BBPData> _dataMap;

    //! ベイの脱着フラグ（記録）
    std::uint8_t _prevStateBey = 0;

    //! バトルパスに記録されたシュートパワー値
    std::uint16_t _sp = 0;

    //! 生データ
    std::uint16_t _raw[32];
};

//-----------------------------------------------------------------------------
}
#endif

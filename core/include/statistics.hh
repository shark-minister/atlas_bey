/*
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
*/
#ifndef ATLAS_STATISTICS_HH
#define ATLAS_STATISTICS_HH

// C++標準ライブラリ
#include <cstdint>      // std::uint8_t
#include <type_traits>  // std::is_trivially_copyable_v

// ATLAS
#include "histogram.hh"

namespace atlas {
//-----------------------------------------------------------------------------

class Statistics
{
public:
    void initialize() noexcept;

    //! 統計データを初期化する
    void clear() noexcept;

    /*!
        @brief  統計情報を更新する
        @param[in]  sp  シュートパワー
    */
    void update(std::uint16_t sp) noexcept;

public:
    //! 統計情報
    std::uint16_t total;    //!< 累計シュート数
    std::uint16_t maxSP;    //!< 最大SP
    std::uint16_t minSP;    //!< 最小SP
    std::uint16_t meanSP;   //!< 平均SP
    std::uint16_t stdevSP;  //!< 標準偏差
    std::uint16_t latestSP; //!< 最新のSP

    //! ヒストグラムデータ本体
    Histogram hist;

private:
    // 計算用の一時変数
    std::uint32_t _sumSP;   //!< SP値の合計
    std::uint64_t _sumSP2;  //!< SP値の二乗の合計
};

static_assert(
    sizeof(Statistics) == 112,
    "Size of 'Statistics' is invalid size"
);

static_assert(std::is_trivially_default_constructible_v<Statistics>,
              "'Statistics' is not trivially default constructable");

static_assert(std::is_trivially_copyable_v<Statistics>,
              "'Statistics' is not trivially copyable");

//-----------------------------------------------------------------------------
} // namespace atlas
#endif


/*
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
*/
#ifndef ATLAS_HISTOGRAM_HH
#define ATLAS_HISTOGRAM_HH

// C++標準ライブラリ
#include <cstdint>      // std::uint8_t
#include <type_traits>  // std::is_trivially_copyable_v

// ATLAS
#include "setting.hh"

namespace atlas {
//-----------------------------------------------------------------------------

//! ヒストグラム
struct Histogram
{
    void initialize() noexcept;

    //! ヒストグラムのクリア
    void clear() noexcept;

    void append(std::uint16_t sp);

    inline std::uint8_t at(std::uint32_t index) const noexcept {
        return data[index];
    }

    std::uint8_t minIndex;
    std::uint8_t maxIndex;
    std::uint16_t binWidth;
    std::uint16_t minSP;
    std::uint16_t maxCount;
    std::uint8_t data[HIST_NUM_BINS];
};

static_assert(sizeof(Histogram) == 88,
              "Size of 'Histogram' is not 88 bytes");

static_assert(std::is_trivially_default_constructible_v<Histogram>,
              "'Histogram' is not trivially default constructable");

static_assert(std::is_trivially_copyable_v<Histogram>,
              "'Histogram' is not trivially copyable");

//-----------------------------------------------------------------------------
} // namespace atlas
#endif

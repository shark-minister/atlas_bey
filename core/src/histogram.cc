/*
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
*/
#include "histogram.hh"

// C++標準ライブラリ
#include <algorithm>   // std::max, std::min

// ATLAS
#include "setting.hh"

namespace atlas {
//-----------------------------------------------------------------------------

void Histogram::initialize() noexcept
{
    this->binWidth = HIST_BIN_WIDTH;
    this->minSP = HIST_MIN_SP;
    this->clear();
}

void Histogram::clear() noexcept
{
    for (std::uint32_t i = 0; i < HIST_NUM_BINS; ++i) {
        this->data[i] = 0;
    }
    this->minIndex = 0xFF;
    this->maxIndex = 0;
    this->maxCount = 0;
}

void Histogram::append(std::uint16_t sp)
{
    // ヒストグラムの更新
    if (sp >= HIST_MIN_SP && sp < HIST_MAX_SP) {
        // インデックスの計算
        std::uint8_t index = (sp - HIST_MIN_SP) / this->binWidth;

        this->minIndex = std::min(this->minIndex, index);
        this->maxIndex = std::max(this->maxIndex, index);

        // ヒストグラムと最大値の更新
        this->maxCount = std::max(
            this->maxCount,
            static_cast<std::uint16_t>(this->data[index] += 1)
        );
    }
}

//-----------------------------------------------------------------------------
}
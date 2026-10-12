/*
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
*/
#include "statistics.hh"

// C++標準ライブラリ
#include <cmath>       // std::sqrt
#include <algorithm>   // std::max, std::min

#include <Arduino.h>


namespace atlas {
//-----------------------------------------------------------------------------

static constexpr std::uint16_t MAX_SP = 0xFFFF;

void Statistics::initialize() noexcept
{
    this->hist.initialize();
    this->clear();
}

void Statistics::clear() noexcept
{
    // SP統計データ
    this->total = 0;
    this->maxSP = 0;
    this->minSP = 0;
    this->meanSP = 0;
    this->stdevSP = 0;
    this->latestSP = 0;

    // ヒストグラム
    this->hist.clear();

    // 計算用
    _sumSP = 0;;
    _sumSP2 = 0.0;
}

void Statistics::update(std::uint16_t sp) noexcept
{
    if (sp == 0) return;

    // SP値更新
    this->latestSP = sp;

    // ヒストグラムの更新
    this->hist.append(sp);

    // シュート数
    this->total += 1;

    // 最大・最小SP
    this->maxSP = std::max(this->maxSP, sp);
    this->minSP = std::min(this->minSP ? this->minSP : MAX_SP, sp);

    // SP合計
    _sumSP  += sp;
    _sumSP2 += sp * sp;

    // 平均SP
    double mean = _sumSP / static_cast<double>(this->total);
    this->meanSP = static_cast<std::uint16_t>(mean);

    // 標準偏差
    this->stdevSP = static_cast<std::uint16_t>(
        std::sqrt(_sumSP2 / static_cast<double>(this->total) - mean * mean)
    );
}

//-----------------------------------------------------------------------------
}
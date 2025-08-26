/*
    ATLAS - AuTomatic LAuncher System
    Copyright (C) 2025  Shark Minister
    All right reserved.
*/
#include "stats.hh"

// C++標準ライブラリ
#include <cmath>

namespace atlas
{
//-----------------------------------------------------------------------------

void Statistics::Header::init() noexcept
{
    total = 0;
    max_sp = 0;
    min_sp = 0;
    avg_sp = 0;
    std_sp = 0;
    hist_begin = 59;
    hist_end = 0;
}

void Statistics::Histogram::init() noexcept
{
    // ヒストグラムのクリア
    for (std::uint32_t i = 0; i < HIST_LENGTH; ++i)
    {
        data[i] = 0;
    }
}

void Statistics::init() noexcept
{
    // ヘッダ情報のクリア
    _header.init();

    // ヒストグラムのクリア
    for (std::uint32_t i = 0; i < NUM_HISTS; ++i)
    {
        _hists[i].init();
    }

    _latest_sp = 0;

    // 計算用
    _avg_sp_tmp = 0.0;
    _std_sp_tmp = 0.0;
    _sum_sp = 0.0;
    _sum_sp2 = 0.0;
}

void Statistics::update(std::uint16_t sp) noexcept
{
    if (sp >= HIST_MIN_SP && sp < HIST_MAX_SP)
    {
        // インデックスの計算
        std::uint8_t index = (sp - HIST_MIN_SP) / HIST_BIN_WIDTH;

        // ヒストグラム更新
        _hists[index / HIST_LENGTH].increment(index % HIST_LENGTH);

        // インデックス情報の更新
        if (index < _header.hist_begin)
        {
            _header.hist_begin = index;
        }
        if (index > _header.hist_end)
        {
            _header.hist_end = index;
        }
    }

    // シュート数
    _header.total += 1;

    // 最大・最小SP
    if (sp > _header.max_sp)
    {
        _header.max_sp = sp;
    }
    if (_header.min_sp == 0 || sp < _header.min_sp)
    {
        _header.min_sp = sp;
    }

    // SP合計
    _sum_sp += sp;
    _sum_sp2 += sp * sp;

    // 平均SP
    _avg_sp_tmp = _sum_sp / _header.total;
    _header.avg_sp = static_cast<std::uint16_t>(_avg_sp_tmp);

    // 標準偏差
    _std_sp_tmp = std::sqrt(_sum_sp2 / _header.total - _avg_sp_tmp * _avg_sp_tmp);
    _header.std_sp = static_cast<std::uint16_t>(_std_sp_tmp);

    // 最新SP
    _latest_sp = sp;
}

//-----------------------------------------------------------------------------
} // namespace atlas

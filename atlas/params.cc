/*
    ATLAS - AuTomatic LAuncher System
    Copyright (C) 2025  Shark Minister
    All right reserved.

    https://x.com/shark_minister
*/
#include "params.hh"

// ATLAS
#include "setting.hh"

namespace atlas
{
//-----------------------------------------------------------------------------

void ElectricLauncher::regulate() noexcept
{
    // SPのソフトウェアリミットチェック
    if (this->sp() > LAUNCHER_SP_UPPER_LIMIT)
    {
        _sp = LAUNCHER_SP_UPPER_LIMIT / 100;
    }
    else if (this->sp() < LAUNCHER_SP_LOWER_LIMIT)
    {
        _sp = LAUNCHER_SP_LOWER_LIMIT / 100;
    }
}

void Params::regulate() noexcept
{
    // 電動ランチャー1
#if NUM_MOTORS == 1
    _flags.field.elr_auto_mode = 0;
    _flags.field.num_motors_minus_1 = 0;
#elif NUM_MOTORS == 2
    // 電動ランチャー2
    _elr2.regulate();
    _flags.field.num_motors_minus_1 = 1;
#endif
    _elr1.regulate();

    // 実装形態
#if SP_MEAS_ONLY != 0
    // SP計測器としてのみ使う
    _flags.field.sp_meas_only = 1;
#else
    // 電動ランチャー制御としても使う
    _flags.field.sp_meas_only = 0;
#endif

    // スイッチレス
#if SWITCH_LESS != 0
    _flags.field.switch_less = 1;
#else
    _flags.field.switch_less = 0;
#endif

    // レイテンシ
    if (this->latency() < LATENCY_LOWER_LIMIT)
    {
        _latency = LATENCY_LOWER_LIMIT / 10;
    }

    // ディレイ
    if (this->delay() > DELAY_UPPER_LIMIT)
    {
        _delay = DELAY_UPPER_LIMIT / 2;
    }
}

//-----------------------------------------------------------------------------
} // namespace atlas

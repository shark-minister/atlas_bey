/*
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
*/
#include "params.hh"

// ATLAS
#include "setting.hh"

namespace atlas {
//-----------------------------------------------------------------------------

bool ELRParams::enabledManual() const noexcept
{
    return _flags.enabledManual == 1;
}

bool ELRParams::isRight() const noexcept
{
    return _flags.rotation == 0;
}

std::uint32_t ELRParams::sp() const noexcept
{
    return static_cast<std::uint32_t>(_sp) * 100;
}

void ELRParams::regulate() noexcept
{
    // SPのソフトウェアリミットチェック
    if (this->sp() > LAUNCHER_SP_UPPER_LIMIT) {
        _sp = LAUNCHER_SP_UPPER_LIMIT / 100;
    }
    else if (this->sp() < LAUNCHER_SP_LOWER_LIMIT) {
        _sp = LAUNCHER_SP_LOWER_LIMIT / 100;
    }

#if NUM_MOTORS == 1
    _flags.enabledManual = 1;
#endif
}

void ELRParams::initialize() noexcept
{
    _sp = DEFAULT_LAUNCHER_SP / 100;
    _flags.enabledManual = 0;
    _flags.rotation = 0;
}

//-----------------------------------------------------------------------------

// オートモードで使用する電動ランチャーのインデックス番号を返す
std::uint8_t Params::autoModeELRIndex() const noexcept
{
    return _flags.elrAutoMode;
}

// オートモードで使用する電動ランチャーのインスタンスを返す
const ELRParams& Params::autoModeELR() const noexcept
{
    return _flags.elrAutoMode == 0 ? _elr1 : _elr2;
}

// 指定インデックス番号の電動ランチャーのインスタンスを返す
const ELRParams& Params::elr(std::uint32_t index) const noexcept
{
    return index == 0 ? _elr1 : _elr2;
}

// オートモードの射出遅延時間 [ms]を返す
std::uint16_t Params::delay() const noexcept
{
    return static_cast<std::uint16_t>(_delay) * 2;
}

// オートモードの猶予時間 [ms]を返す
std::uint16_t Params::latency() const noexcept
{
    return static_cast<std::uint16_t>(_latency) * 10;
}

// 真のSP値をメインに表示するかどうかを返す
MainSPView Params::mainSPView() const noexcept
{
    return static_cast<MainSPView>(_flags.mainSP);
}

// 値を適正化する
void Params::regulate() noexcept
{
    _elr1.regulate();    // 電動ランチャー1
#if NUM_MOTORS == 1
    _flags.elrAutoMode = 0;
#elif NUM_MOTORS == 2
    _elr2.regulate();    // 電動ランチャー2
#endif

    // レイテンシ
    if (this->latency() < LATENCY_LOWER_LIMIT) {
        _latency = LATENCY_LOWER_LIMIT / 10;
    }
    
    // ディレイ
    if (this->delay() > DELAY_UPPER_LIMIT) {
        _delay = DELAY_UPPER_LIMIT / 2;
    }
}

// 値を初期化する
void Params::initialize() noexcept
{
    _latency = DEFAULT_LATENCY / 10;
    _delay = DEFAULT_DELAY / 2;
    _flags.elrAutoMode = 0;
    _flags.mainSP = 0;
    _elr1.initialize();
    _elr2.initialize();
}

//-----------------------------------------------------------------------------
} // namespace atlas

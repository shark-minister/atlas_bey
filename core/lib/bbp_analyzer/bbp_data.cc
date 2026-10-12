/*
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
*/
#include "bbp_data.hh"

// C++標準ライブラリ
#include <cstring>

namespace shark {
//-----------------------------------------------------------------------------

// 内容の初期化
void BBPData::clear() noexcept
{
    // 0で埋める
    std::memset(_bytes, 0, LENGTH);
}

// 指定位置の16ビットデータを16ビット整数値に変換して返す
std::uint16_t BBPData::uint16(int offset) const noexcept
{
    return static_cast<std::uint16_t>(_bytes[offset+1]) << 8 |
           static_cast<std::uint16_t>(_bytes[offset]);
}

//-----------------------------------------------------------------------------
} // namespace shark

/*
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
*/
#ifndef SHARK_MINISTER_BBP_STATE_HH
#define SHARK_MINISTER_BBP_STATE_HH

// C++標準ライブラリ
#include <cstdint>  // std::uint16_t

namespace shark
{
//-----------------------------------------------------------------------------

//! バトルパス状態
enum class BBPState
    : std::uint16_t
{
    ERROR           = 0xFFFF,   //!< データのチェックサムエラー
    NONE            = 0x0000,   //!< 変化なし
    BEY_ATTACHED_S1 = 0x0004,   //!< ベイブレードがランチャーにセットされた
    BEY_DETACHED_S1 = 0x0400,   //!< ベイブレードがランチャーから離れた
    TRANS_S1_TO_S2  = 0x0010,   //!< 電動ランチャー連動が有効になった
    TRANS_S2_TO_S1  = 0x1000,   //!< 電動ランチャー連動が無効になった
    BEY_ATTACHED_S2 = 0x1014,   //!< 電動ランチャーへのシュート指令が出された
    BEY_DETACHED_S2 = 0x1410,   //!< 電動ランチャーへのシュート指令がキャンセルされた
    FINISHED        = 0xF000    //!< 解析が終了した
};

//-----------------------------------------------------------------------------
}
#endif

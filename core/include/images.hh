/*
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
*/
#ifndef ATLAS_IMAGES_HH
#define ATLAS_IMAGES_HH

// C++標準ライブラリ
#include <cstdint>  // std::uint8_t

// Arduino
#include <Arduino.h> // PROGMEM

namespace atlas {
namespace img {
//-----------------------------------------------------------------------------

//! さめ大臣ロゴ, 76x52px
extern const std::uint8_t sharkMinisterLogo[] PROGMEM;

//! オートモードでのベイバトルパスの接続を促すメッセージ, 119x22px
extern const std::uint8_t promotion_BBP[] PROGMEM;

// //! オートモードアイコン, 14x15px
// extern const std::uint8_t modeA[] PROGMEM;

// //! マニュアルモードアイコン, 14x15px
// extern const std::uint8_t modeM[] PROGMEM;

//! オートモードアイコン（1行高）, 14x8px
extern const std::uint8_t modeA[] PROGMEM;

//! マニュアルモードアイコン（1行高）, 14x8px
extern const std::uint8_t modeM[] PROGMEM;

// 'BattlePass_Icon', 10x8px
extern const std::uint8_t bbpIcon [] PROGMEM;

// 'Bey_Icon', 10x8px
extern const std::uint8_t beyIcon [] PROGMEM;

// 'Client_Icon', 10x8px
extern const std::uint8_t clientIcon [] PROGMEM;

// 'ELR1', 18x8px
extern const std::uint8_t elr1Icon [] PROGMEM;

// 'ELR2', 18x8px
extern const std::uint8_t elr2Icon [] PROGMEM;

// 'max', 6x8px
extern const std::uint8_t maxSymbol [] PROGMEM;

// 'min', 6x8px
extern const std::uint8_t minSymbol [] PROGMEM;

// 'mu', 6x8px
extern const std::uint8_t muSymbol [] PROGMEM;

// 'sigma', 6x8px
extern const std::uint8_t sigmaSymbol [] PROGMEM;

// 'Plus Minus', 6x8px
extern const std::uint8_t pmSymbol[] PROGMEM;

// RANGE, 24x8px
extern const std::uint8_t rangeSymbol [] PROGMEM;

// 'crc-error', 98x26px
extern const std::uint8_t crcError [] PROGMEM;

// 'ELR_Canceled', 92x26px
extern const std::uint8_t elrCanceled [] PROGMEM;

//! カウントダウン用'Go-', 50x21px
extern const std::uint8_t cndGo[] PROGMEM;

//! カウントダウン用'Ready Set', 57x29px
extern const std::uint8_t cndReadyset[] PROGMEM;

//! カウントダウン用'Shoot!', 97x21px
extern const std::uint8_t cndShoot[] PROGMEM;

//-----------------------------------------------------------------------------
} // namespace img
} // namespace atlas

#endif

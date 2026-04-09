/*
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
*/
#ifndef ATLAS_IMAGE_NUMBER_HH
#define ATLAS_IMAGE_NUMBER_HH

// C++標準ライブラリ
#include <cstdint>  // std::uint8_t

// Arduino
#include <Arduino.h> // PROGMEM

namespace shark {
//-----------------------------------------------------------------------------

//! '0', 18x24px
extern const std::uint8_t digitW18_0[] PROGMEM;
// '1', 18x24px
extern const std::uint8_t digitW18_1[] PROGMEM;
// '2', 18x24px
extern const std::uint8_t digitW18_2[] PROGMEM;
// '3', 18x24px
extern const std::uint8_t digitW18_3[] PROGMEM;
//! '4', 18x24px
extern const std::uint8_t digitW18_4[] PROGMEM;
//! '5', 18x24px
extern const std::uint8_t digitW18_5[] PROGMEM;
//! '6', 18x24px
extern const std::uint8_t digitW18_6[] PROGMEM;
//! '7', 18x24px
extern const std::uint8_t digitW18_7[] PROGMEM;
//! '8', 18x24px
extern const std::uint8_t digitW18_8[] PROGMEM;
//! '9', 18x24px
extern const std::uint8_t digitW18_9[] PROGMEM;
// 数字の配列
extern const std::uint8_t* digitsW18[10];

//! '0', 9x14px
extern const std::uint8_t digitW9_0 [] PROGMEM;
//! '1', 9x14px
extern const std::uint8_t digitW9_1 [] PROGMEM;
//! '2', 9x14px
extern const std::uint8_t digitW9_2 [] PROGMEM;
//! '3', 9x14px
extern const std::uint8_t digitW9_3 [] PROGMEM;
//! '4', 9x14px
extern const std::uint8_t digitW9_4 [] PROGMEM;
//! '5', 9x14px
extern const std::uint8_t digitW9_5 [] PROGMEM;
//! '6', 9x14px
extern const std::uint8_t digitW9_6 [] PROGMEM;
//! '7', 9x14px
extern const std::uint8_t digitW9_7 [] PROGMEM;
//! '8', 9x14px
extern const std::uint8_t digitW9_8 [] PROGMEM;
//! '9', 9x14px
extern const std::uint8_t digitW9_9 [] PROGMEM;
// 数字の配列
extern const std::uint8_t* digitsW9[10];

//-----------------------------------------------------------------------------
} // namespace shark

#endif

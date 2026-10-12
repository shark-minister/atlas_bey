/*
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
*/
#ifndef ATLAS_UTILS_HH
#define ATLAS_UTILS_HH

// Arduino
#include <Arduino.h>

// ATLAS
#include "setting.hh"

//-----------------------------------------------------------------------------

typedef unsigned long ulong;  // 時刻用

// デバッグ用メッセージ出力
template <typename T>
inline void debugMsg(T msg)
{
#if BUILD_TYPE != BUILD_RELEASE
    Serial.println(msg);
#endif
}

template <typename T>
inline std::size_t writeFile(File& file, T* data, std::size_t size)
{
    return file.write(
        reinterpret_cast<const std::uint8_t*>(data),
        size
    );
}

template <typename T>
inline std::size_t writeFile(File& file, T value)
{
    return file.write(
        reinterpret_cast<const std::uint8_t*>(&value),
        sizeof(T)
    );
}

template <typename T>
inline std::size_t readFile(File& file, T& value)
{
    return file.read(
        reinterpret_cast<std::uint8_t*>(&value),
        sizeof(T)
    );
}

//-----------------------------------------------------------------------------

#endif  // #ifndef ATLAS_UTILS_HH

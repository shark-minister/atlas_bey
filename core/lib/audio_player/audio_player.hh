/*
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
*/
#ifndef SHARK_MINISTER_AUDIO_PLAYER_HH
#define SHARK_MINISTER_AUDIO_PLAYER_HH

// C++標準ライブラリ
#include <cstdint>      // std::uint8_t
#include <memory>       // std::unique_ptr

// Arduino
#include <HardwareSerial.h>
#include <DFRobotDFPlayerMini.h>

namespace shark {
//-----------------------------------------------------------------------------

class AudioPlayer
{
private:
    std::unique_ptr<DFRobotDFPlayerMini> _dfplayer;

public:
    /*!
        @brief  オーディオプレイヤーの開始

        @param[in]  serial  ハードウェアシリアルへの参照
        @param[in]  baud    ボーレート
        @param[in]  pinRX   RXのピン番号
        @param[in]  pinTX   TXのピン番号

        @return  開始の成否
    */
    bool begin(HardwareSerial& serial,
               unsigned long baud,
               std::int8_t pinRX,
               std::int8_t pinTX);

    /*!
        @brief  音量の設定
        @param[in]  volume  音量。1-30
    */
    void setVolume(std::uint8_t volume);

    //! オーディオプレイヤーが有効かどうかを返す
    bool isEnabled() const noexcept;

    //! 音声を再生する
    void play(std::uint8_t fileNumber);
};

//-----------------------------------------------------------------------------
} // namespace shark
#endif

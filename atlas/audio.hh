/*
    ATLAS - AuTomatic LAuncher System
    Copyright (C) 2025  Shark Minister
    All right reserved.

    https://x.com/shark_minister
*/
#ifndef ATLAS_AUDIO_HH
#define ATLAS_AUDIO_HH

// C++標準ライブラリ
#include <cstdint>      // std::uint8_t
#include <memory>

// Arduino
#include <DFRobotDFPlayerMini.h>  // オーディオ制御
#include <HardwareSerial.h>

// ATLAS
#include "setting.hh"

namespace atlas
{
//-----------------------------------------------------------------------------

class AudioPlayer
{
private:
    std::unique_ptr<DFRobotDFPlayerMini> _dfplayer;

public:
    bool begin(HardwareSerial& serial,
               unsigned long baud,
               std::int8_t rx_pin = AUDIO_RX,
               std::int8_t tx_pin = AUDIO_TX)
    {
        serial.begin(baud, SERIAL_8N1, rx_pin, tx_pin);
        if (!serial)
        {
            return false;
        }
        // 音声制御のインスタンス実体化
        _dfplayer.reset(new DFRobotDFPlayerMini);
        if (_dfplayer->begin(serial))
        {
            this->set_volume(DEFAULT_VOLUME);
            return true;
        }
        _dfplayer.reset();
        return false;
    }

    inline void set_volume(std::uint8_t volume)
    {
        if (volume <= 30)
        {
            _dfplayer->volume(volume);
        }
    }

    inline bool enabled() const noexcept
    {
        return static_cast<bool>(_dfplayer);
    }

    inline void se_ack()
    {
        this->_play(AUDIO_SE_ACK);
    }

    inline void se_cancel()
    {
        this->_play(AUDIO_SE_CANCEL);
    }

    inline void se_error()
    {
        this->_play(AUDIO_SE_ERROR);
    }

    inline void countdown()
    {
        this->_play(AUDIO_COUNTDOWN);
    }

private:
    inline void _play(int number)
    {
        if (_dfplayer)
        {
            _dfplayer->playFolder(number, 1);
        }
    }
};

//-----------------------------------------------------------------------------
} // namespace atlas
#endif

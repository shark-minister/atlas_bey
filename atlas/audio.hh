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
#include <memory>       // std::unique_ptr

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
    /*!
        @brief  オーディオプレイヤーの開始

        @param[in]  serial  ハードウェアシリアルへの参照
        @param[in]  baud    ボーレート
        @param[in]  rx_pin  RXのピン番号
        @param[in]  tx_pin  TXのピン番号

        @return  開始の成否
    */
    bool begin(HardwareSerial& serial,
               unsigned long baud = 9600,
               std::int8_t rx_pin = AUDIO_RX,
               std::int8_t tx_pin = AUDIO_TX)
    {
        // シリアル通信の初期化
        serial.begin(baud, SERIAL_8N1, rx_pin, tx_pin);
        if (!serial)
        {
            return false;
        }
        // DFプレイヤーのインスタンス実体化
        _dfplayer.reset(new DFRobotDFPlayerMini);
        if (!_dfplayer->begin(serial))
        {
            // DFプレイヤーの初期化に失敗したので、インスタンスの破棄
            _dfplayer.reset();
            return false;
        }
        // 音量の設定
        this->set_volume(DEFAULT_VOLUME);
        return true;
    }

    /*!
        @brief  音量の設定
        @param[in]  volume  音量。1-30
    */
    inline void set_volume(std::uint8_t volume)
    {
        if (volume <= 30)
        {
            _dfplayer->volume(volume);
        }
    }

    //! オーディオプレイヤーが有効かどうかを返す
    inline bool enabled() const noexcept
    {
        return static_cast<bool>(_dfplayer);
    }

    //! ACK音を再生する
    inline void se_ack()
    {
        this->_play(AUDIO_SE_ACK);
    }

    //! キャンセル音を再生する
    inline void se_cancel()
    {
        this->_play(AUDIO_SE_CANCEL);
    }

    //! エラー音を再生する
    inline void se_error()
    {
        this->_play(AUDIO_SE_ERROR);
    }

    //! カウントダウン音声を再生する
    inline void countdown()
    {
        this->_play(AUDIO_COUNTDOWN);
    }

private:
    /*!
        @brief  音声・SEの再生
        @param[in]  number  ファイルナンバー
    */
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

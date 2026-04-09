/*
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
*/
#include "audio_player.hh"

namespace shark {
//-----------------------------------------------------------------------------

// オーディオプレイヤーの開始
bool AudioPlayer::begin(
    HardwareSerial& serial,
    unsigned long baud,
    std::int8_t rxPin,
    std::int8_t txPin
) {
    // シリアル通信の初期化
    serial.begin(baud, SERIAL_8N1, rxPin, txPin);
    if (!serial) {
        return false;
    }
    // DFプレイヤーのインスタンス実体化
    _dfplayer.reset(new DFRobotDFPlayerMini);
    if (!_dfplayer->begin(serial)) {
        // DFプレイヤーの初期化に失敗したので、インスタンスの破棄
        _dfplayer.reset();
        return false;
    }
    return true;
}

// 音量の設定
void AudioPlayer::setVolume(std::uint8_t volume)
{
    if (_dfplayer && volume <= 30) {
        _dfplayer->volume(volume);
    }
}

// オーディオプレイヤーが有効かどうかを返す
bool AudioPlayer::isEnabled() const noexcept
{
    return static_cast<bool>(_dfplayer);
}

// 音声を再生する
void AudioPlayer::play(std::uint8_t fileNumber)
{
    if (_dfplayer) {
        _dfplayer->playFolder(fileNumber, 1);
    }
}

//-----------------------------------------------------------------------------
} // namespace shark


/*
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
*/
#ifndef SHARK_MINISTER_MOTOR_HH
#define SHARK_MINISTER_MOTOR_HH

// C++標準ライブラリ
#include <cstdint>  // std::uint8_t

namespace shark {
//-----------------------------------------------------------------------------

//! モーター制御の補助を行うクラス
class MotorDriver
{
public:
    /*!
        @brief  設定
        @param[in]  pinPwmL     左回転PWMのピン番号
        @param[in]  pinPwmR     右回転PWMのピン番号
        @param[in]  pinEnabled  左右EN
        @param[in]  maxSP       最大シュートパワー
    */
    void configure(std::uint8_t pinPwmL,
                   std::uint8_t pinPwmR,
                   std::uint8_t pinEnabled,
                   int maxRPM);

    //! リセット
    void reset();
    
    //! モーターの急停止を行う
    void stop();
    
    /*!
        @brief  指定したSPまでモーターを回転する
        @param[in]  sp       シュートパワー
        @param[in]  isRight  右回転かどうか（デフォルトtrue）
    */
    void rotate(int sp, bool isRight = true);

    static void setDummyMode();

private:
    std::uint8_t _pwmL = 0;       //!< 左回転PWMのピン番号
    std::uint8_t _pwmR = 0;       //!< 右回転PWMのピン番号
    std::uint8_t _enabledLR = 0;  //!< 左右EN
    std::uint8_t _curPwm = 0;     //!< 現在のPWMピン番号
    int _maxSP = 0;               //!< 最大シュートパワー

    //! ダミーモードかどうか
    static bool _dummyMode;
};

//-----------------------------------------------------------------------------
} // namespace shark
#endif
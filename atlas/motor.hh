/*
    ATLAS - AuTomatic LAuncher System
    Copyright (C) 2025  Shark Minister
    All right reserved.

    https://x.com/shark_minister
*/
#ifndef ATLAS_MOTOR_HH
#define ATLAS_MOTOR_HH

// C++標準ライブラリ
#include <cstdint>  // std::uint8_t

namespace atlas
{
//-----------------------------------------------------------------------------

//! モーター制御の補助を行うクラス
class Motor
{
public:
    /*!
        @brief  設定
        @param[in]  pin_L_PWM  左回転PWMのピン番号
        @param[in]  pin_R_PWM  右回転PWMのピン番号
        @param[in]  pin_LR_EN  左右EN
        @param[in]  max_sp     最大シュートパワー
    */
    void configure(std::uint8_t pin_L_PWM,
                   std::uint8_t pin_R_PWM,
                   std::uint8_t pin_LR_EN,
                   int max_rpm);

    //! リセット
    void reset();
    
    //! モーターの急停止を行う
    void stop();
    
    /*!
        @brief  指定したSPまでモーターを回転する
        @param[in]  sp        シュートパワー
        @param[in]  is_right  右回転かどうか（デフォルト）
    */
    inline void rotate(int sp,
                       bool is_right = true)
    {
        this->_rotate_impl(is_right ? _r_pwm : _l_pwm, sp);
    }

protected:
    //! 左回転PWMのピン番号
    std::uint8_t _l_pwm = 0;
    
    //! 右回転PWMのピン番号
    std::uint8_t _r_pwm = 0;
    
    //! 左右EN
    std::uint8_t _lr_en = 0;
    
    //! 現在のPWMピン番号
    std::uint8_t _cur_pwm = 0;
    
    //! 最大シュートパワー
    int _max_sp = 0;

    /*!
        @brief  回転操作の実装
        @param[in]  pin_pwm  PWMのピン番号
        @param[in]  sp       シュートパワー
    */
    void _rotate_impl(std::uint8_t pin_pwm, int sp);
};

//-----------------------------------------------------------------------------
} // namespace atlas
#endif

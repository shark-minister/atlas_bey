/*
    ATLAS - AuTomatic LAuncher System
    Copyright (C) 2025  Shark Minister
    All right reserved.

    https://x.com/shark_minister
*/
#include "motor.hh"

// Arduino
#include <Arduino.h>

// ATLAS
#include "setting.hh"

namespace atlas
{
//-----------------------------------------------------------------------------

void Motor::configure(std::uint8_t pin_L_PWM,
                      std::uint8_t pin_R_PWM,
                      std::uint8_t pin_LR_EN,
                      int max_rpm)
{
    _l_pwm = pin_L_PWM;
    _r_pwm =pin_R_PWM;
    _lr_en = pin_LR_EN;
    _cur_pwm =pin_R_PWM;
    _max_sp = max_rpm;

    // 全てのピンを出力モードに
    pinMode(_r_pwm, OUTPUT);
    pinMode(_l_pwm, OUTPUT);
    pinMode(_lr_en, OUTPUT);
}

void Motor::reset()
{
    if constexpr (USE_DUMMY_MOTOR > 0)
    {
        Serial.println("Motor::reset()");
    }
    else
    {
        digitalWrite(_l_pwm, LOW);
        digitalWrite(_r_pwm, LOW);
        digitalWrite(_lr_en, LOW);
    }
}

void Motor::stop()
{
    if constexpr (USE_DUMMY_MOTOR > 0)
    {
        Serial.println("Motor::stop()");
    }
    else
    {
        analogWrite(_cur_pwm, 0);
    }
}

void Motor::_rotate_impl(std::uint8_t pwm, int sp)
{
    // 設定した最大SPを超えないように調整
    if (sp > _max_sp)
    {
        sp = _max_sp;
    }
    // 最小SPを1として、下回らないように調整
    else if (sp < 1)
    {
        sp = 1;
    }

    if constexpr (USE_DUMMY_MOTOR > 0)
    {
        Serial.print("Motor::rotate_*(): pwm = ");
        Serial.print(pwm);
        Serial.print(", SP = ");
        Serial.println(sp);
    }
    else
    {
        // L_EN, R_ENをHIGHにする。これをしないと回らない
        digitalWrite(_lr_en, HIGH);
        
        // PWMデューティ比を求める
        int sp_duty = static_cast<int>((static_cast<double>(sp-1) / _max_sp) * 256);

        // 所定のデューティ比まで段階的に加速する
        for (int duty = 1; duty <= sp_duty; duty += 1)
        {
            analogWrite(pwm, duty);
            delay(15);
        }
        analogWrite(pwm, sp_duty);
    }

    // PWMピン番号を記憶する
    _cur_pwm = pwm;
}

//-----------------------------------------------------------------------------
} // namespace atlas

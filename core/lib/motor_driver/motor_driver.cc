/*
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
*/
#include "motor_driver.hh"

// Arduino
#include <Arduino.h>

namespace shark {
//-----------------------------------------------------------------------------

bool MotorDriver::_dummyMode = false;

void MotorDriver::configure(
    std::uint8_t pinPwmL,
    std::uint8_t pinPwmR,
    std::uint8_t pinEnabled,
    int maxRPM
) {
    _pwmL = pinPwmL;
    _pwmR = pinPwmR;
    _enabledLR = pinEnabled;
    _curPwm = pinPwmR;
    _maxSP = maxRPM;

    if (!_dummyMode) {
        // 全てのピンを出力モードに
        pinMode(_pwmR, OUTPUT);
        pinMode(_pwmL, OUTPUT);
        pinMode(_enabledLR, OUTPUT);
    }
}

void MotorDriver::reset()
{
    if (_dummyMode) {
        Serial.println("reset motor");
    }
    else {
        digitalWrite(_pwmL, LOW);
        digitalWrite(_pwmR, LOW);
        digitalWrite(_enabledLR, LOW);
    }
}

void MotorDriver::stop()
{
    if (_dummyMode) {
        Serial.println("stop motor");
    }
    else {
        analogWrite(_curPwm, 0);
    }
}

void MotorDriver::rotate(int sp, bool isRight)
{
    std::uint8_t pwm = isRight ? _pwmR : _pwmL;

    // 設定した最大SPを超えないように調整
    if (sp > _maxSP) {
        sp = _maxSP;
    }
    // 最小SPを1として、下回らないように調整
    else if (sp < 1) {
        sp = 1;
    }

    if (_dummyMode) {
        Serial.println("rotate motor");
    }
    else {
        // L_EN, R_ENをHIGHにする。これをしないと回らない
        digitalWrite(_enabledLR, HIGH);
        
        // PWMデューティ比を求める
        int spDuty = static_cast<int>(
            (static_cast<double>(sp-1) / _maxSP) * 256
        );

        // 所定のデューティ比まで段階的に加速する
        for (int duty = 1; duty < spDuty; duty += 1) {
            analogWrite(pwm, duty);
            delay(15);
        }
        analogWrite(pwm, spDuty);
    }

    // PWMピン番号を記憶する
    _curPwm = pwm;
}

void MotorDriver::setDummyMode()
{
    _dummyMode = true;
    Serial.println("set dummy motor");
}

//-----------------------------------------------------------------------------
} // namespace shark

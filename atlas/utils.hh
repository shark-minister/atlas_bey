/*
    ATLAS - AuTomatic LAuncher System
    Copyright (C) 2025  Shark Minister
    All right reserved.

    https://x.com/shark_minister
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
inline void debug_msg(T msg)
{
#ifdef DEBUG_MODE
    Serial.println(msg);
#endif
}

// 指定時刻まで待つ
void wait_until(ulong expire, ulong cycle = 5)
{
    while (millis() < expire) delay(cycle);
}

// 各タスクが中止信号を監視しつつ待機する
inline bool standby(QueueHandle_t que, ulong latency)
{
    int sig = 0;
    return xQueueReceive(que, &sig, latency / portTICK_RATE_MS) == pdFALSE;
}

//-----------------------------------------------------------------------------

#endif  // #ifndef ATLAS_UTILS_HH

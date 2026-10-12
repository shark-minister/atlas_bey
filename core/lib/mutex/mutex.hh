/*
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
*/
#ifndef SHARK_MINISTER_MUTEX_HH
#define SHARK_MINISTER_MUTEX_HH

// Arduino
#include <Arduino.h>

namespace shark {
//-----------------------------------------------------------------------------

class Mutex
{
public:
    inline Mutex()
        : _smp(xSemaphoreCreateBinary()) {
        // セマフォの開放（これをしないと機能しない）
        xSemaphoreGive(_smp);
    }
    inline void aquire() {
        // セマフォの取得
        xSemaphoreTake(_smp, portMAX_DELAY);
    }
    inline void release() {
        // セマフォの開放
        xSemaphoreGive(_smp);
    }

private:
    SemaphoreHandle_t _smp;
};

//-----------------------------------------------------------------------------
} // namespace shark
#endif

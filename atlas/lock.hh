/*
    ATLAS - AuTomatic LAuncher System
    Copyright (C) 2025  Shark Minister
    All right reserved.

    https://x.com/shark_minister
*/
#ifndef ATLAS_LOCK_HH
#define ATLAS_LOCK_HH

// Arduino
#include <Arduino.h>

namespace atlas
{
//-----------------------------------------------------------------------------

//! セマフォオブジェクトに対してロック操作を行うためのクラス
class Lock
{
private:
    SemaphoreHandle_t _smp;  //!< セマフォオブジェクト（ポインタ）

public:
    //! コンストラクタ。生成時にセマフォの獲得を行う
    Lock(SemaphoreHandle_t smp)
        : _smp(smp)
    {
        // セマフォの取得
        xSemaphoreTake(_smp, portMAX_DELAY);
    }
    //! デストラクタ。破棄時にセマフォの開放を行う
    ~Lock()
    {
        // セマフォの開放
        xSemaphoreGive(_smp);
    }
};

//-----------------------------------------------------------------------------
} // namespace atlas
#endif

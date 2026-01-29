/*
    ATLAS - AuTomatic LAuncher System
    Copyright (C) 2025  Shark Minister
    All right reserved.

    https://x.com/shark_minister
*/
#ifndef ATLAS_MODE_HH
#define ATLAS_MODE_HH

// Arduino
#include <Arduino.h>

// ATLAS
#include "lock.hh"

namespace atlas
{
//-----------------------------------------------------------------------------

class Mode
{
public:
    //! コンストラクタ
    Mode()
        : _is_auto_mode(true)
        , _smp(xSemaphoreCreateBinary())
    {
        // セマフォの開放（これをしないと機能しない）
        xSemaphoreGive(_smp);
    }

    //! モード切替
    inline void change()
    {
        Lock lock(_smp);
        _is_auto_mode = !_is_auto_mode;
    }

    //! モードの設定
    inline void set(bool auto_mode)
    {
        Lock lock(_smp);
        _is_auto_mode = auto_mode;
    }

    //! オートモードかどうかを返す
    inline bool is_auto_mode() const
    {
        Lock lock(_smp);
        return _is_auto_mode;
    }

    //! マニュアルモードかどうかを返す
    inline bool is_manual_mode() const
    {
        Lock lock(_smp);
        return !_is_auto_mode;
    }

private:
    bool _is_auto_mode;      //!< オートモードかどうかのフラグ
    SemaphoreHandle_t _smp;  //!< セマフォ
};

//-----------------------------------------------------------------------------
} // namespace atlas
#endif

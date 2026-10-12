/*
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
*/
#ifndef SHARK_MINISTER_LOCK_HH
#define SHARK_MINISTER_LOCK_HH

#include "mutex.hh"

namespace shark {
//-----------------------------------------------------------------------------

//! mutexオブジェクトに対してロック操作を行うためのクラス
class Lock
{
private:
    Mutex& _mutex;  //!< セマフォオブジェクト（ポインタ）

public:
    //! コンストラクタ。生成時にセマフォの獲得を行う
    inline Lock(Mutex& mutex)
        : _mutex(mutex) {
        _mutex.aquire();
    }
    //! デストラクタ。破棄時にセマフォの開放を行う
    inline ~Lock() {
        _mutex.release();
    }
};

//-----------------------------------------------------------------------------
} // namespace shark
#endif

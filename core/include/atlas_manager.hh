/*
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
*/
#ifndef ATLAS_MANAGER_HH
#define ATLAS_MANAGER_HH

// shark lib
#include "mutex.hh"
#include "audio_player.hh"  // オーディオ制御
#if ATLAS_FORMAT == ATLAS_FULL_SPEC
#include "motor_driver.hh"  // モーター制御
#endif

// ATLAS
#include "setting.hh"
#include "result.hh"    // 解析結果
#include "params.hh"    // パラメータ
#include "state.hh"
#include "view.hh"      // 画面表示

namespace atlas {
//-----------------------------------------------------------------------------

class AtlasManager
{
public:
    inline static AtlasManager& instance() {
        return _instance;
    }

    void setup();
    void run();

    /*!
        @brief  モードの設定
        @param[in]  isAutoMode  オートモードか否か
    */
    void setMode(bool isAutoMode);

    //! オートモードかどうかを返す
    bool isAutoMode() const;

    //! マニュアルモードかどうかを返す
    bool isManualMode() const;

    //! モードを切り替える
    void switchMode();

    //! 統計情報を取得
    const Statistics& statistics() const noexcept;

private:
    //! インスタンス
    static AtlasManager _instance;

public:
    //! 音声制御
    shark::AudioPlayer player;

    // モーター制御インスタンス
#if ATLAS_FORMAT == ATLAS_FULL_SPEC  // 電動ランチャー制御として使う場合
    shark::MotorDriver motors[NUM_MOTORS];
#endif

    //! 統計データ
    Result result;

    //! 制御パラメータ
    Params params;

    //! 接続状態
    State state;

    //! 画面表示
    View view;

protected:
    AtlasManager();

private:
    //! オートモードかどうかのフラグ
    bool _isAutoMode = true;

    //! 排他制御
    mutable shark::Mutex _mutexMode;
};

extern AtlasManager& ATLAS;

//-----------------------------------------------------------------------------
} // namespace atlas
#endif

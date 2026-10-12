/*
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
*/
#include "atlas_manager.hh"

// Arduino
#include <Arduino.h>
#include <SPIFFS.h>     // フラッシュメモリをファイル保存に使う

// shark lib
#include "lock.hh"

// ATLAS
#include "utils.hh"
#include "mode_process.hh"

namespace atlas
{
//-----------------------------------------------------------------------------

//=============================================================================
//
// タスク
//
//=============================================================================

void taskSwitchMonitor(void* pvParams)
{
//-----------------------------------------------------------------------------
#if SWITCH_TYPE == SW_TACT // タクトスイッチを使う場合
//-----------------------------------------------------------------------------

    bool btnState = HIGH;          // ボタンの状態
    bool lastPhysState = HIGH;     // 最後のボタン状態
    bool longPressFired = false;
    std::uint32_t tLastDebounce = 0;
    std::uint32_t tPressStart = 0;

    TickType_t tLastWake = xTaskGetTickCount();

    while (true)
    {
        // ボタンの状態を取得
        bool reading = digitalRead(SW_SEL_IN);

        // デバウンス検出用
        if (reading != lastPhysState) {
            tLastDebounce = millis();
        }

        // ボタンイベントあり
        if ((millis() - tLastDebounce) > SW_DEBOUNCE_MS) {
            // ボタン状態の変化あり
            if (reading != btnState) {
                btnState = reading;

                // ボタン押下
                if (btnState == LOW) {
                    tPressStart = millis();   // 押下開始
                    longPressFired = false;
                }
                else if (!longPressFired) {
                    // オートモードにおいて、BBPを接続しているとき
                    if (ATLAS.isAutoMode()) {
                        if (ATLAS.state.isBBPReady()) {
                            ATLAS.state.nextPageA();
                            ATLAS.view.autoModeStandby();
                        }
                    }
                    // マニュアルモードにおいて
                    else {
                        ATLAS.state.nextPageM();
                        ATLAS.view.manualModeStandby();
                    }
                }
            }
        }
        lastPhysState = reading;

        // 長押し判定
        if (btnState == LOW  &&    // ボタン押下状態
            !longPressFired &&     // 長押しが決定していない状態
            ((millis() - tPressStart) > SW_LONGPRESS_MS)   // 長押し時間超過
        ) {
            // モード切り替え
            ATLAS.switchMode();

            // 後処理
            longPressFired = true;
        }

        vTaskDelayUntil(&tLastWake, pdMS_TO_TICKS(SW_POLL_INTERVAL_MS));
    }

//-----------------------------------------------------------------------------
#elif SWITCH_TYPE == SW_SLIDE // スライドスイッチを使う場合
//-----------------------------------------------------------------------------

    // スイッチの出力用設定
    pinMode(SW_SEL_OUT, OUTPUT);
    digitalWrite(SW_SEL_OUT, HIGH);
    
    while (true) {
        ATLAS.setMode(digitalRead(SW_SEL_IN) == 1);
        delay(100);   // 100ms間隔でチェック
    }

//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------

    // タスク終了処理
    vTaskDelete(nullptr);
}

//=============================================================================
//
// AtlasManager
//
//=============================================================================

AtlasManager AtlasManager::_instance;
AtlasManager& ATLAS = AtlasManager::instance();

AtlasManager::AtlasManager()
{
#if ATLAS_FORMAT == ATLAS_FULL_SPEC && BUILD_TYPE == BUILD_DEBUG
    shark::MotorDriver::setDummyMode();
#endif
}

void AtlasManager::setup()
{
#if BUILD_TYPE != BUILD_RELEASE
    // シリアル通信開始
    Serial.begin(9600);
    while (!Serial);
#endif

    // ディスプレイの開始
    if (!this->view.begin(SCREEN_ADDR)) {
        debugMsg(F("failed to start display"));
        while (true);
    }

    // さめ大臣ロゴ
    this->view.splashScreen();
    TickType_t tLogoBegin = xTaskGetTickCount();

    // SPIFFS開始
    if (!SPIFFS.begin(true)) {
        debugMsg(F("failed to mount SPIFFS"));
        while (true);
    }

    // 解析結果の読み込み
    this->result.initialize();
    if (File file = SPIFFS.open(RESULT_FPATH, "r")) {
        if (file.size() == sizeof(this->result)) {
            // ファイルの内容を読み込む
            readFile(file, this->result);
            debugMsg(F("read statistics file"));
        }
        file.close();
    }

    // パラメータの読み込み
    this->params.initialize();
    if (File file = SPIFFS.open(PARAMS_FPATH, "r")) {
        if (file.size() == sizeof(this->params)) {
            // ファイルの内容を読み込む
            readFile(file, this->params);
            debugMsg(F("read parameter file"));
        }
        file.close();
    }
    this->params.regulate();

//-----------------------------------------------------------------------------
#if ATLAS_FORMAT == ATLAS_FULL_SPEC  // 電動ランチャー制御として使う場合
//-----------------------------------------------------------------------------

    // 音声制御の開始
    if (!this->player.begin(Serial1, 9600, AUDIO_RX, AUDIO_TX)) {
        debugMsg(F("failed to start audio player"));
    }
    this->player.setVolume(DEFAULT_VOLUME);
    
    // モーター制御インスタンスの設定
    this->motors[0].configure(
        MOTOR1_PWM_L,
        MOTOR1_PWM_R,
        MOTOR_ENBL,
        MOTOR1_MAX_RPM
    );
#if NUM_MOTORS == 2
    this->motors[1].configure(
        MOTOR2_PWM_L,
        MOTOR2_PWM_R,
        MOTOR_ENBL,
        MOTOR2_MAX_RPM
    );
#endif

//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------

    // スイッチ用のピン設定（内蔵プルアップ抵抗を使う）
    pinMode(SW_SEL_IN, INPUT_PULLUP);

    // スイッチの入力を監視するタスクの生成・投入
    xTaskCreate(
        taskSwitchMonitor,  // タスク
        "taskSwitch",       // タスク名
        2048,               // スタックメモリ
        nullptr,            // 起動パラメータ
        1,                  // 優先度（値が大きいほど優先順位が高い）
        nullptr             // タスクハンドル
    );

    // スプラッシュスクリーン表示限度まで待機実行
    vTaskDelayUntil(&tLogoBegin, pdMS_TO_TICKS(1500));
}

void AtlasManager::setMode(bool isAutoMode)
{
    shark::Lock lock(_mutexMode);
    _isAutoMode = isAutoMode;
}

// オートモードかどうかを返す
bool AtlasManager::isAutoMode() const
{
    shark::Lock lock(_mutexMode);
    return _isAutoMode;
}

// マニュアルモードかどうかを返す
bool AtlasManager::isManualMode() const
{
    shark::Lock lock(_mutexMode);
    return !_isAutoMode;
}

void AtlasManager::switchMode()
{
    shark::Lock lock(_mutexMode);
    _isAutoMode = !_isAutoMode;
}

const Statistics& AtlasManager::statistics() const noexcept
{
    switch (this->params.mainSPView()) {
    case MainSPView::EVAL_SP:
        return ATLAS.result.statsEval;
    case MainSPView::ORIG_SP:
        return ATLAS.result.statsOrig;
    }
    return ATLAS.result.statsOrig;
}

void AtlasManager::run()
{
    this->isAutoMode() ? runAutoMode() : runManualMode();
}

//-----------------------------------------------------------------------------
} // namespace atlas

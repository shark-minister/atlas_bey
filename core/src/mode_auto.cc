/*
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
*/
#include "mode_process.hh"

// C++標準ライブラリ
#include <cstring>
#if ATLAS_FORMAT == ATLAS_FULL_SPEC  // 電動ランチャー制御として使う場合
#include <atomic>   // std::atomic_ulong
#endif

// Shark Lib
#include "bbp_analyzer.hh"
#include "statistics.hh"

// Arduino
#include <Arduino.h>
#include <NimBLEDevice.h>
#include <SPIFFS.h>     // フラッシュメモリをファイル保存に使う

// ATLAS
#include "atlas_manager.hh"
#include "utils.hh"
#include "images.hh"

namespace atlas
{
//-----------------------------------------------------------------------------

// BBPからのデータ解析準備
static shark::BBPAnalyzer gAnalyzer;

// スキャン結果保持用
static NimBLEAddress gFoundAddress;
static std::atomic_bool gDeviceFound = false;

// 接続状態
static std::atomic_bool gDisconnected = true;

// 自動発射状況
static std::atomic_bool gIsAbortable = false;

//=============================================================================
//
// タスク
//
//=============================================================================

//-----------------------------------------------------------------------------
#if ATLAS_FORMAT == ATLAS_FULL_SPEC  // 電動ランチャー制御として使う場合
//-----------------------------------------------------------------------------
// イベントグループ
static EventGroupHandle_t gEventGroup;

// タスク制御
#define BIT_START   (1 << 0)
#define BIT_ABORT   (1 << 1)
#define BIT_SHOOT   (1 << 2)

// カウントダウンマスタータスク
void taskCountMaster(void* pvParams)
{
    // イベントグループによるキャンセル処理
    gIsAbortable.store(true);                   // キャンセル可能に
    EventBits_t bits = xEventGroupWaitBits(     // キャンセル待ち
        gEventGroup,
        BIT_ABORT,                              // ABORT信号を待つ
        pdFALSE,
        pdFALSE,
        pdMS_TO_TICKS(ATLAS.params.latency())   // キャンセル可能時間
    );
    gIsAbortable.store(false);                  // キャンセル不可能に

    // キャンセル
    if (bits & BIT_ABORT) {
        debugMsg(F("canceled"));
    }
    // 継続
    else {
        // カウントダウン開始通知
        xEventGroupSetBits(gEventGroup, BIT_START);

        // 4秒カウント
        TickType_t lastWake = xTaskGetTickCount();
        const TickType_t interval = pdMS_TO_TICKS(1000);
        for (int i = 0; i < 4; ++i) {
            vTaskDelayUntil(&lastWake, interval);
        }

        // Shoot!のタイミングを全体通知
        xEventGroupSetBits(gEventGroup, BIT_SHOOT);
    }

    // タスク終了処理
    vTaskDelete(nullptr);
}

// モーター制御: オートモード
void taskMotorAuto(void* pvParams)
{
    // 中止・開始待ち
    EventBits_t bits = xEventGroupWaitBits(
        gEventGroup,
        BIT_START | BIT_ABORT,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY
    );

    // 中止
    if (bits & BIT_ABORT) {
       debugMsg(F("motor canceled"));
    }
    // 継続
    else {
        // 駆動するモーターインスタンスのキャッシュ
        auto& motor = ATLAS.motors[ATLAS.params.autoModeELRIndex()];

        // 回転開始
        const auto& elr = ATLAS.params.autoModeELR();
        motor.rotate(elr.sp(), elr.isRight());

        // SHOOT号令待機
        xEventGroupWaitBits(
            gEventGroup,
            BIT_SHOOT,
            pdTRUE,
            pdTRUE,
            portMAX_DELAY
        );

        // 同期調整時間 + 射出遅延時間
        vTaskDelay(pdMS_TO_TICKS(SYNC_ADJ_TIME + ATLAS.params.delay()));
        
        // モーターの急停止による射出実行
        motor.stop();
    }

    // タスク終了処理
    vTaskDelete(nullptr);
}

// 射出前カウントダウン表示
void taskCountView(void* pvParams)
{
    // "Ready Set"の表示
    ATLAS.view.autoModeCountdown(0);

    // 中止 or 開始待ち
    EventBits_t bits = xEventGroupWaitBits(
        gEventGroup,
        BIT_START | BIT_ABORT,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY
    );

    // 中止
    if (bits & BIT_ABORT) {
        // 中止信号が来たので中止のメッセージを表示する
        ATLAS.view.autoModeAborted();
    }
    // 継続
    else {
        // "3"の表示
        ATLAS.view.autoModeCountdown(1);

        // "2", "1", "Go"の表示
        TickType_t lastWake = xTaskGetTickCount();
        const TickType_t interval = pdMS_TO_TICKS(1000);
        for (int i = 2; i < 5; ++i) {
            vTaskDelayUntil(&lastWake, interval);
            ATLAS.view.autoModeCountdown(i);
        }

        // ZERO待ちで0表示
        xEventGroupWaitBits(
            gEventGroup,
            BIT_SHOOT,
            pdTRUE,
            pdTRUE,
            portMAX_DELAY
        );
        // 同期調整時間
        vTaskDelay(pdMS_TO_TICKS(SYNC_ADJ_TIME));

        // "SHOOT"の表示
        ATLAS.view.autoModeCountdown(5);
    }

    // タスク終了処理
    vTaskDelete(nullptr);
}

// 射出前カウントダウン音声
void taskCountVoice(void* pvParams)
{
    // 中止 or 開始待ち
    EventBits_t bits = xEventGroupWaitBits(
        gEventGroup,
        BIT_START | BIT_ABORT,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY
    );

    // 中止
    if (bits & BIT_ABORT) {
        // 中止SEを流す
        ATLAS.player.play(AUDIO_SE_CANCEL);
    }
    // 継続
    else {
        // カウントダウン音声を流す
        ATLAS.player.play(AUDIO_COUNTDOWN);
    }
    
    // タスク終了処理
    vTaskDelete(nullptr);
}

//-----------------------------------------------------------------------------
#endif  // #if ATLAS_FORMAT == ATLAS_FULL_SPEC
//-----------------------------------------------------------------------------

//=============================================================================
//
// コールバック
//
//=============================================================================
class ScanCallbacks
    : public NimBLEScanCallbacks
{
    void onResult(const NimBLEAdvertisedDevice* device) override
    {
        if (device->getName() == BBP_LOCAL_NAME) {
            debugMsg(F("BeyBattle pass found:"));
            debugMsg(device->getAddress().toString().c_str());

            // 結果の保持
            gFoundAddress = device->getAddress();
            gDeviceFound.store(true);

            NimBLEDevice::getScan()->stop();
        }
    }
};
static ScanCallbacks gScanCallbacks;

class ClientCallbacks
    : public NimBLEClientCallbacks
{
    void onConnect(NimBLEClient* client) override
    {
        Serial.println("client connected");
        gDisconnected.store(false);
    }

    void onDisconnect(NimBLEClient* pClient, int reason) override
    {
        Serial.println("client disconnected");
        gDisconnected.store(true);
    }
};
static ClientCallbacks gClientCallbacks;

// ベイがランチャーに装着された／ランチャーから外された
void onBeyAttachedOrDetached()
{
    debugMsg(F("beyblade has been attached / detached"));

    // 表示更新
    ATLAS.view.autoModeStandby();
}

// ベイが射出された
void onBeyLaunched()
{
    debugMsg(F("beyblade has been launched"));

    // 状態更新
    ATLAS.state.setBey(false);

    // 統計データ更新
    std::uint16_t acc1, acc2;
    ATLAS.result.update(gAnalyzer.sp(), gAnalyzer.raw(), acc1, acc2);

    // 表示更新
    ATLAS.view.autoModeSP(acc1, acc2);

    // 解析結果保存
    if (File file = SPIFFS.open(RESULT_FPATH, "w")) {
        writeFile(file, ATLAS.result);
        file.close();
    }

    // SPデータ保存（追記）
    if (File file = SPIFFS.open(RAW_FPATH, "a")) {
        if (file.size() < MAX_SIZE_SP_FILE) {
            writeFile(file, ATLAS.result.statsOrig.total);
            writeFile(file, gAnalyzer.sp());
            writeFile(file, ATLAS.result.statsEval.latestSP);
            writeFile(file, gAnalyzer.raw(), sizeof(std::uint16_t) * 32);
            file.close();
        }
    }

    // 解析情報クリア
    gAnalyzer.clear();
}

// BLE通信のCRCエラー
void onCRCError()
{
    debugMsg(F("CRC error"));

    // 状態更新
    ATLAS.state.setBey(false);

    // エラー表示
    ATLAS.view.autoModeError();

    // エラー音
    ATLAS.player.play(AUDIO_SE_ERROR);
}

//-----------------------------------------------------------------------------
#if ATLAS_FORMAT == ATLAS_FULL_SPEC  // 電動ランチャー制御として使う
//-----------------------------------------------------------------------------

// 電動ランチャーが有効になった
void onELREnabled()
{
    debugMsg(F("ELR enabled"));

    // 電動ランチャーを有効にする
    ATLAS.state.setELR(true);
    // 描画
    ATLAS.view.autoModeStandby();
    // ACK音
    ATLAS.player.play(AUDIO_SE_ACK);
}

// 電動ランチャーが無効になった
void onELRDisabled()
{
    debugMsg(F("ELR disabled"));

    // 電動ランチャーを有効にする
    ATLAS.state.setELR(false);
    // 描画
    ATLAS.view.autoModeStandby();
    // キャンセル音
    ATLAS.player.play(AUDIO_SE_CANCEL);
}

// 電動ランチャーを回転させ始める
void onLaunchStarted()
{
    debugMsg(F("beyblade will be launched after countdown"));

    // イベントグループのビットをクリア
    xEventGroupClearBits(gEventGroup, 0xFF);

    // マスタータスク
    xTaskCreate(
        taskCountMaster,   // タスク
        "taskCountMaster", // タスク名
        4096,              // スタックメモリ
        nullptr,           // 起動パラメータ
        1,                 // 優先度（値が大きいほど優先順位が高い）
        nullptr            // タスクハンドル
    );

    // モータータスクの投入
    xTaskCreate(
        taskMotorAuto,     // タスク
        "taskMotorAuto",   // タスク名
        4096,              // スタックメモリ
        nullptr,           // 起動パラメータ
        3,                 // 優先度（値が大きいほど優先順位が高い）
        nullptr            // タスクハンドル
    );

    // カウントダウン表示タスクの投入
    xTaskCreate(
        taskCountView,     // タスク
        "taskCountView",   // タスク名
        4096,              // スタックメモリ
        nullptr,           // 起動パラメータ
        2,                 // 優先度（値が大きいほど優先順位が高い）
        nullptr            // タスクハンドル
    );

    // カウントダウン音声タスクの投入
    if (ATLAS.player.isEnabled()) {
        xTaskCreate(
            taskCountVoice,    // タスク
            "taskCountVoice",  // タスク名
            4096,              // スタックメモリ
            nullptr,           // 起動パラメータ
            2,                 // 優先度（値が大きいほど優先順位が高い）
            nullptr            // タスクハンドル
        );
    }
}

// 射出がキャンセルされた
void onLaunchCanceled()
{
    debugMsg(F("beyblade launching will be canceled"));

    // 各タスクに中止信号を送る
    xEventGroupSetBits(gEventGroup, BIT_ABORT);
}

//-----------------------------------------------------------------------------
#else
//-----------------------------------------------------------------------------

// マニュアル/設定モードにスイッチされた
void onSwitchedToManualMode()
{
    debugMsg(F("switched to manual mode"));

    // モードをマニュアル/設定モードに切り替える
    ATLAS.setMode(false);
}

//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------

// ベイバトルパスからデータが来た
void onNotifyData(
    NimBLERemoteCharacteristic* ch,
    std::uint8_t* data,
    std::size_t length,
    bool isNotify
) {
    shark::BBPData bbpData;

    // コピー
    std::memcpy(bbpData.data(), data, shark::BBPData::LENGTH);

    // 値の解析
    auto bbpState = gAnalyzer.analyze(bbpData);
    ATLAS.state.setBey((static_cast<std::uint16_t>(bbpState) & 0x04) > 0);
    switch (bbpState) {
    case shark::BBPState::BEY_ATTACHED_S1:  // ベイがランチャーにセットされた
    case shark::BBPState::BEY_DETACHED_S1:  // ベイがランチャーから外れた
        onBeyAttachedOrDetached();
        break;
    case shark::BBPState::FINISHED: // ベイブレードがシュートされた
        onBeyLaunched();
        break;
    case shark::BBPState::ERROR:    // CRCエラー
        onCRCError();
        break;
//-----------------------------------------------------------------------------
#if ATLAS_FORMAT == ATLAS_FULL_SPEC  // 電動ランチャー制御として使う
//-----------------------------------------------------------------------------
    case shark::BBPState::TRANS_S1_TO_S2:   // BBPのボタンがダブルクリックされた
        onELREnabled();
        break;
    case shark::BBPState::TRANS_S2_TO_S1:   // BBPのボタンがダブルクリックされた
        onELRDisabled();
        break;
    case shark::BBPState::BEY_ATTACHED_S2:  // 射出命令
        onLaunchStarted();
        break;
    case shark::BBPState::BEY_DETACHED_S2:  // 射出命令キャンセル
        if (gIsAbortable.load()) {
            onLaunchCanceled();
        }
        break;
//-----------------------------------------------------------------------------
#else
//-----------------------------------------------------------------------------
    // BBPのボタンがダブルクリックされた
    case shark::BBPState::TRANS_S1_TO_S2:
    case shark::BBPState::TRANS_S2_TO_S1:
        onSwitchedToManualMode();
        break;
//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------
    default:
        break;
    }
}

//=============================================================================
//
// AutoMode
//
//=============================================================================

void runAutoMode()
{
    debugMsg(F("[auto/measurement mode] in"));

    ATLAS.state.clear();

#if ATLAS_FORMAT == ATLAS_FULL_SPEC
    // イベントグループの作成
    gEventGroup = xEventGroupCreate();
#endif

    // BLE通信開始
    NimBLEDevice::init("");

    // BBP（ペリフェラル）のスキャン登録
    gDeviceFound.store(false);
    NimBLEScan* scan = NimBLEDevice::getScan();
    scan->setFilterPolicy(BLE_HCI_SCAN_FILT_NO_WL);
    scan->setScanCallbacks(&gScanCallbacks);
    scan->setActiveScan(true);

    while (ATLAS.isAutoMode()) {
        // BBPのアドバタイズを促すメッセージを表示
        ATLAS.view.autoModePromotion();

        debugMsg(F("scanning BeyBattle pass..."));
        scan->start(5000, false);  // 5秒スキャン
        if (!gDeviceFound.load()) {
            delay(100);
            continue;
        }

        NimBLEClient* client = NimBLEDevice::createClient();
        client->setClientCallbacks(&gClientCallbacks, false);
        if (!client->connect(gFoundAddress)) {
            debugMsg(F("connection failed"));
            NimBLEDevice::deleteClient(client);
            continue;
        }

        NimBLERemoteService* serv = client->getService(BBP_SERVICE);
        if (!serv) {
            debugMsg(F("no service"));
            client->disconnect();
            NimBLEDevice::deleteClient(client);
            continue;
        }

        NimBLERemoteCharacteristic* ch = serv->getCharacteristic(BBP_CHR_SP);
        if (!ch) {
            // キャラクタリスティックが存在しない
            debugMsg(F("no characteristic"));
        }
        else if (!ch->canNotify()) {
            // 購読できるキャラクタリスティックではない
            debugMsg(F("not subscribable"));
        }
        else if (!ch->subscribe(true, onNotifyData)) {
            // キャラクタリスティックの購読に失敗した
            debugMsg(F("subscription failed"));
        }
        else {
            ATLAS.player.play(AUDIO_SE_ACK);  // 接続完了のアナウンス音
            ATLAS.state.setBBP(true);         // 状態を更新
            ATLAS.view.autoModeStandby();     // 描画

            // BBPとATLASの通信開始
            while (client->isConnected()) {
                if (ATLAS.isManualMode()) {
                    ATLAS.state.setELR(false);
                    // デバイスからの切断とクライアントの削除
                    if (client->isConnected()) {
                        if (ch && ch->canNotify()) {
                            ch->unsubscribe();
                        }
                    }
                    break;
                }
                delay(1);
            }
        }

        // クライアントの削除
        if (client->isConnected()) {
            client->disconnect();
            while (!gDisconnected.load()) {
                delay(10);
            }
            delay(50);
        }
        NimBLEDevice::deleteClient(client);
        gDeviceFound.store(false);

        // BBPとATLASのセッション終了の処理
        ATLAS.player.play(AUDIO_SE_CANCEL);  // 音声案内
        ATLAS.state.setBBP(false);           // 状態更新
        ATLAS.state.setBey(false);           // 状態更新
        delay(1);
    }

    // 終了処理
    scan->stop();
    scan->clearResults();
    NimBLEDevice::deinit(true);

#if ATLAS_FORMAT == ATLAS_FULL_SPEC
    // イベントグループの削除
    vEventGroupDelete(gEventGroup);
#endif

    debugMsg(F("[auto/measurement mode] out"));
}

//-----------------------------------------------------------------------------
} // namespace atlas

/*
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
*/
#include "mode_process.hh"

// C++標準ライブラリ
#include <atomic>   // std::atomic_ulong
#include <cstring>

// Arduino
#include <Arduino.h>
#include <NimBLEDevice.h>
#include <SPIFFS.h>

// ATLAS
#include "atlas_manager.hh"
#include "device_info.hh"
#include "utils.hh"
#include "setting.hh"

namespace atlas {
//-----------------------------------------------------------------------------

// ペリフェラルサーバー, サービス
static NimBLEServer* gServer = nullptr;
static NimBLEService* gService = nullptr;

// 生データ転送用
static NimBLECharacteristic* gCharDataRaw;              // キャラクタリスティック
static QueueHandle_t gQueueDataTrans = nullptr;         // 送信開始
static QueueHandle_t gQueueDataAck = nullptr;           // ACK受信用
static TaskHandle_t gHandleTaskDataTrans = nullptr;     // データ転送タスクハンドル
static constexpr std::uint16_t WINDOW_SIZE = 10;        // 送信ウィンドウサイズ
static constexpr std::uint16_t PAYLOAD_SIZE = (64 + 2 * 3) * 3;  // 210 bytes
static std::atomic_bool gNotifyEnabled = false;         // 送信可否

// デバイス情報
static constexpr atlas::DeviceInfo DEVICE_INFO {
    .version {
        .rev   = REVISION,
        .minor = MINOR_VERSION,
        .major = MAJOR_VERSION
    },
    .condition {
        .format     = ATLAS_FORMAT,
        .switchType = SWITCH_TYPE,
        .elrStyle   = NUM_MOTORS - 1
    }
};

//-----------------------------------------------------------------------------
#if ATLAS_FORMAT == ATLAS_FULL_SPEC  // 電動ランチャー制御として使う場合
//-----------------------------------------------------------------------------

// イベントグループ
static EventGroupHandle_t gEventGroup;

// タスク制御
#define  BIT_SHOOT  (1 << 0)

//-----------------------------------------------------------------------------
#endif  // #if ATLAS_FORMAT == ATLAS_FULL_SPEC
//-----------------------------------------------------------------------------

//=============================================================================
//
// タスク
//
//=============================================================================

// 生データ転送タスク
void taskDataTrans(void* pvParams)
{
    std::uint8_t cmd;
    std::uint8_t buf[PAYLOAD_SIZE + 2];  // シーケンス番号の分が2bytes

    while (true) {
        // 通知待ち（ブロック）
        if (xQueueReceive(gQueueDataTrans, &cmd, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        // クライアントがsubscribeしているか
        if (!gNotifyEnabled.load()) {
            debugMsg(F("not subscribed"));
            continue;
        }

        // ファイルを開く
        File file = SPIFFS.open(RAW_FPATH, "r");
        if (!file) {
            debugMsg(F("file not found"));
            continue;
        }

        // サイズ計算等の準備
        const std::uint32_t totalSize = file.size();
        const std::uint32_t numPackets =
            (totalSize + PAYLOAD_SIZE - 1) / PAYLOAD_SIZE;

        // シーケンス番号
        std::uint16_t seq = 0;
        std::uint16_t lastAck = 0;

        debugMsg(F("start sending SP raw data..."));

        Serial.printf("File size: %u\n", file.size());
        int counter = 1;

        while (seq < numPackets) {
            // ウィンドウ分送信
            while (seq < (lastAck + WINDOW_SIZE) && seq < numPackets) {
                // ファイルの内容をバッファに読み込む
                std::size_t size = file.read(buf + 2, PAYLOAD_SIZE);

                // SEQ付与
                buf[0] = seq & 0xff;
                buf[1] = seq >> 8;

                // 値をセット
                gCharDataRaw->setValue(buf, size + 2);

                // 送信
                while (!gCharDataRaw->notify()) {
                    taskYIELD();
                }

                seq += 1;
            }

            // ACK待ち
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

            // ACK反映
            std::uint16_t ack;
            while (xQueueReceive(gQueueDataAck, &ack, 0) == pdTRUE) {
                if (ack >= lastAck) {
                    lastAck = ack + 1;  // ← 次に送るべき位置
                }
            }

            // 中断チェック
            if (uxQueueMessagesWaiting(gQueueDataTrans)) {
                debugMsg(F("raw data sending aborted"));
                break;
            }
        }
        file.close();

        debugMsg(F("data sending completed"));
    }
    // タスク終了処理
    vTaskDelete(nullptr);
}

//-----------------------------------------------------------------------------
#if ATLAS_FORMAT == ATLAS_FULL_SPEC  // 電動ランチャー制御として使う場合
//-----------------------------------------------------------------------------

// モーター制御：マニュアルモード
void taskMotorManual(void* pvParams)
{
    // モーターIDの取得
    auto id = *(static_cast<std::uint32_t*>(pvParams));

    // 回転開始
    const auto& elr = ATLAS.params.elr(id);
    ATLAS.motors[id].rotate(elr.sp(), elr.isRight());

    // SHOOT号令待機
    xEventGroupWaitBits(
        gEventGroup,
        BIT_SHOOT,
        pdTRUE,
        pdTRUE,
        portMAX_DELAY
    );

    // 射出
    ATLAS.motors[id].stop();

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
class ServerCallbacks
    : public NimBLEServerCallbacks
{
    // 接続時
    void onConnect(
        NimBLEServer* server,
        NimBLEConnInfo& connInfo
    ) override {
        debugMsg(F("client connected"));

        // ACK音を鳴らす
        ATLAS.player.play(AUDIO_SE_ACK);
        // クライアントが接続された
        ATLAS.state.setClient(true);
        // 画面更新
        ATLAS.view.manualModeStandby();
    }

    // 切断時
    void onDisconnect(
        NimBLEServer* server,
        NimBLEConnInfo& connInfo,
        int reason
    ) override {
        debugMsg(F("client disconnected"));

        // キャンセル音を鳴らす
        ATLAS.player.play(AUDIO_SE_CANCEL);
        // クライアントが切断された
        ATLAS.state.setClient(false);
        // 画面更新
        ATLAS.view.manualModeStandby();
        // 再アドバタイズ
        server->startAdvertising();
    }
};

// デバイス情報
class DevInfoCallbacks
    : public NimBLECharacteristicCallbacks
{
    void onRead(NimBLECharacteristic* ch, NimBLEConnInfo& conn_info) override {
        debugMsg(F("read device info"));
        ch->setValue(DEVICE_INFO);
    }
};

// パラメータの読み書き
class ParamsCallbacks
    : public NimBLECharacteristicCallbacks
{
    void onRead(NimBLECharacteristic* ch, NimBLEConnInfo& connInfo) override {
        debugMsg(F("read parameters"));

        // パラメータの正規化
        ATLAS.params.regulate();
        ch->setValue(ATLAS.params);
    }

    void onWrite(NimBLECharacteristic* ch, NimBLEConnInfo& connInfo) override {
        debugMsg(F("write parameters"));

        // コピー
        ATLAS.params = ch->getValue<Params>();

        // パラメータの正規化
        ATLAS.params.regulate();

        // 画面更新
        ATLAS.view.manualModeStandby();

        // ACK音
        ATLAS.player.play(AUDIO_SE_ACK);

        // パラメータをファイルに保存する
        if (File file = SPIFFS.open(PARAMS_FPATH, "w")) {
            writeFile(file, ATLAS.params);
            file.close();
        }
    }
};

// 結果関連
class ResultCallbacks
    : public NimBLECharacteristicCallbacks
{
    // 結果の読み出し
    void onRead(NimBLECharacteristic* ch, NimBLEConnInfo& connInfo) override {
        debugMsg(F("read result"));
        ch->setValue(
            reinterpret_cast<const std::uint8_t*>(&ATLAS.result),
            sizeof(Result)
        );
    }

    // 結果の初期化
    void onWrite(NimBLECharacteristic* ch, NimBLEConnInfo& connInfo) override {
        debugMsg(F("clear result"));

        // 解析結果を初期化する
        ATLAS.result.clear();
        // 画面更新
        ATLAS.view.manualModeStandby();
        // ACK音を鳴らす
        ATLAS.player.play(AUDIO_SE_ACK);

        // SP統計データ（空）をファイルに保存する
        if (File file = SPIFFS.open(RESULT_FPATH, "w")) {
            writeFile(file, ATLAS.result);
            file.close();
        }

        // 生データファイルを削除する
        if (SPIFFS.exists(RAW_FPATH)) {
            SPIFFS.remove(RAW_FPATH);
        }
    }
};

// 生データの読み出し制御
class RawCtrlCallbacks
    : public NimBLECharacteristicCallbacks
{
    void onRead(NimBLECharacteristic* ch, NimBLEConnInfo& connInfo) override {
        debugMsg(F("read size of raw data"));

        // ファイルサイズ取得
        std::uint32_t size = 0;
        if (File file = SPIFFS.open(RAW_FPATH, "r")) {
            size = static_cast<std::uint32_t>(file.size());
            file.close();
        }
        ch->setValue(size);
    }

    void onWrite(NimBLECharacteristic* ch, NimBLEConnInfo& connInfo) override {
        auto cmd = ch->getValue();

        // cmdが2バイトならACK処理
        if (cmd.length() == 2) {
            std::uint16_t ack =
                static_cast<std::uint16_t>(cmd[0]) |
                static_cast<std::uint16_t>(cmd[1]) << 8;

            xQueueSend(gQueueDataAck, &ack, 0);

            if (gHandleTaskDataTrans) {
                xTaskNotifyGive(gHandleTaskDataTrans);
            }
            return;
        }

        debugMsg(F("start notify raw data"));
        std::uint8_t ctrl = static_cast<std::uint16_t>(cmd[0]);
        xQueueSend(gQueueDataTrans, &ctrl, 0);
    }
};

// 生データの読み出し購読
class RawDataCallbacks
    : public NimBLECharacteristicCallbacks
{
    void onSubscribe(
        NimBLECharacteristic* pCharacteristic,
        NimBLEConnInfo& connInfo,
        std::uint16_t subValue
    ) override {
        if (subValue & 0x0001) {
            debugMsg(F("notifications enabled"));
            gNotifyEnabled.store(true);
        }
        else {
            debugMsg(F("notifications disabled"));
            gNotifyEnabled.store(false);
        }
    }
};

//-----------------------------------------------------------------------------
#if ATLAS_FORMAT == ATLAS_FULL_SPEC  // 電動ランチャー制御として使う
//-----------------------------------------------------------------------------

// マニュアルシュート指令
class ManualShootCallbacks
    : public NimBLECharacteristicCallbacks
{
    void onWrite(NimBLECharacteristic* ch, NimBLEConnInfo& connInfo) override {
        debugMsg(F("launch beyblade"));

        // モータータスクの投入
        char taskname[14];
        for (std::uint32_t i = 0; i < NUM_MOTORS; ++i) {
            if (!ATLAS.params.elr(i).enabledManual()) continue;

            // タスク名
            std::sprintf(taskname, "taskMotor_m%lu", i+1);

            // タスクの投入
            xTaskCreate(
                taskMotorManual, // タスク
                taskname,        // タスク名
                4096,            // スタックメモリ
                &i,              // 起動パラメータ（モーターID）
                3,               // 優先度（値が大きいほど優先順位が高い）
                nullptr          // タスクハンドル
            );
        }

        // 待機
        vTaskDelay(pdMS_TO_TICKS(MOTOR_PREPARATORY_TIME));

        // Shoot!のタイミングを全体通知
        xEventGroupSetBits(gEventGroup, BIT_SHOOT);
    }
};

//-----------------------------------------------------------------------------
#endif  // #if ATLAS_FORMAT == ATLAS_FULL_SPEC
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
#if SWITCH_TYPE != SW_SLIDE  // スライドスイッチ以外
//-----------------------------------------------------------------------------

// モードをオートモードに切り替える
class ModeSwitchCallbacks
    : public NimBLECharacteristicCallbacks
{
    void onWrite(NimBLECharacteristic* ch, NimBLEConnInfo& connInfo) override {
        debugMsg(F("switch to auto mode"));
        ATLAS.setMode(true);
    }
};

//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------

//=============================================================================
//
// ManualMode
//
//=============================================================================

void runManualMode()
{
    debugMsg(F("[manual/setting mode] in"));

    // 状態のクリア
    ATLAS.state.clear();

#if ATLAS_FORMAT == ATLAS_FULL_SPEC
    // イベントグループの作成
    gEventGroup = xEventGroupCreate();
#endif

    // BLE初期化
    NimBLEDevice::init(ATLAS_LOCAL_NAME);
    NimBLEDevice::setMTU(ATLAS_MTU_SIZE);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);

    // サーバーとサービスの作成
    gServer = NimBLEDevice::createServer();
    gService = gServer->createService(ATLAS_SERVICE);

    //-------------------------------------------------------------------------
    // コールバックの登録
    //-------------------------------------------------------------------------
    // サーバー
    gServer->setCallbacks(new ServerCallbacks);

    // デバイス情報
    NimBLECharacteristic* charDevInfo = gService->createCharacteristic(
        ATLAS_CHR_DEVINFO,
        NIMBLE_PROPERTY::READ
    );
    charDevInfo->setCallbacks(new DevInfoCallbacks);
    charDevInfo->setValue(DEVICE_INFO);

    // パラメータ読み書き
    NimBLECharacteristic* charParams = gService->createCharacteristic(
        ATLAS_CHR_PARAMS,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
    );
    charParams->setCallbacks(new ParamsCallbacks);
    charParams->setValue(ATLAS.params);

    // 解析結果
    NimBLECharacteristic* charResult = gService->createCharacteristic(
        ATLAS_CHR_RESULT,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
    );
    charResult->setCallbacks(new ResultCallbacks);
    charResult->setValue(ATLAS.result);

    // 生データ制御
    NimBLECharacteristic* charStatsCtrl = gService->createCharacteristic(
        ATLAS_CHR_RAW_CTRL,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
    );
    charStatsCtrl->setCallbacks(new RawCtrlCallbacks);
    charStatsCtrl->setValue(0);

    // 生データ取得
    gCharDataRaw = gService->createCharacteristic(
        ATLAS_CHR_RAW_DATA,
        NIMBLE_PROPERTY::NOTIFY
    );
    gCharDataRaw->setCallbacks(new RawDataCallbacks);

#if ATLAS_FORMAT == ATLAS_FULL_SPEC
    // 手動射出
    NimBLECharacteristic* charManualShoot = gService->createCharacteristic(
        ATLAS_CHR_SHOOT,
        NIMBLE_PROPERTY::WRITE
    );
    charManualShoot->setCallbacks(new ManualShootCallbacks);
#endif

#if SWITCH_TYPE != SW_SLIDE  // スライドスイッチ以外
    // モードスイッチ
    NimBLECharacteristic* charModeSwitch = gService->createCharacteristic(
        ATLAS_CHR_SWITCH,
        NIMBLE_PROPERTY::WRITE
    );
    charModeSwitch->setCallbacks(new ModeSwitchCallbacks);
#endif

    //-------------------------------------------------------------------------
    // サービスとアドバタイズの開始
    //-------------------------------------------------------------------------

    // サービス開始
    gService->start();

    // アドバタイズ開始
    NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();
    advertising->addServiceUUID(ATLAS_SERVICE);
    advertising->enableScanResponse(true);
    advertising->setName(ATLAS_LOCAL_NAME);
    advertising->start();

    // データ転送タスク起動
    gQueueDataAck = xQueueCreate(10, sizeof(std::uint16_t));
    gQueueDataTrans = xQueueCreate(4, sizeof(std::uint8_t));
    xTaskCreate(
        taskDataTrans,
        "taskDataTrans",
        4096,
        nullptr,
        1,
        &gHandleTaskDataTrans
    );

    debugMsg(F("[manual/setting mode] BLE advertising started"));

    //-------------------------------------------------------------------------
    // 待機
    //-------------------------------------------------------------------------

    // 表示
    ATLAS.state.setClient(false);
    ATLAS.view.manualModeStandby();

    // マニュアルモード時
    while (ATLAS.isManualMode()) {
        delay(10);
    }

    //-------------------------------------------------------------------------
    // 終了処理
    //-------------------------------------------------------------------------

    // 終了処理
    NimBLEDevice::deinit(true);

#if ATLAS_FORMAT == ATLAS_FULL_SPEC
    // イベントグループの削除
    vEventGroupDelete(gEventGroup);
#endif

    debugMsg(F("[manual/setting mode] out"));
}

//-----------------------------------------------------------------------------
} // namespace atlas

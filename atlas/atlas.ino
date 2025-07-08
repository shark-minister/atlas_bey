/*
    ATLAS - AuTomatic LAuncher System
    Copyright (C) 2025  Shark Minister
    All right reserved.
*/
// C++標準ライブラリ
#include <atomic>             // std::atomic_bool
#include <memory>             // std::unique_ptr

// Arduino
#include <Arduino.h>
#include <ArduinoBLE.h>       // BLE通信
#include <SPIFFS.h>           // フラッシュメモリをファイル保存に使う

// ATLAS
#include "setting.hh"         // 設定
#include "params.hh"          // パラメータ
#include "analyzer.hh"        // BBPデータ解析
#include "statistics.hh"      // 統計データ
#include "view.hh"            // UI

// ディスプレイドライバ
#if DISPLAY_DRIVER == ADAFRUIT_SH1106G
#include <Adafruit_SH110X.h>
typedef  Adafruit_SH1106G  DisplayDriver;
#elif DISPLAY_DRIVER == ADAFRUIT_SSD1306
#include <Adafruit_SSD1306.h>
typedef  Adafruit_SSD1306  DisplayDriver;
#endif

//-----------------------------------------------------------------------------
// グローバル変数
//-----------------------------------------------------------------------------
// タスクハンドラ
TaskHandle_t g_hdl_task_ble = nullptr;  // BLE通信タスクハンドラ

// 状態管理
atlas::Params g_params;                     // 制御パラメータ
atlas::State g_state;                       // 接続状態
atlas::Statistics g_data;
std::uint32_t g_data_index = 0;
std::atomic_bool g_is_auto_mode = true;     // オートモードか否かのフラグ

// ディスプレイ制御
#if (DISPLAY_DRIVER & DISPLAY_IS_SPI) == 0
DisplayDriver g_display(SCREEN_WIDTH, SCREEN_HEIGHT);
#else
DisplayDriver g_display(SCREEN_WIDTH, SCREEN_HEIGHT,
                        SCREEN_SPI_MOSI, SCREEN_SPI_CLK, SCREEN_SPI_DC,
                        SCREEN_SPI_RESET, SCREEN_SPI_CS);
#endif
atlas::View<DisplayDriver> g_view(g_display, g_data, g_params, g_state);


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#if SP_MEAS_ONLY == 0  // 電動ランチャー制御として使う場合
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#include <DFRobotDFPlayerMini.h>  // オーディオ制御
#include "motor.hh"               // モーター制御

// 射出指令時刻
typedef unsigned long ulong;
std::atomic_ulong g_t0 = 0;

// モーター制御
atlas::Motor g_motors[NUM_MOTORS];   // モーター制御インスタンス
void rotate_motor(atlas::Motor& motor,
                  const atlas::ElectricLauncher& elr)
{
    motor.rotate(elr.sp(), elr.is_right());
}

// 音声制御
// 使わないときのためにポインタにしておく
std::unique_ptr<DFRobotDFPlayerMini> g_dfplayer;
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#endif  // #if SP_MEAS_ONLY == 0
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

void play_audio(int number)
{
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#if SP_MEAS_ONLY == 0  // 電動ランチャー制御として使う場合
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    if (g_dfplayer)
    {
        g_dfplayer->playFolder(number, 1);
    }
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#endif  // #if SP_MEAS_ONLY == 0
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
}

//-----------------------------------------------------------------------------
// セットアップ関数
//-----------------------------------------------------------------------------
void setup()
{
    // シリアル通信開始
    Serial.begin(9600);
    while (!Serial);

#if (DISPLAY_DRIVER & ADAFRUIT_SH1106G) > 0
    // ディスプレイ開始（SH1106）
    if (!g_display.begin(SCREEN_ADDR))
#elif DISPLAY_DRIVER == ADAFRUIT_SSD1306
    // ディスプレイ開始（SSD1306）
    if (!g_display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDR))
#elif DISPLAY_DRIVER == (SPI_MASK | ADAFRUIT_SSD1306)
    // ディスプレイ開始（SSD1309@SPI）
    if (!g_display.begin())
#endif
    {
        Serial.println(F("Display allocation failed!"));
        while (true);
    }

    // さめ大臣ロゴ
    g_view.splash_screen();

    // SPIFFS開始
    if (!SPIFFS.begin(true))
    {
        Serial.println(F("SPIFFS Mount Failed"));
        while (true);
    }

    // 統計データの読み込み
    g_data.clear();
    if (SPIFFS.exists(STATISTICS_FILE_NAME))
    {
        if (File file = SPIFFS.open(STATISTICS_FILE_NAME, "r"))
        {
            if (file.size() == sizeof(g_data))
            {
                file.read(reinterpret_cast<std::uint8_t*>(&g_data), sizeof(g_data));
                file.close();
            }
            else
            {
                Serial.println("File size did not match");
            }
        }
    }

    // パラメータの読み込み
    if (SPIFFS.exists(PARAMS_FILE_NAME))
    {
        if (File file = SPIFFS.open(PARAMS_FILE_NAME, "r"))
        {
            file.read(reinterpret_cast<std::uint8_t*>(&g_params), sizeof(g_params));
            file.close();
        }
    }
    g_params.regulate();

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#if SP_MEAS_ONLY == 0  // 電動ランチャー制御として使う場合
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    // 音声制御用通信
    Serial1.begin(9600, SERIAL_8N1, 20, 21);
    if (!Serial1)
    {
        Serial.println(F("Unable to begin Serial1"));
    }
    // 音声制御のインスタンス実体化
    g_dfplayer.reset(new DFRobotDFPlayerMini);
    if (g_dfplayer->begin(Serial1))
    {
        g_dfplayer->volume(20);  // ボリューム
    }
    else
    {
        g_dfplayer.reset();
        Serial.println(F("Unable to begin DFPlayer"));
    }

    // モーター制御インスタンスの設定
    g_motors[0].configure(L_PWM_1, R_PWM_1, LR_EN_1, MOTOR1_MAX_RPM);
    if constexpr (NUM_MOTORS == 2)
    {
        g_motors[1].configure(L_PWM_1, R_PWM_1, LR_EN_1, MOTOR2_MAX_RPM);
    };
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#endif
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#if SWITCH_LESS == 0 // スイッチを使う場合
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    // スライドスイッチのスライド中はINPUTピンはどこにも繋がれないので
    // 念のため内蔵のプルアップ抵抗を使う
    pinMode(MODE_SW, INPUT_PULLUP);  
    g_is_auto_mode.store(digitalRead(MODE_SW) == 1);
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#endif  // #if SWITCH_LESS == 0
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

    // 2秒間待機
    delay(2000);

    // BLE接続タスクの生成・投入
    xTaskCreateUniversal(
        task_ble,            // タスク
        "task_ble",          // タスク名
        8192,                // スタックメモリ
        nullptr,             // 起動パラメータ
        1,                   // 優先度（値が大きいほど優先順位が高い）
        &g_hdl_task_ble,     // タスクハンドル
        PRO_CPU_NUM          // コアID
    );
}

//-----------------------------------------------------------------------------
// ループ関数
//-----------------------------------------------------------------------------
void loop()
{
    // モード切替スイッチの状態をチェックして、モード切替
    g_is_auto_mode.store(digitalRead(MODE_SW) == 1);
    delay(100);
}

//-----------------------------------------------------------------------------
// ファイルの書き込み
//-----------------------------------------------------------------------------
void write_data()
{
    if (File file = SPIFFS.open(STATISTICS_FILE_NAME, "w"))
    {
        file.write(
            reinterpret_cast<const std::uint8_t*>(&g_data),
            sizeof(g_data)
        );
        file.close();
    }
}

//-----------------------------------------------------------------------------
// BLE通信を担うタスク
//-----------------------------------------------------------------------------
void task_ble(void* pv_params)
{
    while (true)
    {
        g_is_auto_mode.load() ? run_auto_mode() : run_manual_mode();
        
        // おまじない
        delay(1);
    }
}

//-----------------------------------------------------------------------------
// オートモード
//-----------------------------------------------------------------------------
void run_auto_mode()
{
    Serial.println(F("[auto mode] in"));

    // 初期化
    g_state.clear();

    // BLE通信開始
    //BLELocalDevice ble;
    if (!BLE.begin())
    {
        BLE.end();
        return;
    }

    // BBP（ペリフェラル）のスキャン開始
    BLE.scanForName(BBP_LOCAL_NAME);

    // オートモード時
    bool mode_change = false;
    while (g_is_auto_mode.load())
    {
        // BBPのアドバタイズを促すメッセージを表示
        g_view.auto_mode_promotion();

        // ベイバトルパスが見つかり、そのデバイスが有効なら
        if (BLEDevice dev = BLE.available())
        {
            // スキャンを一旦停止
            BLE.stopScan();

            // 接続開始
            if (!dev.connect())
            {
                Serial.println(F("connection failed"));
            }
            // サービス検索
            else if (!dev.discoverService(BBP_SERVICE))
            {
                Serial.println(F("no service"));
            }
            else
            {
                // キャラクタリスティックの購読
                BLECharacteristic chr = dev.characteristic(BBP_CHR_SP);
                if (!chr)
                {
                    // キャラクタリスティックが存在しない
                    Serial.println(F("no characteristic"));
                }
                // 購読できるかどうか
                else if (!chr.canSubscribe())
                {
                    // キャラクタリスティックが購読できない（Notify属性がない）
                    Serial.println(F("not subscribable"));
                }
                // 購読
                else if (!chr.subscribe())
                {
                    // キャラクタリスティックの購読に失敗した
                    Serial.println(F("subscription failed"));
                }
                // セッション開始
                else
                {
                    // ベイバトルパス接続完了のアナウンス音
                    play_audio(AUDIO_SE_ACK);

                    // キャラクタリスティックの購読に成功したので、状態を更新
                    g_state.set_bbp(true);

                    // ヘッダのみ描画
                    g_view.auto_mode_plain();

                    if (!bbp_session(dev, chr))
                    {
                        // モード切替あり
                        mode_change = true;
                        g_state.set_elr(false);
                    }
                }
            }
            
            // 音声案内
            play_audio(AUDIO_SE_CANCEL);

            // ベイバトルパスからの切断
            g_state.set_bbp(false);
            g_state.set_bey(false);
            g_view.auto_mode_plain();

            // デバイスからの切断
            dev.disconnect();

            // オートモード終了
            if (mode_change)
            {
                break;
            }

            // スキャン開始
            BLE.scanForName(BBP_LOCAL_NAME);
        }
        // おまじない
        delay(1);
    }

    // リソースの開放
    BLE.end();

    Serial.println(F("[auto mode] out"));
}

// BBPとのBLE通信セッション
// 戻り値が "false" のときモード切替あり
bool bbp_session(BLEDevice& dev, BLECharacteristic& chr)
{
    // BBPからのデータ解析準備
    atlas::BBPAnalyzer analyzer;                // 解析
    atlas::BBPData buf;                         // 読み出し用バッファ

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#if SP_MEAS_ONLY == 0  // 電動ランチャー制御として使う場合
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    // モーター制御・カウントダウンタスク関連
    TaskHandle_t hdl_task_motor = nullptr;        // モーター制御タスクハンドラ
    TaskHandle_t hdl_task_count_view = nullptr;   // カウントダウン表示タスクハンドラ
    TaskHandle_t hdl_task_count_voice = nullptr;  // カウントダウン音声タスクハンドラ
    int abort_sig = 1;                            // 中止信号（値は適当）
    ulong t_cancel_limit = 0;                     // 中止可能期限

    // 射出キャンセル用のキュー
    QueueHandle_t que1 = xQueueCreate(1, sizeof(ulong));  // モーター駆動を中止
    QueueHandle_t que2 = xQueueCreate(1, sizeof(ulong));  // カウントダウン表示を中止
    QueueHandle_t que3 = xQueueCreate(1, sizeof(ulong));  // カウントダウン音声を中止
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#endif
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

    // 購読開始
    while (dev.connected())
    {
        // 値が更新されたら
        if (chr.valueUpdated())
        {
            // BBPからのデータ読み出し
            chr.readValue(buf.data(), atlas::BBPData::LENGTH);

            // 値の解析
            auto state = analyzer.analyze(buf);
            g_state.set_bey(atlas::is_bey_attached(state));
            switch (state)
            {
            // ベイブレードがランチャーにセットされた、外れた
            case atlas::BBPState::BEY_ATTACHED:
            case atlas::BBPState::BEY_DETACHED:
                g_view.auto_mode_plain();
                break;

            // ベイブレードがシュートされた
            case atlas::BBPState::FINISHED:
                // プロファイル解析
                analyzer.calc_true_sp();
                // 表示更新
                g_view.auto_mode_sp(analyzer.bbp_sp(),
                                    analyzer.true_sp(),
                                    analyzer.max_sp());
                // データ更新
                g_data.update(analyzer.true_sp());
                // 保存
                write_data();
                // データクリア
                analyzer.clear();
                break;

            // エラー
            case atlas::BBPState::ERROR:
                g_view.auto_mode_error();
                break;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#if SP_MEAS_ONLY > 0  // SP計測器としてのみ使う
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#if SWITCH_LESS > 0   // スイッチレス実装
            // BBPのボタンがダブルクリックされた
            case atlas::BBPState::ELR_ENABLED:
            case atlas::BBPState::ELR_DISABLED:
                // モードをマニュアル/設定モードに切り替える
                g_is_auto_mode.store(false);
                break;
#endif
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#else  // 電動ランチャー制御として使う
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
            // BBPのボタンがダブルクリックされた
            case atlas::BBPState::ELR_ENABLED:
                play_audio(AUDIO_SE_ACK);    // ACK音
                g_state.set_elr(true);
                g_view.auto_mode_plain();
                break;

            // BBPのボタンがダブルクリックされた
            case atlas::BBPState::ELR_DISABLED:
                play_audio(AUDIO_SE_CANCEL);    // キャンセル音
                g_state.set_elr(false);
                g_view.auto_mode_plain();
                break;

            // 射出命令
            case atlas::BBPState::SHOOT_ORDERED:
                // キューを空にしておく
                xQueueReceive(que1, &abort_sig, 0);
                xQueueReceive(que2, &abort_sig, 0);
                // 現在時刻とキャンセル可能期限時刻の記録
                g_t0.store(millis());
                t_cancel_limit = g_t0.load() + g_params.latency();
                // モータータスクの投入
                xTaskCreateUniversal(
                    task_motor_auto,
                    "task_motor_auto",     // タスク名
                    4096,                  // スタックメモリ
                    que1,                  // 起動パラメータ
                    3,                     // 優先度（値が大きいほど優先順位が高い）
                    &hdl_task_motor,       // タスクハンドル
                    PRO_CPU_NUM            // コアID
                );
                // カウントダウンタスクの投入
                xTaskCreateUniversal(
                    task_count_view,       // タスク
                    "task_count_view",     // タスク名
                    4096,                  // スタックメモリ
                    que2,                  // 起動パラメータ
                    2,                     // 優先度（値が大きいほど優先順位が高い）
                    &hdl_task_count_view,  // タスクハンドル
                    PRO_CPU_NUM            // コアID
                );
                // カウントダウンタスクの投入
                xTaskCreateUniversal(
                    task_count_voice,      // タスク
                    "task_count_voice",    // タスク名
                    4096,                  // スタックメモリ
                    que3,                  // 起動パラメータ
                    2,                     // 優先度（値が大きいほど優先順位が高い）
                    &hdl_task_count_voice, // タスクハンドル
                    PRO_CPU_NUM            // コアID
                );
                break;

            // 電動ランチャー連動のシュートか、射出命令キャンセル
            case atlas::BBPState::BEY_DETACHED_2:
                // キャンセル可能期間内だったら
                if (millis() < t_cancel_limit)
                {
                    // 各タスクに中止信号を送る
                    xQueueSend(que1, &abort_sig, 0);
                    xQueueSend(que2, &abort_sig, 0);
                }
                break;
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#endif
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

            default:
                break;
            }

            // バッファの初期化
            buf.init();
        }

        // モードが変わった
        if (!g_is_auto_mode.load())
        {
            return false;
        }

        // おまじない
        delay(1);
    }

    return true;
}

//-----------------------------------------------------------------------------
// マニュアルモード
//-----------------------------------------------------------------------------
void run_manual_mode()
{
    Serial.println(F("[manual mode] in"));

    // 初期化
    g_state.clear();

    // BLE通信開始
    //BLELocalDevice ble;
    if (!BLE.begin())
    {
        BLE.end();
        return;
    }

    // 接続されたときのハンドラ
    BLE.setEventHandler(
        BLEConnected,
        [](BLEDevice central)
        {
            g_state.set_client(true);
            g_view.manual_mode();
        }
    );

    // 接続が切れたときのハンドラ
    BLE.setEventHandler(
        BLEDisconnected,
        [](BLEDevice central)
        {
            g_state.set_client(false);
            g_view.manual_mode();
        }
    );

    // サービスの作成
    BLEService serv(ATLAS_SERVICE);

    // characteristic: パラメータ設定
    {
        // キャラクタリスティックの作成と初期値の書き込み
        BLETypedCharacteristic<atlas::Params> ch(ATLAS_CHR_SET, BLERead | BLEWrite);
        ch.writeValue(g_params);
        // キャラクタリスティックの説明文
        BLEDescriptor descr("2901", ATLAS_CHR_SET_DESCR);
        ch.addDescriptor(descr);
        // イベントハンドラの登録
        ch.setEventHandler(
            BLEWritten,
            [](BLEDevice central, BLECharacteristic chr)
            {
                play_audio(AUDIO_SE_ACK);

                chr.readValue(&g_params, sizeof(g_params));
                g_params.regulate();
                g_view.manual_mode();
            }
        );
        // キャラクタリスティックの登録
        serv.addCharacteristic(ch);
    }
    
    // characteristic: 現在の設定を保存する
    {
        // キャラクタリスティックの作成と初期値の書き込み
        BLETypedCharacteristic<std::uint8_t> ch(ATLAS_CHR_SAVE, BLEWrite);
        // キャラクタリスティックの説明文
        BLEDescriptor descr("2901", ATLAS_CHR_SAVE_DESCR);
        ch.addDescriptor(descr);
        // イベントハンドラの登録
        ch.setEventHandler(
            BLEWritten,
            [](BLEDevice central, BLECharacteristic chr)
            {
                if (File file = SPIFFS.open(PARAMS_FILE_NAME, "w"))
                {
                    file.write(
                        reinterpret_cast<const std::uint8_t*>(&g_params),
                        sizeof(g_params)
                    );
                    file.close();
                }
            }
        );
        // キャラクタリスティックの登録
        serv.addCharacteristic(ch);
    }

    // characteristic: 手動シュート
    {
        // キャラクタリスティックの作成と初期値の書き込み
        BLETypedCharacteristic<std::uint8_t> ch(ATLAS_CHR_SHOOT, BLEWrite);
        // キャラクタリスティックの説明文
        BLEDescriptor descr("2901", ATLAS_CHR_SHOOT_DESCR);
        ch.addDescriptor(descr);
        // イベントハンドラの登録
        ch.setEventHandler(
            BLEWritten,
            [](BLEDevice central, BLECharacteristic chr)
            {
                // SP測定器としてのみ使うなら、空の実装としておく
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#if SP_MEAS_ONLY == 0  // 電動ランチャー制御として使う場合
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
                // モータータスクの投入
                g_t0.store(millis());
                for (std::uint32_t i = 0; i < NUM_MOTORS; ++i)
                {
                    if (g_params.elr(i).enabled_manual())
                    {
                        xTaskCreateUniversal(
                            task_motor_manual,
                            "task_motor",          // タスク名
                            4096,                  // スタックメモリ
                            &i,                    // 起動パラメータ
                            3,                     // 優先度（値が大きいほど優先順位が高い）
                            nullptr,               // タスクハンドル
                            PRO_CPU_NUM            // コアID
                        );
                    }
                }
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#endif
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
            }
        );
        // キャラクタリスティックの登録
        serv.addCharacteristic(ch);
    }

    // characteristic: データ（ヘッダー）
    {
        // キャラクタリスティックの作成と初期値の書き込み
        BLETypedCharacteristic<atlas::Statistics::Header> ch(ATLAS_CHR_HEADER, BLERead);
        // キャラクタリスティックの説明文
        BLEDescriptor descr("2901", ATLAS_CHR_HEADER_DESCR);
        ch.addDescriptor(descr);
        // イベントハンドラの登録: 読み出し
        ch.setEventHandler(
            BLERead,
            [](BLEDevice central, BLECharacteristic chr)
            {
                chr.writeValue(g_data.header(),
                               sizeof(atlas::Statistics::Header));
                // データインデックスの初期化
                g_data_index = 0;
            }
        );
        // キャラクタリスティックの登録
        serv.addCharacteristic(ch);
    }

    // characteristic: データ（ヒストグラム）
    {
        // キャラクタリスティックの作成と初期値の書き込み
        BLETypedCharacteristic<atlas::Statistics::Histogram> ch(ATLAS_CHR_DATA, BLERead);
        // キャラクタリスティックの説明文
        BLEDescriptor descr("2901", ATLAS_CHR_DATA_DESCR);
        ch.addDescriptor(descr);
        // イベントハンドラの登録: 読み出し
        ch.setEventHandler(
            BLERead,
            [](BLEDevice central, BLECharacteristic chr)
            {
                if (g_data_index >= 3)
                {
                    g_data_index = 0;
                }
                chr.writeValue(g_data.hist(g_data_index++),
                               sizeof(atlas::Statistics::Histogram));
            }
        );
        // キャラクタリスティックの登録
        serv.addCharacteristic(ch);
    }

    // characteristic: データクリア
    {
        // キャラクタリスティックの作成と初期値の書き込み
        BLETypedCharacteristic<std::uint8_t> ch(ATLAS_CHR_CLEAR, BLEWrite);
        // キャラクタリスティックの説明文
        BLEDescriptor descr("2901", ATLAS_CHR_CLEAR_DESCR);
        ch.addDescriptor(descr);
        // イベントハンドラの登録
        ch.setEventHandler(
            BLEWritten,
            [](BLEDevice central, BLECharacteristic chr)
            {
                g_data.clear();
                write_data();
                g_view.manual_mode();
            }
        );
        // キャラクタリスティックの登録
        serv.addCharacteristic(ch);
    }

    // ローカル名の設定
    BLE.setLocalName(ATLAS_LOCAL_NAME);

    // アドバタイズの設定と開始
    BLE.setAdvertisedService(serv);
    BLE.addService(serv);
    BLE.advertise();

    // 表示
    g_state.set_client(false);
    g_view.manual_mode();

    // マニュアルモード時
    while (!g_is_auto_mode.load())
    {
        // 1秒間隔でタイムアウト
        BLE.poll(1000);
        // おまじない
        delay(1);
    }

    // リソースの開放
    BLE.end();

    // リセット
    ESP.restart();

    Serial.println(F("[manual mode] out"));
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#if SP_MEAS_ONLY == 0  // 電動ランチャー制御として使う場合
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

// モーター制御: マニュアルモード
void task_motor_manual(void* pv_params)
{
    std::uint32_t id = *(static_cast<std::uint32_t*>(pv_params));

    // 射出時刻（モーターの加速準備時間が必要）
    ulong t_launch = g_t0.load() + MOTOR_PREPARATORY_TIME;

    // 回転開始
    rotate_motor(g_motors[id], g_params.elr(id));

    // 待機実行
    while (millis() < t_launch)
    {
        delay(5);   // 5msは適当
    }

    // モーターの急停止による射出実行
    g_motors[id].stop();

    // タスク終了処理
    vTaskDelete(nullptr);
}

// モーター制御: オートモード
void task_motor_auto(void* pv_params)
{
    QueueHandle_t que = static_cast<QueueHandle_t>(pv_params);

    // latencyだけ、中止信号を待つ
    // 中止信号はキュー経由で来る
    int sig = 0;
    if (xQueueReceive(que, &sig, g_params.latency() / portTICK_RATE_MS) == pdFALSE)
    {
        // キャッシュ
        auto& motor = g_motors[g_params.automode_elr_index()];

        // 射出時刻の計算を行う。
        // 1行目: 射出指令時のシステム時間（ミリ秒）の取り出し。
        // 2行目: カウントダウン号令 3-2-1-Go-Shoot の間の合計時間。
        // 3行目: 射出遅延時間
        // 4行目: 猶予時間（ReadySet--3の間の時間）
        ulong t_launch = g_t0.load()
                       + COUNTDOWN_INTERVAL * 4
                       + g_params.delay()
                       + g_params.latency();

        // 回転開始
        rotate_motor(motor, g_params.automode_elr());

        // 待機実行
        while (millis() < t_launch)
        {
            delay(5);   // 5msは適当
        }

        // モーターの急停止による射出実行
        motor.stop();
    }

    // タスク終了処理
    vTaskDelete(nullptr);
}

// 射出前カウントダウン表示
void task_count_view(void* pv_params)
{
    // Ready Set
    g_view.auto_mode_countdown(0);

    // キュー
    QueueHandle_t que = static_cast<QueueHandle_t>(pv_params);

    // latencyだけ、中止信号を待つ
    // 中止信号はキュー経由で来る
    int sig = 0;
    if (xQueueReceive(que, &sig, g_params.latency() / portTICK_RATE_MS) == pdFALSE)
    {
        ulong t_expire = g_t0.load() + g_params.latency();

        // カウントダウン 3, 2, 1, Go
        for (int i = 1; i < 5; ++i)
        {
            // カウントダウンメッセージ
            g_view.auto_mode_countdown(i);

            // 待機実行
            t_expire += COUNTDOWN_INTERVAL;
            while (millis() < t_expire)
            {
                delay(5);   // 5msは適当
            }
        }
        // SHOOT!
        g_view.auto_mode_countdown(5);
    }
    else
    {
        // 中止
        g_view.auto_mode_aborted();
    }

    // タスク終了処理
    vTaskDelete(nullptr);
}

// 射出前カウントダウン音声
void task_count_voice(void* pv_params)
{
    // キュー
    QueueHandle_t que = static_cast<QueueHandle_t>(pv_params);

    // latencyだけ、中止信号を待つ
    // 中止信号はキュー経由で来る
    int sig = 0;
    if (xQueueReceive(que, &sig, g_params.latency() / portTICK_RATE_MS) == pdFALSE)
    {
        play_audio(AUDIO_COUNTDOWN);
    }
    else
    {
        // 中止
        play_audio(AUDIO_SE_CANCEL);
    }

    // タスク終了処理
    vTaskDelete(nullptr);
}

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#endif  // #if SP_MEAS_ONLY == 0
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/*
    ATLAS - AuTomatic LAuncher System
    Copyright (C) 2025  Shark Minister
    All right reserved.
*/
// C++標準ライブラリ
#include <atomic>         // std::atomic_bool
#include <cstdio>         // std::sprintf

// Arduino
#include <Arduino.h>
#include <ArduinoBLE.h>   // BLE通信

// ATLAS
#include "setting.hh"     // 設定
#include "params.hh"      // パラメータ
#include "analyzer.hh"    // BBPデータ解析
#include "stats.hh"       // 統計データ
#include "view.hh"        // 画面表示
#include "audio.hh"       // オーディオ制御
#include "info.hh"        // デバイス設定情報
#include "filesys.hh"     // ファイルシステム
#include "mode.hh"        // モード管理
#include "user.hh"        // ユーザーアイキャッチ
#include "utils.hh"       // ユーティリティ

//-----------------------------------------------------------------------------
#if ATLAS_FORMAT == ATLAS_FULL_SPEC  // 電動ランチャー制御として使う場合
//-----------------------------------------------------------------------------
#include "motor.hh"  // モーター制御

// 射出指令時刻
std::atomic_ulong g_t0 = 0;

// モーター制御
atlas::Motor g_motors[NUM_MOTORS];   // モーター制御インスタンス
inline void rotate_motor(atlas::Motor& motor,
                         const atlas::ElectricLauncher& elr)
{
    motor.rotate(elr.sp(), elr.is_right());
}
//-----------------------------------------------------------------------------
#endif  // #if ATLAS_FORMAT == ATLAS_FULL_SPEC
//-----------------------------------------------------------------------------

//=============================================================================
// グローバル変数
//=============================================================================
// 状態管理
const atlas::DeviceInfo g_devinfo = atlas::get_device_info();  // デバイス情報
atlas::Params g_params;           // 制御パラメータ
atlas::State g_state;             // 接続状態
atlas::Statistics g_data;         // 統計データ
std::uint32_t g_data_index = 0;   // 統計データ読み出し用のインデックス
atlas::Mode g_mode;               // モード情報

// ファイルシステム
atlas::FileSys g_fs;

// 音声制御
atlas::AudioPlayer g_player;

// ディスプレイ制御
DisplayDriver g_display(SCREEN_WIDTH, SCREEN_HEIGHT);
atlas::View<DisplayDriver> g_view(g_display, g_data, g_params, g_state);

//=============================================================================
// セットアップ関数
//=============================================================================
void setup()
{
#ifdef DEBUG_MODE
    // シリアル通信開始
    Serial.begin(9600);
    while (!Serial);
#endif

    // ディスプレイの開始
    if (!atlas::begin_display(g_display))
    {
        debug_msg(F("failed to start display"));
        while (true);
    }

    // さめ大臣ロゴ
    g_view.splash_screen();
    ulong t_logo_end = millis() + 1500;  // ロゴ表示時間は1,500ミリ秒間

    // SPIFFS開始
    if (!g_fs.begin(true))
    {
        debug_msg(F("failed to mount SPIFFS"));
        while (true);
    }
    // 統計データの読み込み
    g_data.init();
    if (!g_fs.read(STATISTICS_FILE_NAME, &g_data, sizeof(g_data)))
    {
        debug_msg(F("failed to read data file"));
    }
    // パラメータの読み込み
    g_params.init();
    if (!g_fs.read(PARAMS_FILE_NAME, &g_params, sizeof(g_params)))
    {
        debug_msg(F("failed to read parameter file"));
    }
    g_params.regulate();

//-----------------------------------------------------------------------------
#if ATLAS_FORMAT == ATLAS_FULL_SPEC  // 電動ランチャー制御として使う場合
//-----------------------------------------------------------------------------
    // 音声制御の開始
    if (!g_player.begin(Serial1))
    {
        debug_msg(F("failed to start audio player"));
    }
    // モーター制御インスタンスの設定
    g_motors[0].configure(L_PWM_1, R_PWM_1, LR_EN_1, MOTOR1_MAX_RPM);
#if NUM_MOTORS == 2
    g_motors[1].configure(L_PWM_2, R_PWM_2, LR_EN_2, MOTOR2_MAX_RPM);
#endif
//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------
#if SWITCH_TYPE == SW_SLIDE // スライドスイッチを使う場合
//-----------------------------------------------------------------------------
    // スイッチ用のピン設定
    pinMode(MODE_SW_IN, INPUT_PULLUP);  // プローブは、念のため内蔵のプルアップ抵抗
    pinMode(MODE_SW_OUT, OUTPUT);       // Aモード側は、出力
    digitalWrite(MODE_SW_OUT, HIGH);    // Aモード側のピンをHIGH
    
    // 現在のスイッチ状態をセット
    g_mode.set(digitalRead(MODE_SW_IN) == 1);
    // 切替スイッチの状態を監視するタスクの生成・投入
    xTaskCreateUniversal(
        // タスク
        [](void* pv_params)
        {
            while (true)
            {
                g_mode.set(digitalRead(MODE_SW_IN) == 1);
                delay(50);   // 50ms間隔でチェック
            }
        },
        "task_switch",  // タスク名
        2048,           // スタックメモリ
        nullptr,        // 起動パラメータ
        1,              // 優先度（値が大きいほど優先順位が高い）
        nullptr,        // タスクハンドル
        PRO_CPU_NUM     // コアID
    );
//-----------------------------------------------------------------------------
#elif SWITCH_TYPE == SW_TACT // タクトスイッチを使う場合
//-----------------------------------------------------------------------------
    // スイッチ用のピン設定
    pinMode(MODE_SW_IN, INPUT_PULLUP);  // プローブ（内蔵プルアップ抵抗を使う）
    // スイッチの入力を監視するタスクの生成・投入
    xTaskCreateUniversal(
        // タスク
        [](void* pv_params)
        {
            while (true)
            {
                std::uint64_t counter = 0;
                while (!digitalRead(MODE_SW_IN))
                {
                    counter += 1;
                }
                // 長押し判定
                if (counter > TACT_SW_LONG)
                {
                    g_mode.change();    // モード切り替え
                }
                else if (counter > TACT_SW_SHORT)
                {
                    // オートモードにおいて、BBPを接続しているとき
                    if (g_mode.is_auto_mode() && g_state.bbp_ready())
                    {
                        g_state.next_page_a();
                        g_view.auto_mode_standby();
                    }
                    // マニュアルモードにおいて
                    if (g_mode.is_manual_mode())
                    {
                        g_state.next_page_m();
                        g_view.manual_mode_standby();
                    }
                }
                delay(50);   // 50ms間隔でチェック
            }
        },
        "task_switch",  // タスク名
        2048,           // スタックメモリ
        nullptr,        // 起動パラメータ
        1,              // 優先度（値が大きいほど優先順位が高い）
        nullptr,        // タスクハンドル
        PRO_CPU_NUM     // コアID
    );
//-----------------------------------------------------------------------------
#endif  // #if SWITCH_TYPE == SW_TACT
//-----------------------------------------------------------------------------

    // スプラッシュスクリーン表示限度まで待機実行
    wait_until(t_logo_end);
}

//=============================================================================
// ループ関数
//=============================================================================
void loop()
{
    // BLE接続タスクを回す
    g_mode.is_auto_mode() ? run_auto_mode() : run_manual_mode();
    // おまじない
    delay(1);
}

//=============================================================================
// オートモード
//=============================================================================
void run_auto_mode()
{
    debug_msg(F("[auto mode] in"));

    // 初期化
    g_state.clear();

    // BLE通信開始
    if (!BLE.begin())
    {
        BLE.end();
        return;
    }

    // BBP（ペリフェラル）のスキャン開始
    BLE.scanForName(BBP_LOCAL_NAME);

    // オートモード時
    bool mode_change = false;
    while (g_mode.is_auto_mode())
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
                debug_msg(F("connection failed"));
            }
            // サービス検索
            else if (!dev.discoverService(BBP_SERVICE))
            {
                debug_msg(F("no service"));
            }
            else
            {
                // キャラクタリスティックの購読
                BLECharacteristic chr = dev.characteristic(BBP_CHR_SP);
                if (!chr)
                {
                    // キャラクタリスティックが存在しない
                    debug_msg(F("no characteristic"));
                }
                // 購読できるかどうか
                else if (!chr.canSubscribe())
                {
                    // キャラクタリスティックが購読できない（Notify属性がない）
                    debug_msg(F("not subscribable"));
                }
                // 購読
                else if (!chr.subscribe())
                {
                    // キャラクタリスティックの購読に失敗した
                    debug_msg(F("subscription failed"));
                }
                // セッション開始
                else
                {
                    g_player.se_ack();          // 接続完了のアナウンス音
                    g_state.set_bbp(true);      // 状態を更新
                    g_view.auto_mode_standby(); // 描画

                    // BBPとATLASの通信開始（falseで帰ってくるときはモード切替あり）
                    if (!bbp_session(dev, chr))
                    {
                        mode_change = true;
                        g_state.set_elr(false);
                    }
                }
            }
            
            // BBPとATLASのセッション終了の処理
            g_player.se_cancel();      // 音声案内
            g_state.set_bbp(false);    // 状態更新
            g_state.set_bey(false);    // 状態更新
            g_view.auto_mode_promotion();
            dev.disconnect();          // デバイスからの切断（念のため）

            // オートモードが終了しているなら、ループを抜け出す
            if (mode_change) break;

            // 次の接続のためのスキャン開始
            BLE.scanForName(BBP_LOCAL_NAME);
        }
        // おまじない
        delay(1);
    }
    // リソースの開放
    BLE.end();

    // マニュアル/設定モードへの切り替え時のアイキャッチ
    // アイキャッチが有効になっていない場合は即座に制御が返る
    eyecatch_a2m(g_view, 1500);

    debug_msg(F("[auto mode] out"));
}

// BBPとのBLE通信セッション
// 戻り値が "false" のときモード切替あり
bool bbp_session(BLEDevice& dev, BLECharacteristic& chr)
{
    debug_msg(F("BBP session started"));

    // BBPからのデータ解析準備
    atlas::BBPAnalyzer analyzer;                // 解析
    atlas::BBPData buf;                         // 読み出し用バッファ

//-----------------------------------------------------------------------------
#if ATLAS_FORMAT == ATLAS_FULL_SPEC  // 電動ランチャー制御として使う場合
//-----------------------------------------------------------------------------
    // 射出キャンセル用の信号
    int abort_sig = 1;         // 中止信号（値は適当）
    ulong t_cancel_limit = 0;  // 中止可能期限

    // 射出キャンセル用のキュー
    QueueHandle_t que_mtr = xQueueCreate(1, sizeof(ulong));  // モーター駆動を中止
    QueueHandle_t que_cvw = xQueueCreate(1, sizeof(ulong));  // カウントダウン表示を中止
    QueueHandle_t que_cvo = xQueueCreate(1, sizeof(ulong));  // カウントダウン音声を中止
//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------

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
                debug_msg(F("beyblade has been attached / detached"));
                g_view.auto_mode_standby();   // 表示更新
                break;

            // ベイブレードがシュートされた
            case atlas::BBPState::FINISHED:
                debug_msg(F("beyblade has been shot"));
                g_state.set_bey(false);
                // プロファイル解析
                analyzer.analyze_profile();
                // データ更新
                g_data.update(g_params.is_bbp_sp_main() ?
                              analyzer.bbp_sp() : analyzer.true_sp());
                // 表示更新
                g_view.auto_mode_sp(analyzer.bbp_sp(),
                                    analyzer.true_sp(),
                                    analyzer.max_sp());
                // 保存
                g_fs.write(STATISTICS_FILE_NAME, &g_data, sizeof(g_data));
                // 解析情報クリア
                analyzer.clear();
                break;

            // エラー
            case atlas::BBPState::ERROR:
                debug_msg(F("CRC error"));
                g_state.set_bey(false);
                g_view.auto_mode_error();   // エラー表示
                g_player.se_error();        // エラー音
                break;

//-----------------------------------------------------------------------------
#if ATLAS_FORMAT == ATLAS_FULL_SPEC  // 電動ランチャー制御として使う
//-----------------------------------------------------------------------------
            // BBPのボタンがダブルクリックされた
            case atlas::BBPState::ELR_ENABLED:
                debug_msg(F("ELR enabled"));
                g_player.se_ack();          // ACK音
                g_state.set_elr(true);      // 電動ランチャーを有効にする
                g_view.auto_mode_standby(); // 表示更新
                break;

            // BBPのボタンがダブルクリックされた
            case atlas::BBPState::ELR_DISABLED:
                debug_msg(F("ELR disabled"));
                g_player.se_cancel();       // キャンセル音
                g_state.set_elr(false);     // 電動ランチャーを無効にする
                g_view.auto_mode_standby(); // 表示更新
                break;

            // 射出命令
            case atlas::BBPState::SHOOT_ORDERED:
                debug_msg(F("shoot order"));
                // キューを空にしておく
                xQueueReceive(que_mtr, &abort_sig, 0);  // モーター用
                xQueueReceive(que_cvw, &abort_sig, 0);  // カウントダウン表示用
                xQueueReceive(que_cvo, &abort_sig, 0);  // カウントダウン音声用
                // 現在時刻とキャンセル可能期限時刻の記録
                g_t0.store(millis());
                t_cancel_limit = g_t0.load() + g_params.latency();
                // モータータスクの投入
                xTaskCreateUniversal(
                    task_motor_auto,       // タスク
                    "task_motor_auto",     // タスク名
                    4096,                  // スタックメモリ
                    que_mtr,               // 起動パラメータ
                    3,                     // 優先度（値が大きいほど優先順位が高い）
                    nullptr,               // タスクハンドル
                    PRO_CPU_NUM            // コアID
                );
                // カウントダウン表示タスクの投入
                xTaskCreateUniversal(
                    task_count_view,       // タスク
                    "task_count_view",     // タスク名
                    4096,                  // スタックメモリ
                    que_cvw,               // 起動パラメータ
                    2,                     // 優先度（値が大きいほど優先順位が高い）
                    nullptr,               // タスクハンドル
                    PRO_CPU_NUM            // コアID
                );
                // カウントダウン音声タスクの投入
                if (g_player.enabled())
                {
                    xTaskCreateUniversal(
                        task_count_voice,      // タスク
                        "task_count_voice",    // タスク名
                        4096,                  // スタックメモリ
                        que_cvo,               // 起動パラメータ
                        2,                     // 優先度（値が大きいほど優先順位が高い）
                        nullptr,               // タスクハンドル
                        PRO_CPU_NUM            // コアID
                    );
                }
                break;

            // 電動ランチャー連動のシュートか、射出命令キャンセル
            case atlas::BBPState::BEY_DETACHED_2:
                // キャンセル可能期間内だったら
                if (millis() < t_cancel_limit)
                {
                    debug_msg(F("cancel order"));
                    // 各タスクに中止信号を送る
                    xQueueSend(que_mtr, &abort_sig, 0);
                    xQueueSend(que_cvw, &abort_sig, 0);
                    if (g_player.enabled())
                    {
                        xQueueSend(que_cvo, &abort_sig, 0);
                    }
                }
                break;

//-----------------------------------------------------------------------------
#elif SWITCH_TYPE != SW_SLIDE  // スイッチレスか、タクトスイッチ（SP計測器のみ）
//-----------------------------------------------------------------------------
            // BBPのボタンがダブルクリックされた
            case atlas::BBPState::ELR_ENABLED:
            case atlas::BBPState::ELR_DISABLED:
                debug_msg(F("BBP button double-clicked"));
                // モードをマニュアル/設定モードに切り替える
                g_mode.set(false);
                break;
//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------
            default:
                break;
            }
            // バッファの初期化
            buf.init();
        }
        // モードがマニュアルに変わった
        if (g_mode.is_manual_mode()) return false;

        // おまじない
        delay(1);
    }
    return true;
}

//=============================================================================
// マニュアルモード
//=============================================================================
void run_manual_mode()
{
    debug_msg(F("[manual mode] in"));

    // 初期化
    g_state.clear();

    // BLE通信開始
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
            debug_msg(F("client connected"));
            g_player.se_ack();             // ACK音を鳴らす
            g_state.set_client(true);      // クライアントが接続された
            g_view.manual_mode_standby();  // 画面更新
        }
    );

    // 接続が切れたときのハンドラ
    BLE.setEventHandler(
        BLEDisconnected,
        [](BLEDevice central)
        {
            debug_msg(F("client disconnected"));
            g_player.se_cancel();          // キャンセル音を鳴らす
            g_state.set_client(false);     // クライアントが切断された
            g_view.manual_mode_standby();  // 画面更新
        }
    );

    // サービスの作成
    BLEService serv(ATLAS_SERVICE);

    // デバイス情報
    {
        // キャラクタリスティックの作成と初期値の書き込み
        BLETypedCharacteristic<atlas::Statistics::Header> ch(ATLAS_CHR_DEVINFO, BLERead);
        // キャラクタリスティックの説明文
        BLEDescriptor descr("2901", ATLAS_CHR_DEVINFO_DESCR);
        ch.addDescriptor(descr);
        // イベントハンドラの登録: 読み出し
        ch.setEventHandler(
            BLERead,
            [](BLEDevice central, BLECharacteristic chr)
            {
                debug_msg(F("read device info"));
                chr.writeValue(&g_devinfo, sizeof(g_devinfo));  // デバイス情報の書き込み
            }
        );
        // キャラクタリスティックの登録
        serv.addCharacteristic(ch);
    }

    // パラメータ設定
    {
        // キャラクタリスティックの作成と初期値の書き込み
        BLETypedCharacteristic<atlas::Params> ch(ATLAS_CHR_SET, BLERead | BLEWrite);
        ch.writeValue(g_params);
        // キャラクタリスティックの説明文
        BLEDescriptor descr("2901", ATLAS_CHR_SET_DESCR);
        ch.addDescriptor(descr);
        // イベントハンドラの登録：書き込み
        ch.setEventHandler(
            BLEWritten,
            [](BLEDevice central, BLECharacteristic chr)
            {
                debug_msg(F("write parameters"));
                chr.readValue(&g_params, sizeof(g_params)); // パラメータの読み込み
                g_params.regulate();                        // パラメータの正規化
                g_state.set_page_m(1);                      // パラメータのページに設定
                g_view.manual_mode_standby();               // 画面更新
                g_player.se_ack();                          // ACK音
                // パラメータをファイルに保存する
                g_fs.write(PARAMS_FILE_NAME, &g_params, sizeof(g_params));
            }
        );
        // イベントハンドラの登録：読み出し
        ch.setEventHandler(
            BLERead,
            [](BLEDevice central, BLECharacteristic chr)
            {
                debug_msg(F("read parameters"));
                g_params.regulate();                         // パラメータの正規化
                chr.writeValue(&g_params, sizeof(g_params)); // 書き込み
            }
        );
        // キャラクタリスティックの登録
        serv.addCharacteristic(ch);
    }
    
    // データ（ヘッダー）
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
                debug_msg(F("read data header"));
                // ヘッダ情報をキャラクタリスティックに書き込む
                chr.writeValue(g_data.header(),
                               sizeof(atlas::Statistics::Header));
                // データインデックスの初期化
                g_data_index = 0;
            }
        );
        // キャラクタリスティックの登録
        serv.addCharacteristic(ch);
    }

    // データ（ヒストグラム）
    {
        // キャラクタリスティックの作成と初期値の書き込み
        BLETypedCharacteristic<atlas::Statistics::Histogram8> ch(ATLAS_CHR_DATA, BLERead);
        // キャラクタリスティックの説明文
        BLEDescriptor descr("2901", ATLAS_CHR_DATA_DESCR);
        ch.addDescriptor(descr);
        // イベントハンドラの登録: 読み出し
        ch.setEventHandler(
            BLERead,
            [](BLEDevice central, BLECharacteristic chr)
            {
                debug_msg(F("read data histogram"));
                if (g_data_index >= atlas::Statistics::NUM_HISTS)
                {
                    g_data_index = 0;
                }
                // SP統計データをキャラクタリスティックに書き込む
                chr.writeValue(g_data.hist(g_data_index++),
                               sizeof(atlas::Statistics::Histogram8));
            }
        );
        // キャラクタリスティックの登録
        serv.addCharacteristic(ch);
    }

    // データクリア
    {
        // キャラクタリスティックの作成と初期値の書き込み
        BLETypedCharacteristic<std::uint8_t> ch(ATLAS_CHR_CLEAR, BLEWrite);
        // キャラクタリスティックの説明文
        BLEDescriptor descr("2901", ATLAS_CHR_CLEAR_DESCR);
        ch.addDescriptor(descr);
        // イベントハンドラの登録：書き込み
        ch.setEventHandler(
            BLEWritten,
            [](BLEDevice central, BLECharacteristic chr)
            {
                debug_msg(F("clear data"));
                g_data.init();                 // SP統計データを初期化する
                g_state.set_page_m(1);         // パラメータのページに設定
                g_view.manual_mode_standby();  // 画面更新
                g_player.se_ack();             // ACK音を鳴らす
                // SP統計データ（空）をファイルに保存する
                g_fs.write(STATISTICS_FILE_NAME, &g_data, sizeof(g_data));
            }
        );
        // キャラクタリスティックの登録
        serv.addCharacteristic(ch);
    }

//-----------------------------------------------------------------------------
#if ATLAS_FORMAT == ATLAS_FULL_SPEC  // 電動ランチャー制御として使う場合
//-----------------------------------------------------------------------------
    // 手動シュート
    {
        // キャラクタリスティックの作成と初期値の書き込み
        BLETypedCharacteristic<std::uint8_t> ch(ATLAS_CHR_SHOOT, BLEWrite);
        // キャラクタリスティックの説明文
        BLEDescriptor descr("2901", ATLAS_CHR_SHOOT_DESCR);
        ch.addDescriptor(descr);
        // イベントハンドラの登録：書き込み
        ch.setEventHandler(
            BLEWritten,
            [](BLEDevice central, BLECharacteristic chr)
            {
                debug_msg(F("manual shoot"));
                // モータータスクの投入
                char buf[14];
                g_t0.store(millis());
                for (std::uint32_t i = 0; i < NUM_MOTORS; ++i)
                {
                    std::sprintf(buf, "task_motor_m%lu", i+1);
                    if (g_params.elr(i).enabled_manual())
                    {
                        xTaskCreateUniversal(
                            task_motor_manual,  // タスク
                            buf,                // タスク名
                            4096,               // スタックメモリ
                            &i,                 // 起動パラメータ（モーターID）
                            3,                  // 優先度（値が大きいほど優先順位が高い）
                            nullptr,            // タスクハンドル
                            PRO_CPU_NUM         // コアID
                        );
                    }
                }
            }
        );
        // キャラクタリスティックの登録
        serv.addCharacteristic(ch);
    }
//-----------------------------------------------------------------------------
#elif SWITCH_TYPE != SW_SLIDE  // スイッチレスか、タクトスイッチ（SP計測器のみ）
//-----------------------------------------------------------------------------
    // モード切替
    {
        // キャラクタリスティックの作成と初期値の書き込み
        BLETypedCharacteristic<std::uint8_t> ch(ATLAS_CHR_SWITCH, BLEWrite);
        // キャラクタリスティックの説明文
        BLEDescriptor descr("2901", ATLAS_CHR_SWITCH_DESCR);
        ch.addDescriptor(descr);
        // イベントハンドラの登録：書き込み
        ch.setEventHandler(
            BLEWritten,
            [](BLEDevice central, BLECharacteristic chr)
            {
                debug_msg(F("switch to auto mode"));
                g_mode.set(true);     // オートモードに切り替え
            }
        );
        // キャラクタリスティックの登録
        serv.addCharacteristic(ch);
    }
//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------

    // ローカル名の設定
    BLE.setLocalName(ATLAS_LOCAL_NAME);

    // アドバタイズの設定と開始
    BLE.setAdvertisedService(serv);
    BLE.addService(serv);
    BLE.advertise();

    // 表示
    g_state.set_client(false);
    g_view.manual_mode_standby();

    // マニュアルモード時
    while (g_mode.is_manual_mode())
    {
        BLE.poll(1000);   // 1秒間隔でタイムアウト
        delay(1);         // おまじない
    }

    // リソースの開放
    BLE.end();

    // リセット（これをしないと、サービスが重複して登録されていく）
    // BLE.end()だけじゃダメなのか…？
    ESP.restart();

    debug_msg(F("[manual mode] out"));
}

//-----------------------------------------------------------------------------
#if ATLAS_FORMAT == ATLAS_FULL_SPEC  // 電動ランチャー制御として使う場合
//-----------------------------------------------------------------------------

// モーター制御: マニュアルモード
void task_motor_manual(void* pv_params)
{
    // モーターIDの取得
    std::uint32_t id = *(static_cast<std::uint32_t*>(pv_params));

    // 発射
    rotate_motor(g_motors[id], g_params.elr(id));       // 回転開始
    wait_until(g_t0.load() + MOTOR_PREPARATORY_TIME);   // 回転が安定するまで待機
    g_motors[id].stop();                                // 射出

    // タスク終了処理
    vTaskDelete(nullptr);
}

// モーター制御: オートモード
void task_motor_auto(void* pv_params)
{
    // 駆動するモーターインスタンスのキャッシュ
    auto& motor = g_motors[g_params.automode_elr_index()];
    // 中止信号を待つ
    if (standby(static_cast<QueueHandle_t>(pv_params), g_params.latency()))
    {
        // 回転開始
        rotate_motor(motor, g_params.automode_elr());
        // 待機実行（射出時刻の計算）
        wait_until(
            SYNC_ADJ_TIME +         // 同期調整時間
            g_t0.load() +           // 射出指令時のシステム時間（ミリ秒）の取り出し
            g_params.delay() +      // 射出遅延時間
            g_params.latency() +    // ReadySet--3の間の猶予時間
            COUNTDOWN_INTERVAL * 4  // カウントダウン号令 3--Shoot の間の合計時間
        );
        // モーターの急停止による射出実行
        motor.stop();
    }
    // タスク終了処理
    vTaskDelete(nullptr);
}

// 射出前カウントダウン表示
void task_count_view(void* pv_params)
{
    // "Ready Set"の表示
    g_view.auto_mode_countdown(0);
    // 各表示の開始時刻
    ulong t_view = g_t0.load() + g_params.latency() + SYNC_ADJ_TIME;
    // 中止信号を待つ
    if (standby(static_cast<QueueHandle_t>(pv_params), g_params.latency()))
    {
        // カウントダウン 3, 2, 1, Go
        for (int i = 1; i < 5; ++i)
        {
            // 同期調整
            wait_until(t_view);
            // カウントダウンメッセージ
            g_view.auto_mode_countdown(i);
            // 次の時間
            t_view += COUNTDOWN_INTERVAL;
        }
        // 同期調整
        wait_until(t_view);
        // SHOOT!
        g_view.auto_mode_countdown(5);
    }
    else
    {
        // 中止信号が来たので中止のメッセージを表示する
        g_view.auto_mode_aborted();
    }
    // タスク終了処理
    vTaskDelete(nullptr);
}

// 射出前カウントダウン音声
void task_count_voice(void* pv_params)
{
    // 中止信号を待つ
    if (standby(static_cast<QueueHandle_t>(pv_params), g_params.latency()))
    {
        g_player.countdown();  // カウントダウン音声
    }
    else
    {
        // 中止信号が来たので中止SEを流す
        g_player.se_cancel();
    }
    // タスク終了処理
    vTaskDelete(nullptr);
}

//-----------------------------------------------------------------------------
#endif  // #if ATLAS_FORMAT == ATLAS_FULL_SPEC
//-----------------------------------------------------------------------------

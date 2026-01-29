/*
    ATLAS - AuTomatic LAuncher System
    Copyright (C) 2025  Shark Minister
    All right reserved.

    https://x.com/shark_minister
*/
#ifndef ATLAS_SETTING_HH
#define ATLAS_SETTING_HH

//-----------------------------------------------------------------------------
// ハードウェアの実装
//-----------------------------------------------------------------------------

// デバッグモード（有効にする場合はコメント//を外す）
//#define DEBUG_MODE

// 利用形態
#define ATLAS_FULL_SPEC     0   // ATLASフルスペック。電動ランチャー制御としても使う
#define ATLAS_LITE_SP_ONLY  1   // ATLAS lite。SP計測器のみで使う
#define ATLAS_FORMAT        ATLAS_LITE_SP_ONLY

// モード切替スイッチの種類
#define SW_NONE      0  //  スイッチなし（物理スイッチが無効になる）
#define SW_SLIDE     1  //  スライドスイッチ
#define SW_TACT      2  //  タクトスイッチ
#define SWITCH_TYPE  SW_TACT

// ディスプレイの種類
#define  ADAFRUIT_SH1106G  1   // SH1106G (1.3インチ)
#define  ADAFRUIT_SSD1306  2   // SSD1306 (0.96インチ)
#define  DISPLAY_DRIVER    ADAFRUIT_SSD1306

// 使用するモーターの数
#define  NUM_MOTORS  1

// モーターをダミーモードにする（有効にする場合はコメント//を外す）
#define  USE_DUMMY_MOTOR

// AモードからMモードに切り替え時にアイキャッチを表示（有効にする場合はコメント//を外す）
#define  USE_EYECATCH_A2M

//-----------------------------------------------------------------------------
// ピン割り当て：配線にあわせて変更する
//-----------------------------------------------------------------------------

/*
    フルスペック／初号機／スライドスイッチ
     - MODE_SW_IN  = 8
     - MODE_SW_OUT = 5

    SP計測／量産機／スライドスイッチ
     - MODE_SW_IN  = 5
     - MODE_SW_OUT = 21

    ----------------------------------------

    SP計測／初号機／タクトスイッチ
     - MODE_SW_IN  = 21

    SP計測／量産機／タクトスイッチ
     - MODE_SW_IN  = 5

    フルスペック／量産機／タクトスイッチ
     - MODE_SW_IN  = 8
*/
#define  MODE_SW_IN   5   // 切替スイッチの入力端子
#define  MODE_SW_OUT  8   // 切替スイッチの出力端子（スライドスイッチ用）

// モータードライバーと接続するピン
#define  L_PWM_1   2    // モーター1: L_PWMを接続
#define  R_PWM_1   3    // モーター1: R_PWMを接続
#define  LR_EN_1   4    // モーター1: L_ENとR_ENを接続
#define  L_PWM_2  11    // モーター2: 左回転のPWM
#define  R_PWM_2  10    // モーター2: 右回転のPWM
#define  LR_EN_2   4    // モーター2: L_EN, R_LEN

// オーディオプレイヤー
#define  AUDIO_RX  20  // シリアル通信RX
#define  AUDIO_TX  21  // シリアル通信TX

//-----------------------------------------------------------------------------
// ディスプレイパラメータ
//-----------------------------------------------------------------------------

#define  SCREEN_WIDTH    128   // スクリーン幅
#define  SCREEN_HEIGHT    64   // スクリーン高さ
#define  SCREEN_ADDR    0x3C   // ディスプレイのI2Cアドレス

//-----------------------------------------------------------------------------
// モーター設定
//-----------------------------------------------------------------------------

/*
    OP.68 RS-540 スポーツチューンモーター
    ・適正電圧： 6-8.4 V
    ・最高効率時トルク： 350 gf･cm (7.2V)
    ・最高効率時回転数： 18300 rpm (7.2V)

    OP.1393 380 スポーツチューンモーター
    ・適正電圧： 6.6-7.2 V
    ・最高効率時トルク： 129 gf･cm (7.2V)
    ・無負荷時の回転数： 24900 rpm (7.2V)
*/
#define  MOTOR1_MAX_RPM  24900   // モーター1の最大回転数
#define  MOTOR2_MAX_RPM  24900   // モーター2の最大回転数

//-----------------------------------------------------------------------------
// 音声設定
//-----------------------------------------------------------------------------

/*
    DFPlayer Miniに使うmicroSDカード内の音声ファイル番号を指定する。
     - 01: カウントダウン音声（3, 2, 1, Go, shoot）
     - 02: 成功の効果音
     - 03: キャンセルの効果音
     - 04: エラーの効果音
*/
#define AUDIO_COUNTDOWN    1   // "3, 2, 1, Go, Shoot!"
#define AUDIO_SE_ACK       2   // 成功の効果音
#define AUDIO_SE_CANCEL    3   // キャンセルの効果音
#define AUDIO_SE_ERROR     4   // エラーの効果音

// デフォルト音量
#define DEFAULT_VOLUME    20

//-----------------------------------------------------------------------------
// 動作パラメータ設定
//-----------------------------------------------------------------------------

/*
    電動ランチャーのSPデフォルト値

    電動ランチャーでベイを射出するときのシュートパワーSPのデフォルト値を指定する。
    SPの内部パラメータは、マニュアル/設定モード時に外部からBLE通信で変更できる。
*/
#define  DEFAULT_LAUNCHER_SP  10000

/*
    射出遅延時間のデフォルト値 [ms]

    オートモードでのカウントダウン最後の "Shoot!" の言い始めと
    モーター停止（ベイ射出）の間隔（射出遅延）のデフォルト値をミリ秒で指定する。
    遅延時間の内部パラメータは、マニュアル/設定モード時に外部からBLE通信で変更できる。
*/
#define  DEFAULT_DELAY  0

/*
    猶予時間のデフォルト値 [ms]

    オートモードでのベイ設置感知の "ReadySet" 表示 と
    カウントダウン開始 "3" までの間隔のデフォルト値をミリ秒で指定する。
    この猶予時間内にベイをランチャーから外すとカウントダウンと
    モーター駆動開始がキャンセルされる。
    猶予時間の内部パラメータは、マニュアル/設定モード時に外部からBLE通信で変更できる。
*/
#define  DEFAULT_LATENCY  1300

/*
    電動ランチャーのSP上限値（ソフトウェアリミット）

    電動ランチャーでベイを射出するときのシュートパワーSPの上限値を指定する。
    モーターの最高回転数以下にすることが望ましい。
    この値は外部から変更できない。

    ※想定するモーターの最高回転数
    ・OP.68 RS-540 スポーツチューンモーター： 18300 rpm (7.2V)
    ・OP.1393 380 スポーツチューンモーター：  24900 rpm (7.2V)
*/
#define  LAUNCHER_SP_UPPER_LIMIT  24900

/*
    電動ランチャーのSP下限値（ソフトウェアリミット）

    電動ランチャーでベイを射出するときのシュートパワーSPの下限値を指定する。
    この値は外部から変更できない。
*/
#define  LAUNCHER_SP_LOWER_LIMIT  3000

/*
    猶予時間の下限値（ソフトウェアリミット） [ms]

    オートモードでのカウントダウン最初の "ReadySet" と "3" までの
    間隔の下限値をミリ秒で指定する。
    この値は外部から変更できない。
*/
#define  LATENCY_LOWER_LIMIT  500

/*
    射出遅延時間の上限値（ソフトウェアリミット） [ms]

    オートモードでのカウントダウン最後の "Shoot!" の言い始めと
    モーター停止（ベイ射出）の間隔（射出遅延）の上限値をミリ秒で指定する。
    この値は外部から変更できない。
*/
#define  DELAY_UPPER_LIMIT  500

/*
    カウントダウンコールの間隔 [ms]

    カウントダウンコールは、"3, 2, 1, Go, Shoot!" であり、
    そのコール間隔をミリ秒で指定する。
    この値は外部から変更できない。
    原則的に変更しない。
*/
#define  COUNTDOWN_INTERVAL  1000

/*
    モーター安定の猶予時間 [ms]
    
    マニュアル射出の際、モーターが最大回転数になっても回転が安定するまでに猶予時間を
    とっておいた方が良いので、それを考慮した値をミリ秒で指定する。
    この値は外部から変更できない。
*/
#define  MOTOR_PREPARATORY_TIME  2000

/*
    オートモード射出・音声・表示の同期調整時間 [ms]

    音声タスクとモーター駆動タスク・表示タスクの同期をとるための調整時間。
    この値は外部から変更できない。
*/
#define  SYNC_ADJ_TIME  300

//-----------------------------------------------------------------------------
// タクトスイッチ設定
//-----------------------------------------------------------------------------

// タクトスイッチのゲート設定
#define TACT_SW_SHORT   30000  // クリック判定
#define TACT_SW_LONG   700000  // 長押し判定

//-----------------------------------------------------------------------------
// ページ設定
//-----------------------------------------------------------------------------

#define MAX_PAGE_A   3  // オートモードの待機画面のページ数
#define MAX_PAGE_M   5  // マニュアル/設定モードの待機画面のページ数

//-----------------------------------------------------------------------------
// システム設定（変更しないこと！）
//-----------------------------------------------------------------------------

// バージョン情報
#define  MAJOR_VERSION  1
#define  MINOR_VERSION  3
#define  REVISION       0

// ファイル名
#define  PARAMS_FILE_NAME         "/params.dat"
#define  STATISTICS_FILE_NAME     "/statistics.dat"

// BLEペリフェラル側のGATT通信設定
#define  ATLAS_LOCAL_NAME         "ATLAS_AUTO_LAUNCHER"
#define  ATLAS_SERVICE            "32150000-9A86-43AC-B15F-200ED1B7A72A"
#define  ATLAS_CHR_SET            "32150001-9A86-43AC-B15F-200ED1B7A72A"
#define  ATLAS_CHR_SET_DESCR      "Parameters for Electric Launcher"
#define  ATLAS_CHR_SHOOT          "32150020-9A86-43AC-B15F-200ED1B7A72A"
#define  ATLAS_CHR_SHOOT_DESCR    "Manual shoot"
#define  ATLAS_CHR_HEADER         "32150030-9A86-43AC-B15F-200ED1B7A72A"
#define  ATLAS_CHR_HEADER_DESCR   "Read header for SP statistics"
#define  ATLAS_CHR_DATA           "32150031-9A86-43AC-B15F-200ED1B7A72A"
#define  ATLAS_CHR_DATA_DESCR     "Read SP-histogram data"
#define  ATLAS_CHR_CLEAR          "32150040-9A86-43AC-B15F-200ED1B7A72A"
#define  ATLAS_CHR_CLEAR_DESCR    "Clear data in flash memory"
#define  ATLAS_CHR_SWITCH         "32150050-9A86-43AC-B15F-200ED1B7A72A"
#define  ATLAS_CHR_SWITCH_DESCR   "Switch to auto mode for switchless controller"
#define  ATLAS_CHR_DEVINFO        "32150060-9A86-43AC-B15F-200ED1B7A72A"
#define  ATLAS_CHR_DEVINFO_DESCR  "Device information"

// BLEセントラル側のGATT設定（ベイバトルパスのパラメータ）
#define  BBP_LOCAL_NAME  "BEYBLADE_TOOL01"
#define  BBP_SERVICE     "55c40000-f8eb-11ec-b939-0242ac120002"
#define  BBP_CHR_SP      "55c4f002-f8eb-11ec-b939-0242ac120002"

#endif
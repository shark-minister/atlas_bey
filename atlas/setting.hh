/*
    ATLAS - AuTomatic LAuncher System
    Copyright (C) 2025  Shark Minister
    All right reserved.

    https://x.com/shark_minister
*/
#ifndef ATLAS_SETTING_HH
#define ATLAS_SETTING_HH

//#define DEBUG_MODE

//-----------------------------------------------------------------------------
// ハードウェアの実装
//-----------------------------------------------------------------------------

// 利用形態
// - 0: 電動ランチャー制御としても使う
// - 1: SP計測器のみで使う
#define SP_MEAS_ONLY  1

// スイッチレスでモード切替を行うかどうか
// - 0: スイッチでモードを切り替える
// - 1: スイッチレスのコントローラ（物理スイッチが無効になります）
#define SWITCH_LESS  0

//-----------------------------------------------------------------------------
// デバイス関連
//-----------------------------------------------------------------------------

// ディスプレイ設定
#define  SCREEN_WIDTH        128   // スクリーン幅
#define  SCREEN_HEIGHT        64   // スクリーン高さ
#define  SCREEN_ADDR        0x3C   // ディスプレイのI2Cアドレス
#define  DISPLAY_IS_SPI        0   // ディスプレイがSPI接続なら1, I2Cなら0
#define  SPI_MASK          0x100   // SPI接続用のマスク（原則変更しない）
#define  ADAFRUIT_SH1106G      1   // SH1106G (1.3インチ)
#define  ADAFRUIT_SSD1306      2   // SSD1306 (0.96インチ)
#define  DISPLAY_DRIVER        ADAFRUIT_SSD1306
#if  DISPLAY_IS_SPI == 1
#define  DISPLAY_DRIVER        (DISPLAY_DRIVER | SPI_MASK)
#endif

// ピン割り当て
#define  MODE_SW_IN   5   // 切替スイッチの中央端子と繋ぐピン番号
#define  MODE_SW_OUT 21   // 切替スイッチの電源側端子と繋ぐピン番号
#define  L_PWM_1      2   // モータードライバー1のL_PWMをつなぐGPIOピン番号
#define  R_PWM_1      3   // モータードライバー1のR_PWMをつなぐGPIOピン番号
#define  LR_EN_1      4   // モータードライバー1のL_ENとR_ENをつなぐGPIOピン番号
//#define  L_PWM_2    9   // モーター2の左回転用のPWMピン番号
//#define  R_PWM_2   10   // モーター2の右回転用のPWMピン番号
//#define  LR_EN_2    5   // モーター2のL_EN, R_LENの両方を繋ぐピン番号
#define  SCREEN_SPI_MOSI  10  // SDA --> MOSI
#define  SCREEN_SPI_DC     9  //  DC --> MISO
#define  SCREEN_SPI_CLK    8  // SCL --> SCK
#define  SCREEN_SPI_RESET  7  // RES -->
#define  SCREEN_SPI_CS    20  //  CS --> SS (GPIO20)

/*
    モーター設定

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
#define  NUM_MOTORS          1   // 使用するモーターの数
#define  USE_DUMMY_MOTOR     1   // モーターをダミーモードにする

/*
    音声設定
*/
#define AUDIO_COUNTDOWN    1   // "3, 2, 1, Go, Shoot!"
#define AUDIO_SE_ACK       2   // 成功の効果音
#define AUDIO_SE_CANCEL    3   // キャンセルの効果音
#define AUDIO_SE_ERROR     4   // エラーの効果音

//-----------------------------------------------------------------------------
// 動作パラメータ設定
//-----------------------------------------------------------------------------

/*
    電動ランチャーのSPデフォルト値

    電動ランチャーでベイを射出するときのシュートパワーSPのデフォルト値を指定する。
    SPの内部パラメータは、外部からBLE通信で変更できる。
*/
#define  DEFAULT_LAUNCHER_SP  10000

/*
    射出遅延時間のデフォルト値 [ms]

    オートモードでのカウントダウン最後の "Shoot!" の言い始めと
    モーター停止（ベイ射出）の間隔（射出遅延）のデフォルト値をミリ秒で指定する。
    遅延時間の内部パラメータは、外部からBLE通信で変更できる。
*/
#define  DEFAULT_DELAY  0

/*
    猶予時間のデフォルト値 [ms]

    オートモードでのカウントダウン最初の "ReadySet" と "3" までの
    間隔のデフォルト値をミリ秒で指定する。この猶予時間内にベイをランチャーから
    外すとカウントダウンとモーター駆動開始がキャンセルされる。
    猶予時間の内部パラメータは、外部からBLE通信で変更できる。
*/
#define  DEFAULT_LATENCY  1300

/*
    電動ランチャーのSP上限値（ソフトウェアリミット）

    電動ランチャーでベイを射出するときのシュートパワーSPの上限値を指定する。
    モーターの最高回転数以下にすることが望ましい。
    この値は外部から変更できない。
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
    モーター加速の余裕時間 [ms]
    
    モーターが最大回転数になるまでにはある程度の時間が必要であり、
    それを考慮した値をミリ秒で指定する。
    この値は外部から変更できない。
*/
#define  MOTOR_PREPARATORY_TIME  2000

/*
    カウントダウンコールの間隔 [ms]

    カウントダウンコールは、"3, 2, 1, Go, Shoot!" であり、
    そのコール間隔をミリ秒で指定する。
    この値は外部から変更できない。
    原則的に変更しない。
*/
#define  COUNTDOWN_INTERVAL  1000

//-----------------------------------------------------------------------------
// システム設定（変更しないこと！）
//-----------------------------------------------------------------------------

// ファイル名
#define  PARAMS_FILE_NAME  "/params.dat"
#define  STATISTICS_FILE_NAME  "/statistics.dat"

// BLEペリフェラル側のGATT通信設定
#define  ATLAS_LOCAL_NAME         "ATLAS_AUTO_LAUNCHER"
#define  ATLAS_SERVICE            "32150000-9A86-43AC-B15F-200ED1B7A72A"
#define  ATLAS_CHR_SET            "32150001-9A86-43AC-B15F-200ED1B7A72A"
#define  ATLAS_CHR_SET_DESCR      "Parameters for Electric Launcher"
#define  ATLAS_CHR_SAVE           "32150010-9A86-43AC-B15F-200ED1B7A72A"
#define  ATLAS_CHR_SAVE_DESCR     "Save parameters to flash memory"
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

// BLEセントラル側のGATT設定（ベイバトルパスのパラメータ）
#define  BBP_LOCAL_NAME  "BEYBLADE_TOOL01"
#define  BBP_SERVICE     "55c40000-f8eb-11ec-b939-0242ac120002"
#define  BBP_CHR_SP      "55c4f002-f8eb-11ec-b939-0242ac120002"

#endif

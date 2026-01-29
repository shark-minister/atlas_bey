/*
    ATLAS - AuTomatic LAuncher System
    Copyright (C) 2025  Shark Minister
    All right reserved.

    https://x.com/shark_minister
*/
#include "view.hh"

// C++標準ライブラリ
#include <cstdio>   // std::sprintf
#include <cstring>  // std::strlen
#include <cmath>    // std::ceil

// ATLAS
#include "images.hh"
#include "lock.hh"

namespace atlas
{
//=============================================================================

ViewBase::ViewBase(Adafruit_GFX& driver,
                   const Statistics& data,
                   const Params& params,
                   const State& state)
    : _driver(&driver)
    , _data(&data)
    , _params(&params)
    , _state(&state)
    , _smp(xSemaphoreCreateBinary())
{
    // セマフォの開放（これをしないと機能しない）
    xSemaphoreGive(_smp);
}

void ViewBase::_text(std::int16_t x,
                     std::int16_t y,
                     std::uint8_t size,
                     const char* text)
{
    _driver->setTextSize(size);
    _driver->setCursor(x, y);
    _driver->print(text);
}

void ViewBase::_number(std::int16_t x,
                       std::int16_t y,
                       std::uint8_t size,
                       std::int32_t number)
{
    _driver->setTextSize(size);
    _driver->setCursor(x, y);
    _driver->print(number);
}

void ViewBase::_number_w9(std::int16_t x,
                          std::int16_t y,
                          std::uint32_t number,
                          std::int16_t n_digits)
{
    this->_number_img(x, y, number, n_digits, 9, 14, img::digit_w9_list);
}

void ViewBase::_number_w18(std::int16_t x,
                           std::int16_t y,
                           std::uint32_t number,
                           std::int16_t n_digits)
{
    this->_number_img(x, y, number, n_digits, 18, 24, img::digit_w18_list);
}

void ViewBase::_number_img(std::int16_t x,
                           std::int16_t y,
                           std::uint32_t number,
                           std::int16_t n_digits,
                           std::int16_t w,
                           std::int16_t h,
                           const std::uint8_t** img_list)
{
    // 桁数の取得、文字列化
    char buf[21];
    std::int16_t n = std::sprintf(buf, "%u", number);

    // 表示
    if (n_digits > 0)
    {
        for (std::int16_t i = n-1; i >= 0; i -= 1)
        {
            this->_image(
                x + w * (i + n_digits - n), y,
                img_list[static_cast<int>(buf[i])-0x30],
                w, h
            );
        }
    }
    else
    {
        for (std::uint32_t i = 0; i < n; i += 1)
        {
            this->_image(
                x+w*i, y,
                img_list[static_cast<int>(buf[i])-0x30],
                w, h
            );
        }
    }
}

//=============================================================================
// スプラッシュスクリーン
//=============================================================================
void ViewBase::splash_screen()
{
    // ロック
    Lock lock(_smp);

    // 画面のクリア
    this->_clear_display();
    
    // さめ大臣のロゴ表示
    this->_image(26, 8, img::shark_minister_logo, 76, 52);

    // バージョン表記
    _driver->setTextColor(_color);
    char buf[16];
    auto n = std::sprintf(buf, "%u.%u.%u",
                          MAJOR_VERSION, MINOR_VERSION, REVISION);
    this->_text(128-n*6, 0, 1, buf);

    // 表示
    this->_show_display();
}

void ViewBase::image(std::int16_t x,
                     std::int16_t y,
                     const std::uint8_t* bitmap,
                     std::int16_t w,
                     std::int16_t h)
{
    // ロック
    Lock lock(_smp);

    // 画面のクリア
    this->_clear_display();
    
    // 画像表示
    this->_image(x, y, bitmap, w, h);

    // 表示
    this->_show_display();
}

//=============================================================================
// 共通
//=============================================================================

void ViewBase::_show_page_info(const char* page_header)
{
    // ページ名
    this->_text(128 - std::strlen(page_header)*6, 0, 1, page_header);
}

/*
    統計情報の表示
    ・トータルシュート回数
    ・最大、最小SP値
    ・平均SP値
    ・標準偏差
    ・最新SP値
*/
void ViewBase::_show_stats()
{
    // 最新SP
    this->_text(0, 16, 1, "LAST");
    this->_number_w9(28, 16, _data->latest_sp(), 5);
    // シュート数
    this->_text(83, 22, 1, "#");
    this->_number_w9(91, 16, _data->total());

    // 平均SP
    this->_text(0, 33, 1, "MEAN");
    this->_number_w9(28, 33, _data->avg_sp(), 5);
    // 標準偏差
    this->_image(75, 36, img::pm_symbol, 6, 8);
    this->_number_w9(83, 33, _data->std_sp());

    // 最大／最小
    this->_image(0, 50, img::range_symbol, 24, 8);
    this->_number_w9(28, 50, _data->max_sp(), 5);
    this->_text(75, 54, 1, "\\");
    this->_number_w9(83, 50, _data->min_sp());
}

/*
    SPヒストグラムの表示
*/
void ViewBase::_show_hist()
{
    std::uint8_t h_begin = _data->header()->hist_begin / Statistics::HIST_LENGTH;
    std::uint8_t h_end = _data->header()->hist_end / Statistics::HIST_LENGTH;
    if (_data->max_count() == 0)
    {
        h_begin = h_end = 0;
    }

    std::uint8_t sp1 = (h_begin * Statistics::HIST_LENGTH * Statistics::HIST_BIN_WIDTH + Statistics::HIST_MIN_SP) / 1000;
    std::uint8_t sp5 = ((h_end+1) * Statistics::HIST_LENGTH * Statistics::HIST_BIN_WIDTH + Statistics::HIST_MIN_SP) / 1000;
    std::uint8_t sp3 = (sp1 + sp5) >> 1;
    std::uint8_t sp2 = (sp1 + sp3) >> 1;
    std::uint8_t sp4 = (sp3 + sp5) >> 1;

    // 軸ラベル（数値が一桁か二桁かで開始位置を場合分け）
    this->_number(0, 57, 1, sp1);
    this->_number(sp2 >= 10 ?  25 :  28, 57, 1, sp2);
    this->_number(sp3 >= 10 ?  55 :  58, 57, 1, sp3);
    this->_number(sp4 >= 10 ?  85 :  88, 57, 1, sp4);
    this->_number(sp5 >= 10 ? 110 : 116, 57, 1, sp5);
    this->_text(  sp1 >= 10 ?  12 :   6, 57, 1, "k");
    this->_text(  sp2 >= 10 ?  37 :  34, 57, 1, "k");
    this->_text(  sp3 >= 10 ?  67 :  64, 57, 1, "k");
    this->_text(  sp4 >= 10 ?  97 :  94, 57, 1, "k");
    this->_text(122, 57, 1, "k");

    // 横軸
    _driver->drawLine(  4, 55,   4, 55, _color);
    _driver->drawLine( 34, 55,  34, 55, _color);
    _driver->drawLine( 64, 55,  64, 55, _color);
    _driver->drawLine( 94, 55,  94, 55, _color);
    _driver->drawLine(124, 55, 124, 55, _color);
    _driver->drawLine(  0, 54, 128, 54, _color);

    // ヒストグラム
    if (_data->max_count() > 0)
    {
        std::uint8_t x = 4;
        std::uint8_t w = 6 / (h_end - h_begin + 1);
        for (std::uint8_t h = h_begin; h <= h_end; ++h)
        {
            auto* hist = _data->hist(h);
            for (std::uint8_t i = 0; i < Statistics::HIST_LENGTH; ++i)
            {
                if (auto v = hist->at(i))
                {
                    auto h = static_cast<std::uint8_t>(
                        std::ceil(static_cast<double>(v) / _data->max_count() * 42)
                    );
                    _driver->fillRect(x, 54-h, w, h, _color);
                }
                x += w;
            }
        }
    }
}

void ViewBase::_show_params()
{
    // バージョン情報
    char buf[16];
    auto n = std::sprintf(buf, "%u.%u.%u",
                          MAJOR_VERSION, MINOR_VERSION, REVISION);
    this->_text( 0, 12, 1, " VER");
    this->_text(30, 12, 1, buf);

//-----------------------------------------------------------------------------
#if ATLAS_FORMAT == ATLAS_LITE_SP_ONLY  // SP計測器として使う場合
//-----------------------------------------------------------------------------
    this->_text(33 + 6 * n, 12, 1, "L");
//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------

    // SP表示の設定
    this->_text( 0, 21, 1, "  SP");
    this->_text(30, 21, 1, _params->is_bbp_sp_main() ? "Bey Battle Pass" : "Estimated");

    // 電動ランチャー1の設定
    this->_text( 0, 30, 1, "ELR1");
    if constexpr (ATLAS_FORMAT == ATLAS_FULL_SPEC)
    {
        this->_text(30, 30, 1, _params->automode_elr_index() == 0 ? "A" : "-");
        this->_text(36, 30, 1, _params->elr1().enabled_manual() ? "M" : "-");
        this->_text(48, 30, 1, _params->elr1().is_right() ? "R" : "L");
        this->_number(60, 30, 1, _params->elr1().sp());
        this->_number(96, 30, 1, MOTOR1_MAX_RPM);
    }
    else
    {
        this->_text(30, 30, 1, "-- - ----- -----");
    }

    // 電動ランチャー2の設定
    this->_text(0, 38, 1, "ELR2");
    if constexpr (ATLAS_FORMAT == ATLAS_FULL_SPEC && NUM_MOTORS == 2)
    {
        this->_text(30, 38, 1, _params->automode_elr_index() == 1 ? "A" : "-");
        this->_text(36, 38, 1, _params->elr2().enabled_manual() ? "M" : "-");
        this->_text(48, 38, 1, _params->elr2().is_right() ? "R" : "L");
        this->_number(60, 38, 1, _params->elr2().sp());
        this->_number(96, 38, 1, MOTOR2_MAX_RPM);
    }
    else
    {
        this->_text(30, 38, 1, "-- - ----- -----");
    }

    // オートモードにおける発射待機時間
    this->_text(0, 47, 1, " LTN");
    if constexpr (ATLAS_FORMAT == ATLAS_FULL_SPEC)
    {
        this->_number(30, 47, 1, _params->latency());
    }
    else
    {
        this->_text(30, 47, 1, "-");
    }

    // オートモードにおける発射遅延時間
    this->_text(0, 56, 1, " DEL");
    if constexpr (ATLAS_FORMAT == ATLAS_FULL_SPEC)
    {
        this->_number(30, 56, 1, _params->delay());
    }
    else
    {
        this->_text(30, 56, 1, "-");
    }
}

void ViewBase::_show_client_info()
{
    this->_text(0, 20, 1, "WEB CLIENT APP");
    _driver->drawLine(0, 32, 128, 32, _color);
    this->_text(0, 37, 1, "https://shark-ministe");
    this->_text(0, 46, 1, "r.github.io/atlas_bey");
}

void ViewBase::_show_document_info()
{
    this->_text(0, 20, 1, "SOURCE CODE / MANUAL");
    _driver->drawLine(0, 32, 128, 32, _color);
    this->_text(0, 37, 1, "https://github.com/sh");
    this->_text(0, 46, 1, "ark-minister/atlas_be");
    this->_text(0, 55, 1, "y");
}

//=============================================================================
// マニュアルモード
//=============================================================================
void ViewBase::manual_mode_standby()
{
    //-----------------------------------------------
    Lock lock(_smp);                // ロック
    this->_clear_display();         // 画面のクリア
    _driver->setTextColor(_color);  // フォントカラー
    this->_manual_mode_header();    // ヘッダの表示
    //-----------------------------------------------

//-----------------------------------------------------------------------------
#if SWITCH_TYPE == SW_TACT
//-----------------------------------------------------------------------------
    switch (_state->page_m())
    {
    case 0:
        this->_show_params();
        this->_show_page_info("PARAMS");
        break;
    case 1:
        this->_show_stats();
        this->_show_page_info("STATS");
        break;
    case 2:
        this->_show_hist();
        this->_show_page_info("HIST");
        break;
    case 3:
        this->_show_client_info();
        this->_show_page_info("CLIENT");
        break;
    case 4:
        this->_show_document_info();
        this->_show_page_info("MANUAL");
        break;
    }
//-----------------------------------------------------------------------------
#else
//-----------------------------------------------------------------------------
    this->_manual_mode_old();
//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------

    //-----------------------------------------------
    this->_show_display();          // 描画
    //-----------------------------------------------
}

void ViewBase::_manual_mode_header()
{
    // モードアイコン
    this->_image(0, 0, img::mode_M_small, 14, 8);

    // セントラル（クライアント）接続状態
    if (_state->client_ready())
    {
        this->_image(20, 0, img::client_icon, 10, 8);
    }

//-----------------------------------------------------------------------------
#if ATLAS_FORMAT == ATLAS_FULL_SPEC  // 電動ランチャー制御として使う場合
//-----------------------------------------------------------------------------
    // 電動ランチャー1の状態
    if (_params->elr1().enabled_manual())
    {
        this->_image(34, 0, img::elr1_icon, 18, 8);
    }

    // 電動ランチャー2の状態
    if (_params->elr2().enabled_manual())
    {
        this->_image(54, 0, img::elr2_icon, 18, 8);
    }
//-----------------------------------------------------------------------------
#endif  // #if ATLAS_FORMAT == ATLAS_FULL_SPEC
//-----------------------------------------------------------------------------
}

//=============================================================================
// オートモード
//=============================================================================

struct CountDownImage
{
    constexpr CountDownImage(std::int16_t x_,
                             std::int16_t y_,
                             const std::uint8_t* image_,
                             std::int16_t w_,
                             std::int16_t h_)
        : x(x_)
        , y(y_)
        , image(image_)
        , w(w_)
        , h(h_)
    {
    }
    std::int16_t x;
    std::int16_t y;
    const std::uint8_t* image;
    std::int16_t w;
    std::int16_t h;
};

constexpr CountDownImage countdown_images[6] = {
    CountDownImage(36, 23, atlas::img::cnd_readyset, 57, 29),
    CountDownImage(55, 26, atlas::img::digit_w18_3, 18, 24),
    CountDownImage(55, 26, atlas::img::digit_w18_2, 18, 24),
    CountDownImage(55, 26, atlas::img::digit_w18_1, 18, 24),
    CountDownImage(39, 26, atlas::img::cnd_go, 50, 21),
    CountDownImage(16, 26, atlas::img::cnd_shoot, 97, 21)
};

void ViewBase::auto_mode_promotion()
{
    //-----------------------------------------------
    Lock lock(_smp);                // ロック
    this->_clear_display();         // 画面のクリア
    _driver->setTextColor(_color);  // フォントカラー
    this->_auto_mode_header();      // ヘッダの表示
    //-----------------------------------------------

    /*
        START YOUR PASS AND
        HOLD DOWN THE BUTTON
        ベイバトルパスを起動して
        ボタンを長押しして下さい
    */
    this->_text(4, 16, 1, "START YOUR PASS AND");
    this->_text(4, 25, 1, "HOLD DOWN THE BUTTON");
    this->_image(4, 36, img::promotion_BBP, 119, 22);

    //-----------------------------------------------
    this->_show_display();          // 描画
    //-----------------------------------------------
}

void ViewBase::auto_mode_standby()
{
    //-----------------------------------------------
    Lock lock(_smp);                // ロック
    this->_clear_display();         // 画面のクリア
    _driver->setTextColor(_color);  // フォントカラー
    this->_auto_mode_header();      // ヘッダの表示
    //-----------------------------------------------

//-----------------------------------------------------------------------------
#if SWITCH_TYPE == SW_TACT
//-----------------------------------------------------------------------------
    switch (_state->page_a())
    {
    case 0:
        this->_show_stats();
        this->_show_page_info("STATS");
        break;
    case 1:
        this->_show_hist();
        this->_show_page_info("HIST");
        break;
    case 2:
        this->_show_params();
        this->_show_page_info("PARAMS");
        break;
    }
//-----------------------------------------------------------------------------
#else
//-----------------------------------------------------------------------------
    this->_auto_mode_old();
//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------

    //-----------------------------------------------
    this->_show_display();  // 描画
    //-----------------------------------------------
}

void ViewBase::auto_mode_error()
{
    //-----------------------------------------------
    Lock lock(_smp);                // ロック
    this->_clear_display();         // 画面のクリア
    _driver->setTextColor(_color);  // フォントカラー
    this->_auto_mode_header();      // ヘッダの表示
    //-----------------------------------------------

    // エラーの表示
    this->_image(15, 26, img::crc_error, 98, 26);

    //-----------------------------------------------
    this->_show_display();          // 描画
    //-----------------------------------------------
}

void ViewBase::auto_mode_aborted()
{
    //-----------------------------------------------
    Lock lock(_smp);                // ロック
    this->_clear_display();         // 画面のクリア
    _driver->setTextColor(_color);  // フォントカラー
    this->_auto_mode_header();      // ヘッダの表示
    //-----------------------------------------------

    // キャンセルの表示
    this->_image(18, 26, img::elr_canceled, 92, 26);

    //-----------------------------------------------
    this->_show_display();          // 描画
    //-----------------------------------------------
}

void ViewBase::auto_mode_sp(std::uint16_t bbp_sp,
                            std::uint16_t true_sp,
                            std::uint16_t max_sp)
{
    //-----------------------------------------------
    Lock lock(_smp);                // ロック
    this->_clear_display();         // 画面のクリア
    _driver->setTextColor(_color);  // フォントカラー
    this->_auto_mode_header();      // ヘッダの表示
    //-----------------------------------------------

    // シュート数
    char buf[6];
    std::int16_t n = std::sprintf(buf, "%u", _data->total());
    auto x = 128 - 6 * n;
    this->_text(x-9, 12, 1, "#");
    this->_text(x, 12, 1, buf);   

    // 囲い
    _driver->drawRect(0, 20, 128, 33, _color);

    // 見出し
    this->_text(6, 35, 1, "SP");

    std::uint16_t main_sp, sub_sp;
    if (_params->is_bbp_sp_main())
    {
        main_sp = bbp_sp;
        sub_sp = true_sp;

        if (true_sp < bbp_sp)
        {
            this->_text(24, 35, 1, "?");
        }
        this->_text(6, 26, 1, "YOUR");
        this->_text(0, 56, 1, "EST");
    }
    else
    {
        main_sp = true_sp;
        sub_sp = bbp_sp;
        this->_text(6, 26, 1, "EST.");
        this->_text(0, 56, 1, "BBP");
    }
    this->_number_w18(35, 26, main_sp, 5);
    this->_number(24, 56, 1, sub_sp);

    // 最大SP
    this->_text(64, 56, 1, "MAX");
    this->_number(88, 56, 1, max_sp);

    //-----------------------------------------------
    this->_show_display();          // 描画
    //-----------------------------------------------
}

void ViewBase::auto_mode_countdown(int i)
{
    //-----------------------------------------------
    Lock lock(_smp);                // ロック
    this->_clear_display();         // 画面のクリア
    _driver->setTextColor(_color);  // フォントカラー
    this->_auto_mode_header();      // ヘッダの表示
    //-----------------------------------------------

    this->_image(
        countdown_images[i].x,
        countdown_images[i].y,
        countdown_images[i].image,
        countdown_images[i].w,
        countdown_images[i].h
    );

    //-----------------------------------------------
    this->_show_display();          // 描画
    //-----------------------------------------------
}

void ViewBase::_auto_mode_header()
{
    // モードアイコン
    this->_image(0, 0, img::mode_A_small, 14, 8);

    // バトルパス接続状態
    if (_state->bbp_ready())
    {
        this->_image(20, 0, img::bbp_icon, 10, 8);
    }

    // ベイの装着状態
    if (_state->bey_ready())
    {
        this->_image(34, 0, img::bey_icon, 10, 8);
    }

    // 電動ランチャーの状態
    if (_state->elr_enabled())
    {
        this->_image(
            48, 0,
            _params->automode_elr_index() == 0 ? img::elr1_icon : img::elr2_icon,
            18, 8
        );
    }
}

//=============================================================================
// タクトスイッチ以外の描画
//=============================================================================

void ViewBase::_manual_mode_old()
{
    // SP表示の設定
    this->_text( 92, 0, 1, "SP=");
    this->_text(110, 0, 1, _params->is_bbp_sp_main() ? "BBP" : "Est");

//-----------------------------------------------------------------------------
#if ATLAS_FORMAT == ATLAS_LITE_SP_ONLY  // SP計測器として使う場合
//-----------------------------------------------------------------------------
    // クライアントアプリのURL表示
    this->_text(22, 14, 1, "WEB CLIENT APP");
    this->_text( 0, 24, 1, "https://shark-ministe");
    this->_text( 0, 33, 1, "r.github.io/atlas_bey");
//-----------------------------------------------------------------------------
#elif ATLAS_FORMAT == ATLAS_FULL_SPEC  // 電動ランチャー制御として使う場合
//-----------------------------------------------------------------------------
    // ラベル
    this->_text( 0, 14, 1, "ELR1");
    this->_text( 0, 23, 1, "ELR2");
    this->_text( 0, 33, 1, "LTN");
    this->_text(66, 33, 1, "DEL");

    // 電動ランチャー1の設定
    this->_text(30, 14, 1, _params->automode_elr_index() == 0 ? "A" : "-");
    this->_text(36, 14, 1, _params->elr1().enabled_manual() ? "M" : "-");
    this->_text(48, 14, 1, _params->elr1().is_right() ? "R" : "L");
    this->_number(60, 14, 1, _params->elr1().sp());
    this->_number(96, 14, 1, MOTOR1_MAX_RPM);

    // 電動ランチャー2の設定
    if constexpr (NUM_MOTORS == 2)
    {
        this->_text(30, 23, 1, _params->automode_elr_index() == 1 ? "A" : "-");
        this->_text(36, 23, 1, _params->elr2().enabled_manual() ? "M" : "-");
        this->_text(48, 23, 1, _params->elr2().is_right() ? "R" : "L");
        this->_number(60, 23, 1, _params->elr2().sp());
        this->_number(96, 23, 1, MOTOR2_MAX_RPM);
    }
    else
    {
        this->_text(30, 23, 1, "-- - ----- -----");
    }

    // オートモードにおける発射待機時間
    this->_number(24, 33, 1, _params->latency());

    // オートモードにおける発射遅延時間
    this->_number(90, 33, 1, _params->delay());
//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------

    _driver->drawLine(0, 43, 128, 43, _color);
    
    // 平均・標準偏差
    std::uint8_t x = 0;
    this->_image(x, 47, img::mu_symbol, 6, 8);
    x += 8;
    this->_text(x, 47, 1, "=");
    x += 8;
    this->_number(x, 47, 1, _data->avg_sp());
    x += (_data->avg_sp() < 10000 ? 24 : 30) + 2;
    this->_image(x, 47, img::pm_symbol, 6, 8);
    x += 8;
    this->_number(x, 47, 1, _data->std_sp());

    // 最大・最小
    x = 0;
    this->_image(x, 56, img::max_symbol, 6, 8);
    x += 9;
    this->_number(x, 56, 1, _data->max_sp());
    x += (_data->max_sp() < 10000 ? 24 : 30) + 6;
    this->_image(x, 56, img::min_symbol, 6, 8);
    x += 9;
    this->_number(x, 56, 1, _data->min_sp());

    // 最新SP
    this->_text(90, 56, 1, "#");
    this->_number(99, 56, 1, _data->total());
}

void ViewBase::_auto_mode_old()
{
    // シュート数
    std::uint8_t x = 128;
    std::uint16_t v = _data->total();
    while (v > 0)
    {
        x -= 6;
        v /= 10;
    }
    this->_number(x, 0, 1, _data->total());
    this->_text(x-9, 0, 1, "#");

    // 最新SP
    this->_text(0, 11, 1, "P");
    this->_number(_data->latest_sp() < 10000 ? 16 : 10, 11, 1, _data->latest_sp());

    // 罫線
    _driver->drawLine(0, 21, 40, 21, _color);

    // 平均SP
    this->_image(0, 25, img::mu_symbol, 6, 8);
    this->_number(_data->avg_sp() < 10000 ? 16 : 10, 25, 1, _data->avg_sp());

    // 標準偏差
    this->_image(0, 34, img::sigma_symbol, 6, 8);
    x = 40;
    v = _data->std_sp();
    while (v > 0)
    {
        x -= 6;
        v /= 10;
    }
    this->_number(x, 34, 1, _data->std_sp());

    // 罫線
    _driver->drawLine(0, 44, 40, 44, _color);

    // 最大
    this->_image(0, 48, img::max_symbol, 6, 8);
    this->_number(_data->max_sp() < 10000 ? 16 : 10, 48, 1, _data->max_sp());

    // 最小
    this->_image(0, 57, img::min_symbol, 6, 8);
    this->_number(_data->min_sp() < 10000 ? 16 : 10, 57, 1, _data->min_sp());

    // 軸ラベル
    this->_text(46, 11, 1, "15k");
    this->_text(46, 34, 1, "10k");
    this->_text(46, 57, 1, " 5k");

    // 軸
    _driver->drawLine(64, 13, 65, 13, _color);
    _driver->drawLine(64, 38, 65, 38, _color);
    _driver->drawLine(64, 63, 65, 63, _color);
    _driver->drawLine(66, 13, 66, 63, _color);

    // ヒストグラム
    if (_data->max_count() > 0)
    {
        std::uint8_t y = 63;
        auto draw_hist = [&](const Statistics::Histogram8* hist,
                             std::uint8_t begin,
                             std::uint8_t end)
        {
            for (std::uint8_t i = begin; i < end; ++i)
            {
                if (auto v = hist->at(i))
                {
                    auto w = static_cast<std::uint8_t>(
                        std::ceil(static_cast<double>(v) / _data->max_count() * 60)
                    ) + 68;
                    _driver->drawLine(68, y, w, y, _color);
                }
                y -= 1;
            }
        };

        // ヒストグラムの描画
        draw_hist(_data->hist(0), 5, Statistics::HIST_LENGTH);
        draw_hist(_data->hist(1), 0, Statistics::HIST_LENGTH);
        draw_hist(_data->hist(2), 0, Statistics::HIST_LENGTH-5);
    }
}

//=============================================================================
} // namespace atlas

/*
    ATLAS - AuTomatic LAuncher System
    Copyright (C) 2025  Shark Minister
    All right reserved.

    https://x.com/shark_minister
*/
#include "view.hh"

// C++標準ライブラリ
#include <cstdio>   // std::sprintf

// ATLAS
#include "images.hh"

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

//=============================================================================
// スプラッシュスクリーン
//=============================================================================
void ViewBase::splash_screen()
{
    // セマフォの取得
    xSemaphoreTake(_smp, portMAX_DELAY);

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
#if SP_MEAS_ONLY > 0
    this->_text(122, 8, 1, "L");
#endif

    // 表示
    this->_show_display();

    // セマフォの開放
    xSemaphoreGive(_smp);
}

void ViewBase::image(std::int16_t x,
                     std::int16_t y,
                     const std::uint8_t* bitmap,
                     std::int16_t w,
                     std::int16_t h)
{
    // セマフォの取得
    xSemaphoreTake(_smp, portMAX_DELAY);

    // 画面のクリア
    this->_clear_display();
    
    // 画像表示
    this->_image(x, y, bitmap, w, h);

    // 表示
    this->_show_display();

    // セマフォの開放
    xSemaphoreGive(_smp);
}

//=============================================================================
// マニュアルモード
//=============================================================================
void ViewBase::manual_mode()
{
    // セマフォの取得
    xSemaphoreTake(_smp, portMAX_DELAY);

    // 画面のクリア
    this->_clear_display();

    // フォントカラー
    _driver->setTextColor(_color);

    // モードアイコン
    this->_image(0, 0, img::mode_M, 14, 15);

    // セントラル（クライアント）接続状態
    this->_text(20, 0, 1, "CLI");
    if (_state->client_ready())
    {
        this->_text(26, 8, 1, "*");
    }

    // SP表示
    this->_text(44, 0, 1, "SP-VIEW");
    this->_text(56, 8, 1, _params->is_bbp_sp_main() ? "BBP" : "EST");

//-----------------------------------------------------------------------------
#if SP_MEAS_ONLY == 0  // 電動ランチャー制御として使う場合
//-----------------------------------------------------------------------------
    // ラベル
    this->_text(0, 19, 1, "ELR1");
    this->_text(0, 27, 1, "ELR2");
    this->_text(0, 35, 1, "LTN");
    this->_text(66, 35, 1, "DEL");

    // 電動ランチャー1の設定
    this->_text(30, 19, 1, _params->automode_elr_index() == 0 ? "A" : "-");
    this->_text(36, 19, 1, _params->elr1().enabled_manual() ? "M" : "-");
    this->_text(48, 19, 1, _params->elr1().is_right() ? "R" : "L");
    this->_number(60, 19, 1, _params->elr1().sp());
    this->_number(96, 19, 1, MOTOR1_MAX_RPM);

    // 電動ランチャー2の設定
    if constexpr (NUM_MOTORS == 2)
    {
        this->_text(30, 27, 1, _params->automode_elr_index() == 1 ? "A" : "-");
        this->_text(36, 27, 1, _params->elr2().enabled_manual() ? "M" : "-");
        this->_text(48, 27, 1, _params->elr2().is_right() ? "R" : "L");
        this->_number(60, 27, 1, _params->elr2().sp());
        this->_number(96, 27, 1, MOTOR2_MAX_RPM);
    }
    else
    {
        this->_text(30, 27, 1, "-- - ----- -----");
    }

    // オートモードにおける発射待機時間
    this->_number(24, 35, 1, _params->latency());

    // オートモードにおける発射遅延時間
    this->_number(90, 35, 1, _params->delay());
//-----------------------------------------------------------------------------
#else  // #if SP_MEAS_ONLY == 0
//-----------------------------------------------------------------------------
    this->_text(22, 19, 1, "WEB CLIENT APP");
    this->_text(0, 27, 1, "https://shark-ministe");
    this->_text(0, 35, 1, "r.github.io/atlas_bey");
//-----------------------------------------------------------------------------
#endif  // #if SP_MEAS_ONLY == 0
//-----------------------------------------------------------------------------

    _driver->drawLine(0, 45, 128, 45, _color);

    // 最大・最小
    this->_text(0, 48, 1, "MAX");
    this->_text(0, 56, 1, "MIN");
    this->_number(24, 48, 1, _data->max_sp());
    this->_number(24, 56, 1, _data->min_sp());

    // 平均・標準偏差
    this->_text(66, 48, 1, "AVG");
    this->_text(66, 56, 1, "STD");
    this->_number(90, 48, 1, _data->avg_sp());
    this->_number(90, 56, 1, _data->std_sp());

    // 表示
    this->_show_display();

    // セマフォの開放
    xSemaphoreGive(_smp);
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
    CountDownImage(36, 27, atlas::img::cnd_readyset, 57, 29),
    CountDownImage(55, 30, atlas::img::digit_3, 18, 24),
    CountDownImage(55, 30, atlas::img::digit_2, 18, 24),
    CountDownImage(55, 30, atlas::img::digit_1, 18, 24),
    CountDownImage(39, 30, atlas::img::cnd_go, 50, 21),
    CountDownImage(16, 30, atlas::img::cnd_shoot, 97, 21)
};

void ViewBase::auto_mode_promotion()
{
    // セマフォの取得
    xSemaphoreTake(_smp, portMAX_DELAY);

    // 画面のクリア
    this->_clear_display();

    // フォントカラー
    _driver->setTextColor(_color);

    // ヘッダの表示
    this->_auto_mode_header();

    /*
        START YOUR PASS AND
        HOLD DOWN THE BUTTON
        ベイバトルパスを起動して
        ボタンを長押しして下さい
    */
    this->_text(4, 22, 1, "START YOUR PASS AND");
    this->_text(4, 30, 1, "HOLD DOWN THE BUTTON");
    this->_image(4, 42, img::promotion_BBP, 119, 22);

    // 表示
    this->_show_display();

    // セマフォの開放
    xSemaphoreGive(_smp);
}

void ViewBase::auto_mode_plain()
{
    // セマフォの取得
    xSemaphoreTake(_smp, portMAX_DELAY);

    // 画面のクリア
    this->_clear_display();
    
    // フォントカラー
    _driver->setTextColor(_color);

    // ヘッダの表示
    this->_auto_mode_header();

    // 表示
    this->_show_display();

    // セマフォの開放
    xSemaphoreGive(_smp);
}

void ViewBase::auto_mode_error()
{
    // セマフォの取得
    xSemaphoreTake(_smp, portMAX_DELAY);

    // 画面のクリア
    this->_clear_display();
    
    // フォントカラー
    _driver->setTextColor(_color);

    // ヘッダの表示
    this->_auto_mode_header();

    // エラーの表示
    this->_text(34, 32, 2, "ERROR");

    // 表示
    this->_show_display();

    // セマフォの開放
    xSemaphoreGive(_smp);
}

void ViewBase::auto_mode_aborted()
{
    // セマフォの取得
    xSemaphoreTake(_smp, portMAX_DELAY);

    // 画面のクリア
    this->_clear_display();
    
    // フォントカラー
    _driver->setTextColor(_color);

    // ヘッダの表示
    this->_auto_mode_header();

    // エラーの表示
    this->_text(16, 32, 2, "CANCELED");

    // 表示
    this->_show_display();

    // セマフォの開放
    xSemaphoreGive(_smp);
}

void ViewBase::auto_mode_sp(std::uint16_t bbp_sp,
                            std::uint16_t true_sp,
                            std::uint16_t max_sp)
{
    // セマフォの取得
    xSemaphoreTake(_smp, portMAX_DELAY);

    // 画面のクリア
    this->_clear_display();
    
    // フォントカラー
    _driver->setTextColor(_color);

    // ヘッダの表示
    this->_auto_mode_header();

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
            this->_text(24, 35, 2, "?");
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

    char buf[6];
    auto n = std::sprintf(buf, "%u", main_sp);
    for (int i = n-1; i >= 0; i -= 1)
    {
        this->_image(
            35+18*(i+5-n),
            26,
            img::digit_list[static_cast<int>(buf[i])-0x30],
            18,
            24
        );
    }
    this->_number(24, 56, 1, sub_sp);

    // 最大SP
    this->_text(64, 56, 1, "MAX");
    this->_number(88, 56, 1, max_sp);

    // 表示
    this->_show_display();

    // セマフォの開放
    xSemaphoreGive(_smp);
}

void ViewBase::auto_mode_countdown(int i)
{
    // セマフォの取得
    xSemaphoreTake(_smp, portMAX_DELAY);

    // 画面のクリア
    this->_clear_display();

    // フォントカラー
    _driver->setTextColor(_color);

    // ヘッダの表示
    this->_auto_mode_header();

    this->_image(
        countdown_images[i].x,
        countdown_images[i].y,
        countdown_images[i].image,
        countdown_images[i].w,
        countdown_images[i].h
    );

    // 表示
    this->_show_display();

    // セマフォの開放
    xSemaphoreGive(_smp);
}

void ViewBase::_auto_mode_header()
{
    // モードアイコン
    this->_image(0, 0, img::mode_A, 14, 15);

    // バトルパス接続状態
    this->_text(20, 0, 1, _state->bbp_ready() ? "P" : "-");

    // ベイの装着状態
    this->_text(20, 8, 1, _state->bey_ready() ? "B" : "-");

    // 電動ランチャーの状態
    if (_state->elr_enabled())
    {
        this->_number(27, 0, 1, _params->automode_elr_index()+1);
        this->_text(27, 8, 1, _params->automode_elr().is_right() ? "R" : "L");
    }
    else
    {
        this->_text(27, 0, 1, "-");
        this->_text(27, 8, 1, "-");
    }

    // 最大
    this->_text(40, 0, 1, "max");
    this->_number(62, 0, 1, _data->max_sp());

    // シュート数
    this->_text(96, 0, 1, "n");
    this->_number(104, 0, 1, _data->total());

    // 平均・標準偏差
    this->_text(40, 8, 1, "avg");
    this->_number(62, 8, 1, _data->avg_sp());
    this->_image(96, 8, img::plusminus, 6, 8);
    this->_number(104, 8, 1, _data->std_sp());
}

//=============================================================================
} // namespace atlas

/*
    ATLAS - AuTomatic LAuncher System
    Copyright (C) 2025  Shark Minister
    All right reserved.

    https://x.com/shark_minister
*/
#ifndef ATLAS_VIEW_HH
#define ATLAS_VIEW_HH

// C++標準ライブラリ
#include <cstdint>  // std::uint16_t

// Arduino
#include <Arduino.h>
#include <Adafruit_GFX.h>

// ATLAS
#include "params.hh"
#include "state.hh"
#include "stats.hh"
#include "setting.hh"

#if DISPLAY_DRIVER == ADAFRUIT_SH1106G
#include <Adafruit_SH110X.h>
typedef  Adafruit_SH1106G  DisplayDriver;
#elif DISPLAY_DRIVER == ADAFRUIT_SSD1306
#include <Adafruit_SSD1306.h>
typedef  Adafruit_SSD1306  DisplayDriver;
#endif

namespace atlas
{
//-----------------------------------------------------------------------------

//! 画面構成用ベースクラス
class ViewBase
{
public:
    //! デフォルトコンストラクタ（禁止）
    ViewBase() = delete;

    /*!
        @brief  コンストラクタ
        @param[in]   driver  ディスプレイドライバ
        @param[in]   data    データ
        @param[in]   params  制御パラメータ
        @param[in]   state   稼働状況
    */
    ViewBase(Adafruit_GFX& driver,
             const Statistics& data,
             const Params& params,
             const State& state);

    //! デストラクタ
    virtual ~ViewBase() = default;

    //! スプラッシュスクリーンを表示する
    void splash_screen();

    /*!
        @brief  単一の画像を表示する
        @param[in]  x       左上のX座標
        @param[in]  y       左上のY座標
        @param[in]  bitmap  ビットマップ画像データ配列
        @param[in]  w       画像の幅
        @param[in]  h       画像の高さ
    */
    void image(std::int16_t x,
               std::int16_t y,
               const std::uint8_t* bitmap,
               std::int16_t w,
               std::int16_t h);

    //! マニュアルモードの画面を表示する
    void manual_mode();

    //! オートモードで、ヘッダのみを表示する
    void auto_mode_plain();

    //! オートモードで、エラーを表示する
    void auto_mode_error();

    //! オートモードで、射出キャンセルを表示する
    void auto_mode_aborted();

    //! オートモードで、BBPの接続を促すメッセージを表示する
    void auto_mode_promotion();
    
    /*!
        @brief  オートモードで、シュート情報を表示する
        @param[in]   bbp_sp    BBPに記録されたSP
        @param[in]   true_sp   真のSP
        @param[in]   max_sp    プロファイル錠の最大SP
    */
    void auto_mode_sp(std::uint16_t bbp_sp,
                      std::uint16_t true_sp,
                      std::uint16_t max_sp);
    
    //! オートモードで、カウントダウンを表示する
    void auto_mode_countdown(int i);

    /*! 
        @brief  前景色を設定する
        @param[in]   color   前景色
    */
    inline void set_fgcolor(std::uint16_t color)
    {
        _color = color;
    }

protected:
    //! Adafruitドライバ
    Adafruit_GFX* _driver = nullptr;

    //! 統計データ
    const Statistics* _data;

    //! パラメータ
    const Params* _params;
    
    //! 状態
    const State* _state;
    
    //! 前景の色
    std::uint16_t _color = 1;
    
    //! セマフォ
    SemaphoreHandle_t _smp;

    //! バッファをクリアする
    virtual void _clear_display() = 0;
    
    //! 描画を終了し、画面に反映させる
    virtual void _show_display() = 0;

    /*!
        @brief  画像を描画する
        @param[in]  x       左上のX座標
        @param[in]  y       左上のY座標
        @param[in]  bitmap  ビットマップ画像データ配列
        @param[in]  w       画像の幅
        @param[in]  h       画像の高さ
    */
    virtual void _image(std::int16_t x,
                        std::int16_t y,
                        const std::uint8_t* bitmap,
                        std::int16_t w,
                        std::int16_t h) = 0;

private:
    /*!
        @brief  テキストを描画する

        @param[in]  x       左上のX座標
        @param[in]  y       左上のY座標
        @param[in]  size    文字サイズ
        @param[in]  text    テキスト本文
    */
    void _text(std::int16_t x,
               std::int16_t y,
               std::uint8_t size,
               const char* text);

    /*!
        @brief  数値を描画する

        @param[in]  x       左上のX座標
        @param[in]  y       左上のY座標
        @param[in]  size    文字サイズ
        @param[in]  text    数値
    */
    void _number(std::int16_t x,
                 std::int16_t y,
                 std::uint8_t size,
                 std::int32_t number);

    //! オートモードのヘッダ情報を表示する
    void _auto_mode_header();
};

/*!
    画面構成クラス
    ディスプレイドライバをテンプレートにとる
    
    e.g.) View<Adafruit_SSD1306>, View<Adafruit_SH1106G>
*/
template <typename DisplayDriver>
class View
    : public ViewBase
{
public:
    /*!
        @brief  コンストラクタ
        @param[in]   driver  ディスプレイドライバ
        @param[in]   data    データ
        @param[in]   params  制御パラメータ
        @param[in]   state   稼働状況
    */
    View(DisplayDriver& driver,
         const Statistics& data,
         const Params& params,
         const State& state)
        : ViewBase(driver, data, params, state)
    {
    }

    //! デストラクタ
    virtual ~View()
    {
    }

    //! ディスプレイドライバの登録
    inline void set_driver(DisplayDriver* driver) noexcept
    {
        _driver = driver;
    }
    
    //! ディスプレイドライバの取得
    inline DisplayDriver* driver() noexcept
    {
        return static_cast<DisplayDriver*>(_driver);
    }

    //! ディスプレイドライバの取得
    inline const DisplayDriver* driver() const noexcept
    {
        return static_cast<const DisplayDriver*>(_driver);
    }

private:
    //! バッファをクリアする
    void _clear_display() override
    {
        this->driver()->clearDisplay();
    }

    //! 描画を終了し、画面に反映させる
    void _show_display() override
    {
        this->driver()->display();
    }

    /*!
        @brief  画像を描画する

        @param[in]  x       左上のX座標
        @param[in]  y       左上のY座標
        @param[in]  bitmap  ビットマップ画像データ配列
        @param[in]  w       画像の幅
        @param[in]  h       画像の高さ
    */
    void _image(std::int16_t x,
                std::int16_t y,
                const std::uint8_t* bitmap,
                std::int16_t w,
                std::int16_t h) override
    {
        this->driver()->drawBitmap(x, y, bitmap, w, h, _color);
    }
};

#if DISPLAY_DRIVER == ADAFRUIT_SH1106G
inline bool begin_display(DisplayDriver& driver)
{
    return driver.begin(SCREEN_ADDR);
}
#elif DISPLAY_DRIVER == ADAFRUIT_SSD1306
inline bool begin_display(DisplayDriver& driver)
{
    return driver.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDR);
}
#endif

//-----------------------------------------------------------------------------
} // namespace atlas
#endif

/*
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
*/
#ifndef SHARK_MINISTER_MONOCHROME_DISPLAY_HH
#define SHARK_MINISTER_MONOCHROME_DISPLAY_HH

// C++標準ライブラリ
#include <cstdint>
#include <type_traits>

// Arduino
#include <Adafruit_SSD1306.h>
#include <Adafruit_SH110X.h>

// shark
#include "image_number.hh"

namespace shark {
//-----------------------------------------------------------------------------

//! モノクロディスプレイ
template <typename T>
class MonochromeDisplay
{
public:
    //! コンストラクタ
    MonochromeDisplay(std::uint8_t width,
                      std::uint8_t height);

    //! カンバス初期化
    bool begin(std::uint8_t screen_addr);

    //! バッファをクリアする
    inline void clear() {
        _driver.clearDisplay();
    }

    //! 描画を終了し、画面に反映させる
    inline void show() {
        _driver.display();
    }

    inline void setTextColor(std::uint16_t color) {
        _textColor = color;
    }

    inline void setImageColor(std::uint16_t color) {
        _imageColor = color;
    }

    inline void applyTextColor() {
        _driver.setTextColor(_textColor);
    }

    /*!
        @brief  テキストを描画する

        @param[in]  x       左上のX座標
        @param[in]  y       左上のY座標
        @param[in]  size    文字サイズ
        @param[in]  text    テキスト本文
    */
    void text(std::int16_t x,
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
    void number(std::int16_t x,
                std::int16_t y,
                std::uint8_t size,
                std::int32_t number);

    inline void numberW9(
        std::int16_t x,
        std::int16_t y,
        std::uint32_t number,
        std::int16_t numDigits = 0
    ) {
        this->_numberImg(x, y, number, numDigits, 9, 14, digitsW9);
    }

    inline void numberW18(
        std::int16_t x,
        std::int16_t y,
        std::uint32_t number,
        std::int16_t numDigits = 0
    ) {
        this->_numberImg(x, y, number, numDigits, 18, 24, digitsW18);
    }

    /*!
        @brief  単一の画像を表示する
        @param[in]  x       左上のX座標
        @param[in]  y       左上のY座標
        @param[in]  bitmap  ビットマップ画像データ配列
        @param[in]  w       画像の幅
        @param[in]  h       画像の高さ
    */
    inline void image(
        std::int16_t x,
        std::int16_t y,
        const std::uint8_t* bitmap,
        std::int16_t w,
        std::int16_t h
    ) {
        _driver.drawBitmap(x, y, bitmap, w, h, _imageColor);
    }
    
    inline void line(
        std::int16_t x0,
        std::int16_t y0,
        std::int16_t x1,
        std::int16_t y1
    ) {
        _driver.drawLine(x0, y0, x1, y1, _imageColor);
    }

    inline void rect(
        std::int16_t x,
        std::int16_t y,
        std::int16_t w,
        std::int16_t h
    ) {
        _driver.drawRect(x, y, w, h, _imageColor);
    }

    inline void fillRect(
        std::int16_t x,
        std::int16_t y,
        std::int16_t w,
        std::int16_t h
    ) {
        _driver.fillRect(x, y, w, h, _imageColor);
    }

private:
    //! Adafruitドライバ
    T _driver;

    //! 前景色
    std::uint16_t _textColor = 1;
    std::uint16_t _imageColor = 1;

    void _numberImg(
        std::int16_t x,
        std::int16_t y,
        std::uint32_t number,
        std::int16_t numDigits,
        std::int16_t w,
        std::int16_t h,
        const std::uint8_t** images
    );
};

template <typename T>
MonochromeDisplay<T>::MonochromeDisplay(
    std::uint8_t width,
    std::uint8_t height
) : _driver(width, height)
{
}

//! カンバス初期化
template <typename T>
bool MonochromeDisplay<T>::begin(std::uint8_t screen_addr)
{
#if DISPLAY_DRIVER == ADAFRUIT_SSD1306
    return _driver.begin(SSD1306_SWITCHCAPVCC, screen_addr);
#elif DISPLAY_DRIVER == ADAFRUIT_SH1106G
    return _driver.begin(screen_addr);
#else
    return false;
#endif
}

template <typename T>
void MonochromeDisplay<T>::text(
    std::int16_t x,
    std::int16_t y,
    std::uint8_t size,
    const char* text
) {
    _driver.setTextSize(size);
    _driver.setCursor(x, y);
    _driver.print(text);
}

template <typename T>
void MonochromeDisplay<T>::number(
    std::int16_t x,
    std::int16_t y,
    std::uint8_t size,
    std::int32_t number
) {
    _driver.setTextSize(size);
    _driver.setCursor(x, y);
    _driver.print(number);
}

template <typename T>
void MonochromeDisplay<T>::_numberImg(
    std::int16_t x,
    std::int16_t y,
    std::uint32_t number,
    std::int16_t numDigits,
    std::int16_t w,
    std::int16_t h,
    const std::uint8_t** images
) {
    // 桁数の取得、文字列化
    char buf[21];
    std::int16_t n = std::sprintf(buf, "%u", number);

    // 表示
    if (numDigits > 0) {
        for (std::int16_t i = n-1; i >= 0; i -= 1) {
            this->image(
                x + w * (i + numDigits - n), y,
                images[static_cast<int>(buf[i])-0x30],
                w, h
            );
        }
    }
    else {
        for (std::uint32_t i = 0; i < n; i += 1) {
            this->image(
                x+w*i, y,
                images[static_cast<int>(buf[i])-0x30],
                w, h
            );
        }
    }
}

//-----------------------------------------------------------------------------
} // namespace shark
#endif

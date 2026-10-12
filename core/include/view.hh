/*
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
*/
#ifndef ATLAS_VIEW_HH
#define ATLAS_VIEW_HH

// C++標準ライブラリ
#include <cstdint>  // std::uint16_t

// 設定
#include "setting.hh"

// shark lib
#include "mutex.hh"
#include "monochrome_display.hh"
#if DISPLAY_DRIVER == ADAFRUIT_SSD1306
#include <Adafruit_SSD1306.h>
typedef shark::MonochromeDisplay<Adafruit_SSD1306> DisplayDriver;
#elif DISPLAY_DRIVER == ADAFRUIT_SH1106G
#include <Adafruit_SH110X.h>
typedef shark::MonochromeDisplay<Adafruit_SH1106G> DisplayDriver;
#endif

namespace atlas {
//-----------------------------------------------------------------------------

class View
    : public DisplayDriver
{
public:
    View();

    //! スプラッシュスクリーンを表示する
    void splashScreen();

    //! マニュアルモードの画面を表示する
    void manualModeStandby();

    //! オートモードの待機画面を表示する
    void autoModeStandby();

    //! オートモードで、エラーを表示する
    void autoModeError();

    //! オートモードで、射出キャンセルを表示する
    void autoModeAborted();

    //! オートモードで、BBPの接続を促すメッセージを表示する
    void autoModePromotion();
    
    /*!
        @brief  オートモードで、シュート情報を表示する
        @param[in]   origSP   BBPに記録されたオリジナルSP値
        @param[in]   evalSP   プロファイル解析から評価されたSP値
    */
    void autoModeSP(
        std::uint16_t origSP,
        std::uint16_t evalSP
    );
    
    //! オートモードで、カウントダウンを表示する
    void autoModeCountdown(int i);

    //! ユーザーイメージ
    inline void userImage(
        std::int16_t x,
        std::int16_t y,
        const std::uint8_t* bitmap,
        std::int16_t w,
        std::int16_t h
    ) {
        this->image(x, y, bitmap, w, h);
    }

private:
    //! マニュアル/設定モードのヘッダ情報を表示する
    void _manualModeHeader();

    //! オートモードのヘッダ情報を表示する
    void _autoModeHeader();

    //! マニュアル/設定モードのクライアント情報を表示する
    void _showClientInfo();
    void _showDocumentInfo();
    void _showPageInfo(const char* page_header);
    void _showStats();
    void _showHist();
    void _showParams();
    
private:
    //! 排他制御」
    shark::Mutex _mutex;
};

//-----------------------------------------------------------------------------
} // namespace atlas
#endif

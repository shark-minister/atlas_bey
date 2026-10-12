/*
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
*/
#include "view.hh"

// C++標準ライブラリ
#include <cstdio>  // std::snprintf
#include <cstring> // std::strlen

// Atlas lib
#include "lock.hh"
#include "image_number.hh"

// Atlas
#include "atlas_manager.hh"
#include "images.hh"
#include "setting.hh"

namespace atlas {
//-----------------------------------------------------------------------------

View::View()
    : DisplayDriver(SCREEN_WIDTH, SCREEN_HEIGHT)
{
}

// スプラッシュスクリーン
void View::splashScreen()
{
    // ロック
    shark::Lock lock(_mutex);

    // 画面のクリア
    this->clear();
    
    // さめ大臣のロゴ表示
    this->image(26, 8, img::sharkMinisterLogo, 76, 52);

    // バージョン表記
    this->applyTextColor();
    char buf[16];
    auto w = 6 * std::snprintf(
        buf,
        16,
        "%u.%u.%u",
        MAJOR_VERSION, MINOR_VERSION, REVISION
    );
    this->text(128-w, 0, 1, buf);

    // 表示
    this->show();
}

//=============================================================================
// 共通
//=============================================================================

void View::_showPageInfo(const char* page_header)
{
    // ページ名
    this->text(128 - std::strlen(page_header)*6, 0, 1, page_header);
}

/*
    統計情報の表示
    ・トータルシュート回数
    ・最大、最小SP値
    ・平均SP値
    ・標準偏差
    ・最新SP値
*/
void View::_showStats()
{
    // 表示する統計情報の取得
    const auto& stats = ATLAS.statistics();

    // 最新SP
    this->text(0, 16, 1, "LAST");
    this->numberW9(28, 16, stats.latestSP, 5);
    // シュート数
    this->text(83, 22, 1, "#");
    this->numberW9(91, 16, stats.total);

    // 平均SP
    this->text(0, 33, 1, "MEAN");
    this->numberW9(28, 33, stats.meanSP, 5);
    // 標準偏差
    this->image(75, 36, img::pmSymbol, 6, 8);
    this->numberW9(83, 33, stats.stdevSP);

    // 最大／最小
    this->image(0, 50, img::rangeSymbol, 24, 8);
    this->numberW9(28, 50, stats.maxSP, 5);
    this->text(75, 54, 1, "\\");
    this->numberW9(83, 50, stats.minSP);
}

/*
    SPヒストグラムの表示
*/
void View::_showHist()
{
    // 表示するヒストグラムの取得
    const auto& hist = ATLAS.statistics().hist;
    std::int16_t i0 = hist.minIndex;
    std::int16_t i1 = hist.maxIndex;
    if (hist.maxCount == 0 || i0 > i1) {
        i0 = i1 = 0;
    }

    const std::int16_t L1000 = i0 * HIST_BIN_WIDTH + HIST_MIN_SP;
    const std::int16_t L = L1000 / 1000;
    const std::int16_t R = 1 + (
        (i1 * HIST_BIN_WIDTH + HIST_MIN_SP) / 1000
    );
    const std::int16_t diff = R - L;
    const std::int16_t step = (diff - 1) / 4 + 1;
    std::int16_t m;
    if (diff < 3) {
        m = diff;
    }
    else if (diff == 3 || diff == 5 || diff == 6 || diff == 9) {
        m = 3;
    }
    else if (diff > 12) {
        m = 6;
    }
    else {
        m = 4;
    }
    const std::int16_t d = 120 / m;

    // 軸描画
    this->line(0, 54, 127, 54);
    std::int16_t xTicks = 4;
    char buf[4];
    for (std::int16_t i = 0; i <= m; ++i) {
        // ticks
        this->line(xTicks, 55, xTicks, 55);

        // ticks label
        const std::int16_t len = std::snprintf(buf, 4, "%d", L + i * step);
        std::int16_t xLabel;
        // 先頭
        if (i == 0) {
            xLabel = (len == 1) ? 2 : 0;
        }
        // 末端
        else if (i == m) {
            xLabel = (len == 1) ? 122 : (128 - len * 6);
        }
        // それ以外
        else {
            xLabel = xTicks - len * 3 + 1;
        }
        // テキスト
        this->text(xLabel, 57, 1, buf);

        // next
        xTicks += d;
    }

    // ヒストグラム
    if (hist.maxCount > 0) {
        std::int16_t w = d / (5 * step);
        std::int16_t x = 4 + w * (
            (L1000 - L * 1000) / HIST_BIN_WIDTH
        );
        for (std::int16_t i = i0; i <= i1; ++i) {
            if (auto v = hist.at(i)) {
                auto h = static_cast<std::uint8_t>(
                    std::ceil(
                        static_cast<double>(v) / hist.maxCount * 42
                    )
                );
                this->fillRect(x, 54-h, w, h);
            }
            x += w;
        }
    }
}

void View::_showParams()
{
    // バージョン情報
    char buf[16];
    auto w = 6 * std::snprintf(
        buf,
        16,
        "%u.%u.%u",
        MAJOR_VERSION, MINOR_VERSION, REVISION
    );
    this->text( 0, 12, 1, " VER");
    this->text(30, 12, 1, buf);

//-----------------------------------------------------------------------------
#if ATLAS_FORMAT == ATLAS_LITE_SP_ONLY  // SP計測器として使う場合
//-----------------------------------------------------------------------------
    this->text(33 + w, 12, 1, "L");
//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------

    // SP表示の設定
    this->text( 0, 21, 1, "  SP");
    this->text(30, 21, 1,
        ATLAS.params.mainSPView() == MainSPView::EVAL_SP
            ? "Prof. Evaluated"
            : "Battle Pass Rec."
    );

    // 電動ランチャー1の設定
    this->text( 0, 30, 1, "ELR1");
    if constexpr (ATLAS_FORMAT == ATLAS_FULL_SPEC) {
        this->text(30, 30, 1, ATLAS.params.autoModeELRIndex() == 0 ? "A" : "-");
        this->text(36, 30, 1, ATLAS.params.elr1().enabledManual() ? "M" : "-");
        this->text(48, 30, 1, ATLAS.params.elr1().isRight() ? "R" : "L");
        this->number(60, 30, 1, ATLAS.params.elr1().sp());
        this->number(96, 30, 1, MOTOR1_MAX_RPM);
    }
    else {
        this->text(30, 30, 1, "-- - ----- -----");
    }

    // 電動ランチャー2の設定
    this->text(0, 38, 1, "ELR2");
    if constexpr (ATLAS_FORMAT == ATLAS_FULL_SPEC && NUM_MOTORS == 2) {
        this->text(30, 38, 1, ATLAS.params.autoModeELRIndex() == 1 ? "A" : "-");
        this->text(36, 38, 1, ATLAS.params.elr2().enabledManual() ? "M" : "-");
        this->text(48, 38, 1, ATLAS.params.elr2().isRight() ? "R" : "L");
        this->number(60, 38, 1, ATLAS.params.elr2().sp());
        this->number(96, 38, 1, MOTOR2_MAX_RPM);
    }
    else {
        this->text(30, 38, 1, "-- - ----- -----");
    }

    // オートモードにおける発射待機時間
    this->text(0, 47, 1, " LTN");
    if constexpr (ATLAS_FORMAT == ATLAS_FULL_SPEC) {
        this->number(30, 47, 1, ATLAS.params.latency());
    }
    else {
        this->text(30, 47, 1, "-");
    }

    // オートモードにおける発射遅延時間
    this->text(0, 56, 1, " DEL");
    if constexpr (ATLAS_FORMAT == ATLAS_FULL_SPEC) {
        this->number(30, 56, 1, ATLAS.params.delay());
    }
    else {
        this->text(30, 56, 1, "-");
    }
}

void View::_showClientInfo()
{
    this->text(0, 20, 1, "WEB CLIENT APP");
    this->line(0, 32, 128, 32);
    this->text(0, 37, 1, "https://shark-ministe");
    this->text(0, 46, 1, "r.github.io/atlas2");
}

void View::_showDocumentInfo()
{
    this->text(0, 20, 1, "SOURCE CODE / MANUAL");
    this->line(0, 32, 128, 32);
    this->text(0, 37, 1, "https://github.com/sh");
    this->text(0, 46, 1, "ark-minister/atlas2");
}

//=============================================================================
// マニュアルモード
//=============================================================================
void View::manualModeStandby()
{
    shark::Lock lock(_mutex);   // ロック
    this->clear();              // 画面のクリア
    this->applyTextColor();     // フォントカラー
    this->_manualModeHeader();  // ヘッダの表示

    switch (ATLAS.state.pageM()) {
    case 0:
        this->_showParams();
        this->_showPageInfo("PARAMS");
        break;
    case 1:
        this->_showStats();
        this->_showPageInfo("STATS");
        break;
    case 2:
        this->_showHist();
        this->_showPageInfo("x1000");
        break;
    case 3:
        this->_showClientInfo();
        this->_showPageInfo("CLIENT");
        break;
    case 4:
        this->_showDocumentInfo();
        this->_showPageInfo("MANUAL");
        break;
    }

    // 描画
    this->show();
}

void View::_manualModeHeader()
{
    // モードアイコン
    this->image(0, 0, img::modeM, 14, 8);

    // セントラル（クライアント）接続状態
    if (ATLAS.state.isClientReady()) {
        this->image(20, 0, img::clientIcon, 10, 8);
    }

//-----------------------------------------------------------------------------
#if ATLAS_FORMAT == ATLAS_FULL_SPEC  // 電動ランチャー制御として使う場合
//-----------------------------------------------------------------------------
    // 電動ランチャー1の状態
    if (ATLAS.params.elr1().enabledManual()) {
        this->image(34, 0, img::elr1Icon, 18, 8);
    }

    // 電動ランチャー2の状態
    if (ATLAS.params.elr2().enabledManual()) {
        this->image(54, 0, img::elr2Icon, 18, 8);
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
    constexpr CountDownImage(
        std::int16_t x_,
        std::int16_t y_,
        const std::uint8_t* image_,
        std::int16_t w_,
        std::int16_t h_
    )
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
    CountDownImage(36, 23, atlas::img::cndReadyset, 57, 29),
    CountDownImage(55, 26, shark::digitW18_3, 18, 24),
    CountDownImage(55, 26, shark::digitW18_2, 18, 24),
    CountDownImage(55, 26, shark::digitW18_1, 18, 24),
    CountDownImage(39, 26, atlas::img::cndGo, 50, 21),
    CountDownImage(16, 26, atlas::img::cndShoot, 97, 21)
};

void View::autoModePromotion()
{
    shark::Lock lock(_mutex);  // ロック
    this->clear();             // 画面のクリア
    this->applyTextColor();    // フォントカラー
    this->_autoModeHeader();   // ヘッダの表示

    /*
        START YOUR PASS AND
        HOLD DOWN THE BUTTON
        ベイバトルパスを起動して
        ボタンを長押しして下さい
    */
    this->text(4, 16, 1, "START YOUR PASS AND");
    this->text(4, 25, 1, "HOLD DOWN THE BUTTON");
    this->image(4, 36, img::promotion_BBP, 119, 22);

    // 描画
    this->show();
}

void View::autoModeStandby()
{
    shark::Lock lock(_mutex);  // ロック
    this->clear();             // 画面のクリア
    this->applyTextColor();    // フォントカラー
    this->_autoModeHeader();   // ヘッダの表示

    switch (ATLAS.state.pageA()) {
    case 0:
        this->_showStats();
        this->_showPageInfo("STATS");
        break;
    case 1:
        this->_showHist();
        this->_showPageInfo("HIST");
        break;
    case 2:
        this->_showParams();
        this->_showPageInfo("PARAMS");
        break;
    }

    // 描画
    this->show();
}

void View::autoModeError()
{
    shark::Lock lock(_mutex);  // ロック
    this->clear();             // 画面のクリア
    this->applyTextColor();    // フォントカラー
    this->_autoModeHeader();   // ヘッダの表示

    // エラーの表示
    this->image(15, 26, img::crcError, 98, 26);

    // 描画
    this->show();
}

void View::autoModeAborted()
{
    shark::Lock lock(_mutex);   // ロック
    this->clear();              // 画面のクリア
    this->applyTextColor();     // フォントカラー
    this->_autoModeHeader();    // ヘッダの表示

    // キャンセルの表示
    this->image(18, 26, img::elrCanceled, 92, 26);

    // 描画
    this->show();
}

void View::autoModeSP(
    std::uint16_t acc1,
    std::uint16_t acc2
)
{
    shark::Lock lock(_mutex);  // ロック
    this->clear();             // 画面のクリア
    this->applyTextColor();    // フォントカラー
    this->_autoModeHeader();   // ヘッダの表示

    const std::uint16_t evalSP = ATLAS.result.statsEval.latestSP;
    const std::uint16_t origSP = ATLAS.result.statsOrig.latestSP;

    // 文字列変換用バッファ
    char buf[16];
    std::int16_t w = 0;

    // シュート数
    w = std::snprintf(buf, 16, "%u", ATLAS.statistics().total) * 6;
    this->text(128-w-9, 56, 1, "#");
    this->text(128-w, 56, 1, buf);

    // 囲い
    this->rect(0, 20, 128, 33);

    // SP
    this->text(6, 35, 1, "SP");
    if (ATLAS.params.mainSPView() == MainSPView::EVAL_SP) {
        // 主SP（プロファイル評価SP）
        this->text(6, 26, 1, "EVAL");
        this->numberW18(35, 26, evalSP, 5);

        // 副SP（BBP記録SP）
        w = 6 * std::snprintf(buf, 16, "BBP: %u", origSP);
    }
    else {
        if (evalSP < origSP) {
            this->text(21, 35, 1, "?");
        }

        // 主SP（BBP記録SP）
        this->text(6, 26, 1, "BBP");
        this->numberW18(35, 26, origSP, 5);

        // 副SP（プロファイル評価SP）
        w = 6 * std::snprintf(buf, 16, "EVAL: %u", evalSP);
    }
    // 副SP
    this->text(128-w, 12, 1, buf);

    // 加速度
    std::uint16_t offset = 0;
    this->text(offset, 56, 1, "ACC:");
    offset += 27;
    // 前半の加速度
    if (acc1 >= 300) {
        this->text(offset, 56, 1, "-");
        offset += 9;
    }
    else {
        w = 6 * std::snprintf(buf, 16, "%u", acc1);
        this->text(offset, 56, 1, buf);
        offset += w;
    }
    // 区切り文字
    this->text(offset, 56, 1, ",");
    offset += 9;
    // 後半の加速度
    if (acc2 >= 300) {
        this->text(offset, 56, 1, "-");
    }
    else {
        this->number(offset, 56, 1, acc2);
    }

    // 描画
    this->show();
}

void View::autoModeCountdown(int i)
{
    shark::Lock lock(_mutex);  // ロック
    this->clear();             // 画面のクリア
    this->applyTextColor();    // フォントカラー
    this->_autoModeHeader();   // ヘッダの表示

    this->image(
        countdown_images[i].x,
        countdown_images[i].y,
        countdown_images[i].image,
        countdown_images[i].w,
        countdown_images[i].h
    );

    // 描画
    this->show();
}

void View::_autoModeHeader()
{
    // モードアイコン
    this->image(0, 0, img::modeA, 14, 8);

    // バトルパス接続状態
    if (ATLAS.state.isBBPReady()) {
        this->image(20, 0, img::bbpIcon, 10, 8);
    }

    // ベイの装着状態
    if (ATLAS.state.isBeyReady()) {
        this->image(34, 0, img::beyIcon, 10, 8);
    }

    // 電動ランチャーの状態
    if (ATLAS.state.isELREnabled()) {
        this->image(
            48, 0,
            ATLAS.params.autoModeELRIndex() == 0 ? img::elr1Icon : img::elr2Icon,
            18, 8
        );
    }
}

//-----------------------------------------------------------------------------
} // namespace atlas

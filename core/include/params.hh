/*
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
*/
#ifndef ATLAS_PARAMS_HH
#define ATLAS_PARAMS_HH

// C++標準ライブラリ
#include <cstdint>      // std::uint8_t, std::uint16_t
#include <type_traits>  // std::is_trivially_copyable_v

namespace atlas {
//-----------------------------------------------------------------------------

//! 電動ランチャーの設定値を管理するクラス
class ELRParams
{
public:
    //! マニュアルモードで使用する電動ランチャーかどうか
    bool enabledManual() const noexcept;

    //! 電動ランチャーが右回転かどうかを返す
    bool isRight() const noexcept;

    //! 電動ランチャーの設定SPを返す
    std::uint32_t sp() const noexcept;

    //! 値を適正化する
    void regulate() noexcept;

    //! 値を初期化する
    void initialize() noexcept;

private:
    //! フラグ用ビットフィールド
    struct Flags
    {
        /*! 
            @brief  マニュアルモードで利用されるか
            - `0`: 無効
            - `1`: 有効
        */
        std::uint8_t enabledManual : 1;

        //! 予約領域
        std::uint8_t reserved1 : 3;

        /*! 
            @brief  電動ランチャーの回転方向
            - `0`: 右回転
            - `1`: 左回転
        */
        std::uint8_t rotation : 1;

        //! 予約領域
        std::uint8_t reserved2 : 3;
    } _flags;

    //! シュートパワー (_sp * 100)
    std::uint8_t _sp;
};

static_assert(
    sizeof(ELRParams) == 2,
    "Size of 'ELRParams' is not 2 bytes"
);

static_assert(
    std::is_trivially_copyable_v<ELRParams>,
    "'ELRParams' is not trivially copyable"
);

static_assert(
    std::is_trivially_default_constructible_v<ELRParams>,
    "'ELRParams' is not trivially default constructable"
);

//-----------------------------------------------------------------------------

enum class MainSPView
    : std::uint8_t
{
    EVAL_SP = 0,
    ORIG_SP = 1
};
              
//! 制御パラメータを管理するクラス
class Params
{
public:
    //! オートモードで使用する電動ランチャーのインデックス番号を返す
    std::uint8_t autoModeELRIndex() const noexcept;

    //! オートモードで使用する電動ランチャーのインスタンスを返す
    const ELRParams& autoModeELR() const noexcept;

    //! 指定インデックス番号の電動ランチャーのインスタンスを返す
    const ELRParams& elr(std::uint32_t index) const noexcept;

    //! オートモードの射出遅延時間 [ms]を返す
    std::uint16_t delay() const noexcept;

    //! オートモードの猶予時間 [ms]を返す
    std::uint16_t latency() const noexcept;

    //! 真のSP値をメインに表示するかどうかを返す
    MainSPView mainSPView() const noexcept;

    //! 値を適正化する
    void regulate() noexcept;

    //! 値を初期化する
    void initialize() noexcept;

    //! 電動ランチャー1のインスタンスを返す
    inline const ELRParams& elr1() const noexcept {
        return _elr1;
    }

    //! 電動ランチャー2のインスタンスを返す
    inline const ELRParams& elr2() const noexcept {
        return _elr2;
    }

private:
    //! フラグ用ビットフィールド
    struct Flags
    {
        /*! 
            @brief  オートモードで利用する電動ランチャー
            - `0`: ELR 1
            - `1`: ELR 2
        */
        std::uint8_t elrAutoMode : 1;

        /*!
            @brief  メイン表示のSP値
            - `0`: プロファイル解析で評価されたSP値がメイン
            - `1`: BBP記録のオリジナルSP値がメイン
            - `2`: 予約領域
            - `3`: 予約領域
        */
        std::uint8_t mainSP : 2;

        //! 予約領域
        std::uint16_t reserved : 13;
    } _flags;
    
    //! オートモードの猶予時間 (_latency * 10) [ms]
    std::uint8_t _latency;
    
    //! オートモードの射出遅延時間 (_delay * 2) [ms]
    std::uint8_t _delay;

    //! 電動ランチャー1の設定
    ELRParams _elr1;
    
    //! 電動ランチャー2の設定
    ELRParams _elr2;
};

static_assert(
    sizeof(Params) == 8,
    "Size of 'Params' is not 8 bytes"
);

static_assert(
    std::is_trivially_copyable_v<Params>,
    "'Params' is not trivially copyable"
);

static_assert(
    std::is_trivially_default_constructible_v<Params>,
    "'Params' is not trivially default constructable"
);

//-----------------------------------------------------------------------------
} // namespace atlas
#endif

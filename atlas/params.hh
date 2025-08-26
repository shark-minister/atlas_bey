/*
    ATLAS - AuTomatic LAuncher System
    Copyright (C) 2025  Shark Minister
    All right reserved.

    https://x.com/shark_minister
*/
#ifndef ATLAS_PARAMS_HH
#define ATLAS_PARAMS_HH

// C++標準ライブラリ
#include <cstdint>      // std::uint8_t
#include <type_traits>  // std::is_trivially_copyable_v

// ATLAS
#include "setting.hh"

namespace atlas
{
//-----------------------------------------------------------------------------

//! 電動ランチャーの設定値を管理するクラス
class ElectricLauncher
{
public:
    //! マニュアルモードで使用する電動ランチャーかどうか
    inline bool enabled_manual() const noexcept
    {
        return _flags.field.enabled_manual == 1;
    }

    //! 電動ランチャーが右回転かどうかを返す
    inline bool is_right() const noexcept
    {
        return _flags.field.rotation == 0;
    }

    //! 電動ランチャーの設定SPを返す
    inline std::uint32_t sp() const noexcept
    {
        return static_cast<std::uint32_t>(_sp) * 100;
    }

    //! 値を適正化する
    void regulate() noexcept
    {
        // SPのソフトウェアリミットチェック
        if (this->sp() > LAUNCHER_SP_UPPER_LIMIT)
        {
            _sp = LAUNCHER_SP_UPPER_LIMIT / 100;
        }
        else if (this->sp() < LAUNCHER_SP_LOWER_LIMIT)
        {
            _sp = LAUNCHER_SP_LOWER_LIMIT / 100;
        }
#if NUM_MOTORS == 1
        _flags.field.enabled_manual = 1;
#endif
    }

    //! 値を初期化する
    void init() noexcept
    {
        _sp = DEFAULT_LAUNCHER_SP / 100;
        _flags.data = 0;
    }

private:
    union Flags
    {
    public:
        //! 内部データ
        std::uint8_t data;

        //! フラグ用ビットフィールド
        struct BitField
        {
            /*! 
                @brief  マニュアルモードで利用されるか
                - `0`: 無効
                - `1`: 有効
            */
            std::uint8_t enabled_manual : 1;

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
        } field;
    };
    
    //! フラグ群
    Flags _flags;

    //! シュートパワー (_sp * 100)
    std::uint8_t _sp;
};

static_assert(sizeof(ElectricLauncher) == 2,
              "Size of 'ElectricLauncher' is not 2 bytes");

static_assert(std::is_trivially_copyable_v<ElectricLauncher>,
              "'ElectricLauncher' is not trivially copyable");

static_assert(std::is_trivially_default_constructible_v<ElectricLauncher>,
              "'ElectricLauncher' is not trivially default constructable");

//! 制御パラメータを管理するクラス
class Params
{
public:
    //! オートモードで使用する電動ランチャーのインデックス番号を返す
    inline std::uint8_t automode_elr_index() const noexcept
    {
        return _flags.field.elr_auto_mode;
    }

    //! オートモードで使用する電動ランチャーのインスタンスを返す
    inline const ElectricLauncher& automode_elr() const noexcept
    {
        return _flags.field.elr_auto_mode == 0 ? _elr1 : _elr2;
    }

    //! 指定インデックス番号の電動ランチャーのインスタンスを返す
    inline const ElectricLauncher& elr(std::uint32_t index) const noexcept
    {
        return index == 0 ? _elr1 : _elr2;
    }

    //! 電動ランチャー1のインスタンスを返す
    inline const ElectricLauncher& elr1() const noexcept
    {
        return _elr1;
    }

    //! 電動ランチャー2のインスタンスを返す
    inline const ElectricLauncher& elr2() const noexcept
    {
        return _elr2;
    }

    //! オートモードの射出遅延時間 [ms]を返す
    inline std::uint16_t delay() const noexcept
    {
        return static_cast<std::uint16_t>(_delay) * 2;
    }

    //! オートモードの猶予時間 [ms]を返す
    inline std::uint16_t latency() const noexcept
    {
        return static_cast<std::uint16_t>(_latency) * 10;
    }

    //! 真のSP値をメインに表示するかどうかを返す
    inline bool is_bbp_sp_main() const noexcept
    {
        return _flags.field.bbp_sp_main > 0;
    }

    //! 値を適正化する
    void regulate() noexcept
    {
        _elr1.regulate();    // 電動ランチャー1
#if NUM_MOTORS == 1
        _flags.field.elr_auto_mode = 0;
#elif NUM_MOTORS == 2
        _elr2.regulate();    // 電動ランチャー2
#endif
        // レイテンシ
        if (this->latency() < LATENCY_LOWER_LIMIT)
        {
            _latency = LATENCY_LOWER_LIMIT / 10;
        }
        // ディレイ
        if (this->delay() > DELAY_UPPER_LIMIT)
        {
            _delay = DELAY_UPPER_LIMIT / 2;
        }
    }

    //! 値を初期化する
    void init() noexcept
    {
        _latency = DEFAULT_LATENCY / 10;
        _delay = DEFAULT_DELAY / 2;
        _flags.data = 0;
        _elr1.init();
        _elr2.init();
    }

private:
    union Flags
    {
    public:
        //! 内部データ
        std::uint8_t data;

        /*!
            フラグ用ビットフィールド
            読み出し専用
        */
        struct BitField
        {
            /*! 
                @brief  オートモードで利用する電動ランチャー
                - `0`: ELR 1
                - `1`: ELR 2
            */
            std::uint8_t elr_auto_mode : 1;

            //! 予約領域
            std::uint8_t reserved1 : 2;

            /*!
                @brief  BBP記録値をメインに表示するか
                - `0`: 「真」のSP値をメインに表示する
                - `1`: BBP記録値をメインに表示する
            */
            std::uint8_t bbp_sp_main : 1;

            //! 予約領域
            std::uint8_t reserved2 : 4;
        } field;
    };
    
    //! フラグ群
    Flags _flags;
    
    //! オートモードの猶予時間 (_latency * 10) [ms]
    std::uint8_t _latency;
    
    //! オートモードの射出遅延時間 (_delay * 2) [ms]
    std::uint8_t _delay;
    
    //! 電動ランチャー1の設定
    ElectricLauncher _elr1;
    
    //! 電動ランチャー2の設定
    ElectricLauncher _elr2;
};

static_assert(sizeof(Params) == 7,
              "Size of 'Params' is not 7 bytes");

static_assert(std::is_trivially_copyable_v<Params>,
              "'Params' is not trivially copyable");

static_assert(std::is_trivially_default_constructible_v<Params>,
              "'Params' is not trivially default constructable");

//-----------------------------------------------------------------------------
} // namespace atlas
#endif

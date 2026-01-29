/*
    ATLAS - AuTomatic LAuncher System
    Copyright (C) 2025  Shark Minister
    All right reserved.

    https://x.com/shark_minister
*/
#ifndef ATLAS_STATE_HH
#define ATLAS_STATE_HH

// C++標準ライブラリ
#include <cstdint>      // std::uint8_t

namespace atlas
{
//-----------------------------------------------------------------------------

//! アプリの状態を管理
union State
{
public:
    //! ベイバトルパスが接続されているかどうかを返す（オートモード）
    inline bool bbp_ready() const noexcept
    {
        return _field.bbp_ready == 1;
    }

    //! ベイがランチャーにセットされているかどうかを返す（オートモード）
    inline bool bey_ready() const noexcept
    {
        return _field.bey_ready == 1;
    }

    //! 電動ランチャーが連動するかどうかを返す（オートモード）
    inline bool elr_enabled() const noexcept
    {
        return _field.elr_enabled == 1;
    }

    //! クライアント（PC/スマホ）が接続されているかどうかを返す（マニュアルモード）
    inline bool client_ready() const noexcept
    {
        return _field.client_ready == 1;
    }

    inline std::uint8_t page_a() const noexcept
    {
        return _field.page_a;
    }

    inline std::uint8_t page_m() const noexcept
    {
        return _field.page_m;
    }

    inline void next_page_a() noexcept
    {
        auto p = this->page_a() + 1;
        _field.page_a = (p == MAX_PAGE_A) ? 0 : p;
    }

    inline void next_page_m() noexcept
    {
        auto p = this->page_m() + 1;
        _field.page_m = (p == MAX_PAGE_M) ? 0 : p;
    }

    inline void set_page_a(std::uint8_t page) noexcept
    {
        if (page < MAX_PAGE_A)
        {
            _field.page_a = page;
        }
    }

    inline void set_page_m(std::uint8_t page) noexcept
    {
        if (page < MAX_PAGE_M)
        {
            _field.page_m = page;
        }
    }

    //! ベイバトルパスが接続されているかどうかを設定する（オートモード）
    inline void set_bbp(bool is_ready) noexcept
    {
        _field.bbp_ready = is_ready ? 1 : 0;
    }

    //! ベイがランチャーにセットされているかどうかを設定する（オートモード）
    inline void set_bey(bool is_ready) noexcept
    {
        _field.bey_ready = is_ready ? 1 : 0;
    }
    
    //! 電動ランチャーが連動するかどうかを設定する（オートモード）
    inline void set_elr(bool is_enabled) noexcept
    {
        _field.elr_enabled = is_enabled ? 1 : 0;
    }

    //! クライアント（PC/スマホ）が接続されているかどうかを設定する（マニュアルモード）
    inline void set_client(bool is_ready) noexcept
    {
        _field.client_ready = is_ready ? 1 : 0;
    }

    //! クリア
    inline void clear() noexcept
    {
        this->data = 0;
    }

private:
    //! 内部データ
    std::uint16_t data = 0;

    //! フラグ用ビットフィールド
    struct BitField
    {
        /*! 
            @brief  BBPが接続されているか
            - `0`: 未接続
            - `1`: 接続
        */
        std::uint8_t bbp_ready : 1;

        /*! 
            @brief  ベイが装着されているか
            - `0`: 未装着
            - `1`: 装着
        */
        std::uint8_t bey_ready : 1;

        /*! 
            @brief  電動ランチャーが有効かどうか
            - `0`: 無効
            - `1`: 有効
        */
        std::uint8_t elr_enabled : 1;

        /*! 
            @brief  クライアントが接続されているか
            - `0`: 未接続
            - `1`: 接続
        */
        std::uint8_t client_ready : 1;

        //! オートモードのページ
        std::uint8_t page_a : 3;

        //! マニュアル/設定モードのページ
        std::uint8_t page_m : 3;

        //! 予約領域
        std::uint8_t reserved : 6;
    } _field;
};

//-----------------------------------------------------------------------------
} // namespace atlas
#endif

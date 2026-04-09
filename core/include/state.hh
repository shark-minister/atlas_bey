/*
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
*/
#ifndef ATLAS_STATE_HH
#define ATLAS_STATE_HH

// C++標準ライブラリ
#include <cstdint>      // std::uint8_t

// ATLAS
#include "setting.hh"

namespace atlas {
//-----------------------------------------------------------------------------

//! アプリの状態を管理
class State
{
public:
    //! ベイバトルパスが接続されているかどうかを返す（オートモード）
    inline bool isBBPReady() const noexcept {
        return _isBBPReady == 1;
    }

    //! ベイがランチャーにセットされているかどうかを返す（オートモード）
    inline bool isBeyReady() const noexcept {
        return _isBeyReady == 1;
    }

    //! 電動ランチャーが連動するかどうかを返す（オートモード）
    inline bool isELREnabled() const noexcept {
        return _isELREnabled == 1;
    }

    //! クライアント（PC/スマホ）が接続されているかどうかを返す（マニュアルモード）
    inline bool isClientReady() const noexcept {
        return _isClientReady == 1;
    }

    inline std::uint8_t pageA() const noexcept {
        return _pageA;
    }

    inline std::uint8_t pageM() const noexcept {
        return _pageM;
    }

    //! オートモードで次のページに進む
    void nextPageA() noexcept {
        auto p = _pageA + 1;
        _pageA = (p == MAX_PAGE_A) ? 0 : p;
    }

    //! マニュアルモードで次のページに進む
    void nextPageM() noexcept {
        auto p = _pageM + 1;
        _pageM = (p == MAX_PAGE_M) ? 0 : p;
    }

    //! ベイバトルパスが接続されているかどうかを設定する（オートモード）
    inline void setBBP(bool isReady) noexcept {
        _isBBPReady = isReady;
    }

    //! ベイがランチャーにセットされているかどうかを設定する（オートモード）
    inline void setBey(bool isReady) noexcept {
        _isBeyReady = isReady;
    }
    
    //! 電動ランチャーが連動するかどうかを設定する（オートモード）
    inline void setELR(bool isEnabled) noexcept {
        _isELREnabled = isEnabled;
    }

    //! クライアント（PC/スマホ）が接続されているかどうかを設定する（マニュアルモード）
    inline void setClient(bool isReady) noexcept {
        _isClientReady = isReady;
    }

    //! クリア
    void clear() noexcept {
        _isBBPReady = false;
        _isBeyReady = false;
        _isELREnabled = false;
        _isClientReady = false;
        _pageA = 0;
        _pageM = 0;
    }

private:
    /*! 
        @brief  BBPが接続されているか
        - `0`: 未接続
        - `1`: 接続
    */
    bool _isBBPReady = false;

    /*! 
        @brief  ベイが装着されているか
        - `0`: 未装着
        - `1`: 装着
    */
    bool _isBeyReady = false;

    /*! 
        @brief  電動ランチャーが有効かどうか
        - `0`: 無効
        - `1`: 有効
    */
    bool _isELREnabled = false;

    /*! 
        @brief  クライアントが接続されているか
        - `0`: 未接続
        - `1`: 接続
    */
    bool _isClientReady = false;

    //! オートモードのページ
    std::uint8_t _pageA = 0;

    //! マニュアル/設定モードのページ
    std::uint8_t _pageM = 0;

};

//-----------------------------------------------------------------------------
} // namespace atlas
#endif

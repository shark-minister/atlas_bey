/*
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
*/
#ifndef ATLAS_DEVICE_INFO_HH
#define ATLAS_DEVICE_INFO_HH

// C++標準ライブラリ
#include <cstdint>      // std::uint8_t
#include <type_traits>  // std::is_trivially_copyable_v, std::is_trivially_default_constructible_v

namespace atlas {
//-----------------------------------------------------------------------------

//! デバイスの実装情報を保持する構造体
struct DeviceInfo
{
    //! バージョン情報
    struct Version
    {
        std::uint8_t rev   : 8;  //!< リビジョン
        std::uint8_t minor : 4;  //!< マイナーバージョン
        std::uint8_t major : 4;  //!< メジャーバージョン
    } version;

    //! コンディション
    struct Condition
    {
        /*!
            @brief  利用形態
            - `0`: 電動ランチャー制御器
            - `1`: SP測定器
            - `2`: 予備
            - `3`: 予備
        */
        std::uint16_t format : 2;

        /*!
            @brief  切替スイッチタイプ
            - `0`: スイッチなし
            - `1`: スライドスイッチ
            - `2`: タクトスイッチ
            - `3`-`7`: 予備
        */
        std::uint16_t switchType : 3;

        /*! 
            @brief  制御する電動ランチャースタイル
            - `0`: 1台構成
            - `1`: 2台構成
            - `2`: 予備
            - `3`: 予備
        */
        std::uint16_t elrStyle : 2;

        //! 予約領域
        std::uint16_t reserved : 9;
    } condition;
};

static_assert(sizeof(DeviceInfo::Version) == 2,
              "Size of 'Version' is not 2 bytes");

static_assert(sizeof(DeviceInfo::Condition) == 2,
              "Size of 'Condition' is not 2 bytes");

static_assert(sizeof(DeviceInfo) == 4,
              "Size of 'DeviceInfo' is not 6 bytes");

static_assert(std::is_trivially_copyable_v<DeviceInfo>,
              "'DeviceInfo' is not trivially copyable");

static_assert(std::is_trivially_default_constructible_v<DeviceInfo>,
              "'DeviceInfo' is not trivially default constructable");

//-----------------------------------------------------------------------------
} // namespace atlas
#endif

/*
    ATLAS - AuTomatic LAuncher System
    Copyright (C) 2025  Shark Minister
    All right reserved.

    https://x.com/shark_minister
*/
#ifndef ATLAS_DEVICE_INFO_HH
#define ATLAS_DEVICE_INFO_HH

// C++標準ライブラリ
#include <cstdint>      // std::uint8_t
#include <type_traits>  // std::is_trivially_copyable_v

// ATLAS
#include "setting.hh"

namespace atlas
{
//-----------------------------------------------------------------------------

//! バージョン情報
union Version
{
    std::uint16_t data;
    struct Field
    {
        //! リビジョン
        std::uint8_t rev : 8;

        //! マイナーバージョン
        std::uint8_t minor : 4;

        //! メジャーバージョン
        std::uint8_t major : 4;
    } field;
};

union Condition
{
    std::uint16_t data;
    struct Field
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
            @brief  モード切替スイッチタイプ
            - `0`: スイッチなし
            - `1`: スライドスイッチ
            - `2`: タクトスイッチ
            - `3`-`7`: 予備
        */
        std::uint16_t switch_type : 3;

        /*! 
            @brief  電動ランチャーは2台構成か否か
            - `0`: モータは1台構成
            - `1`: モータは2台構成
            - `2`: 予備
            - `3`: 予備
        */
        std::uint16_t double_elrs : 2;

        //! 予約領域
        std::uint16_t reserved : 9;
    } field;
};

//! デバイスの実装情報を保持する構造体
struct DeviceInfo
{
    Version version;
    Condition condition;
    std::uint8_t motor1_max_rpm;
    std::uint8_t motor2_max_rpm;
};

DeviceInfo get_device_info()
{
    DeviceInfo result;
    result.motor1_max_rpm = MOTOR1_MAX_RPM / 100;
    result.motor2_max_rpm = MOTOR2_MAX_RPM / 100;
    result.version.field.major = MAJOR_VERSION;
    result.version.field.minor = MINOR_VERSION;
    result.version.field.rev = REVISION;
    result.condition.field.format = ATLAS_FORMAT;
    result.condition.field.switch_type = SWITCH_TYPE;
    result.condition.field.double_elrs = NUM_MOTORS - 1;
    return result;
}

static_assert(sizeof(Version) == 2,
              "Size of 'Version' is not 2 bytes");

static_assert(sizeof(Condition) == 2,
              "Size of 'Condition' is not 2 bytes");

static_assert(sizeof(DeviceInfo) == 6,
              "Size of 'DeviceInfo' is not 5 bytes");

static_assert(std::is_trivially_copyable_v<DeviceInfo>,
              "'DeviceInfo' is not trivially copyable");

static_assert(std::is_trivially_default_constructible_v<DeviceInfo>,
              "'DeviceInfo' is not trivially default constructable");

//-----------------------------------------------------------------------------
} // namespace atlas
#endif

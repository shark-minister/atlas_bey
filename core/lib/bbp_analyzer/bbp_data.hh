/*
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
*/
#ifndef SHARK_MINISTER_BBP_DATA_HH
#define SHARK_MINISTER_BBP_DATA_HH

// C++標準ライブラリ
#include <cstdint>  // std::uint8_t, std::uint16_t, std::uint32_t
#include <type_traits>  // std::is_trivially_copyable_v, std::is_trivially_default_constructible_v

namespace shark {
//-----------------------------------------------------------------------------

//! ベイバトルパス（BBP）からの1回分のデータを扱うクラス
class BBPData
{
public:
    //! データ長
    static constexpr int LENGTH = 17;

    //! 内容の初期化
    void clear() noexcept;

    //! ヘッダを返す
    inline std::uint8_t header() const noexcept {
        return _bytes[0];
    }

    //! 内部ポインタを返す
    inline const std::uint8_t* data() const noexcept {
        return _bytes;
    }

    //! 内部ポインタを返す
    inline std::uint8_t* data() noexcept {
        return _bytes;
    }

    /*!
        @brief  指定位置の要素を返す
        @param[in]  index  インデックス
    */
    inline std::uint8_t at(int index) const {
        return _bytes[index];
    }

    /*!
        @brief   指定位置の16ビットデータを16ビット整数値に変換して返す
        @param[in]  offset  先頭からのバイトオフセット値
    */
    std::uint16_t uint16(int offset) const noexcept;

private:
    //! ベイバトルパスからのデータを格納するコンテナ
    std::uint8_t _bytes[LENGTH];
};

static_assert(sizeof(BBPData) == BBPData::LENGTH,
              "Size of 'DeviceInfo' is not 17 bytes");

static_assert(std::is_trivially_copyable_v<BBPData>,
              "'BBPData' is not trivially copyable");

static_assert(std::is_trivially_default_constructible_v<BBPData>,
              "'BBPData' is not trivially default constructable");

//-----------------------------------------------------------------------------
}
#endif

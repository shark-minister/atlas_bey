
/*
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
*/
#ifndef ATLAS_RESULT_HH
#define ATLAS_RESULT_HH

// C++標準ライブラリ
#include <cstdint>      // std::uint8_t
#include <type_traits>  // std::is_trivially_copyable_v

// ATLAS
#include "statistics.hh"

namespace atlas {
//-----------------------------------------------------------------------------

class Result
{
public:
    //! 結果を初期化する
    void initialize() noexcept;

    //! 統計データを初期化する
    void clear() noexcept;

    /*!
        @brief  結果を更新する
        @param[in]  origSP   バトルパスで記録されたオリジナルのSP
        @param[in]  rawProf  プロファイルデータ
    */
    void update(
        std::uint16_t origSP,
        const std::uint16_t* rawProf,
        std::uint16_t& acc1,
        std::uint16_t& acc2
    );

public:
    //! 統計情報
    Statistics statsOrig;
    Statistics statsEval;
};

static_assert(sizeof(Result) == 112 * 2,
              "Size of 'Result' has invalid size");

static_assert(std::is_trivially_default_constructible_v<Result>,
              "'Result' is not trivially default constructable");

static_assert(std::is_trivially_copyable_v<Result>,
              "'Result' is not trivially copyable");

//-----------------------------------------------------------------------------
} // namespace atlas
#endif

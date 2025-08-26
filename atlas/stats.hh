/*
    ATLAS - AuTomatic LAuncher System
    Copyright (C) 2025  Shark Minister
    All right reserved.

    https://x.com/shark_minister
*/
#ifndef ATLAS_STATISTICS_HH
#define ATLAS_STATISTICS_HH

// C++標準ライブラリ
#include <cstdint>      // std::uint8_t
#include <type_traits>  // std::is_trivially_copyable_v

namespace atlas
{
//-----------------------------------------------------------------------------

class Statistics
{
public:
    //! ヒストグラムサイズ
    static constexpr std::uint32_t HIST_LENGTH = 20;

    //! ヒストグラム数
    static constexpr std::uint32_t NUM_HISTS = 3;

    //! ヒストグラムに記録する最小SP
    static constexpr std::uint32_t HIST_MIN_SP = 4000;

    //! ヒストグラムに記録する最大SP
    static constexpr std::uint32_t HIST_MAX_SP = 16000;

    //! ヒストグラムのビン幅
    static constexpr std::uint32_t HIST_BIN_WIDTH = 200;

    //! ヘッダ情報
    struct Header
    {
        // 統計情報
        std::uint16_t total;       //!< 累計シュート数
        std::uint16_t max_sp;      //!< 最大SP
        std::uint16_t min_sp;      //!< 最小SP
        std::uint16_t avg_sp;      //!< 平均SP
        std::uint16_t std_sp;      //!< 標準偏差

        // ヒストグラム情報
        std::uint8_t hist_begin;
        std::uint8_t hist_end;

        void init() noexcept;
    };

    //! ヒストグラム
    struct Histogram
    {
        std::uint8_t data[HIST_LENGTH];
        void init() noexcept;
        inline void increment(std::uint32_t index) noexcept
        {
            data[index] += 1;
        }
    };

    //! 合計シュート回数を返す
    inline std::uint16_t total() const noexcept
    {
        return _header.total;
    }

    //! 最大SP値を返す
    inline std::uint16_t max_sp() const noexcept
    {
        return _header.max_sp;
    }

    //! 最小SP値を返す
    inline std::uint16_t min_sp() const noexcept
    {
        return _header.min_sp;
    }

    //! 平均SP値を返す
    inline std::uint16_t avg_sp() const noexcept
    {
        return _header.avg_sp;
    }

    //! SPの標準偏差を返す
    inline std::uint16_t std_sp() const noexcept
    {
        return _header.std_sp;
    }
    
    //! 最新のSP値を返す
    inline std::uint16_t latest_sp() const noexcept
    {
        return _latest_sp;
    }

    //! ヘッダ情報
    const Header* header() const noexcept
    {
        return &_header;
    }

    /*!
        @brief  ヒストグラム（送信用）を返す
        @param[in]  i  ヒストグラムのインデックス
        @return  ヒストグラムインスタンスへのポインタ
    */
    const Histogram* hist(std::uint32_t i) const noexcept
    {
        return &(_hists[i]);
    }

    //! 統計データを初期化する
    void init() noexcept;

    /*!
        @brief  統計情報を更新する
        @param[in]  sp  シュートパワー
    */
    void update(std::uint16_t sp) noexcept;

private:
    //! ヘッダ情報
    Header _header;

    //! ヒストグラムデータ本体
    Histogram _hists[NUM_HISTS];

    // 最新SP
    std::uint16_t _latest_sp;

    // 計算用の一時変数
    double _avg_sp_tmp;          //!< 平均
    double _std_sp_tmp;          //!< 標準偏差
    double _sum_sp;              //!< SP値の合計
    double _sum_sp2;             //!< SP値の二乗の合計
};

static_assert(sizeof(Statistics::Histogram) == 20,
              "Size of 'Statistics::Histogram' is not 20 bytes");

static_assert(sizeof(Statistics::Header) == 12,
              "Size of 'Statistics::Histogram' is not 12 bytes");

static_assert(std::is_trivially_default_constructible_v<Statistics::Histogram>,
              "'Statistics::Histogram' is not trivially default constructable");

static_assert(std::is_trivially_copyable_v<Statistics>,
              "'Statistics' is not trivially copyable");

static_assert(std::is_trivially_default_constructible_v<Statistics>,
              "'Statistics' is not trivially default constructable");

//-----------------------------------------------------------------------------
} // namespace atlas
#endif

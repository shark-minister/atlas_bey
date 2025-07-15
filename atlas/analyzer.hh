/*
    ATLAS - AuTomatic LAuncher System
    Copyright (C) 2025  Shark Minister
    All right reserved.
*/
#ifndef ATLAS_BBP_ANALYZER_HH
#define ATLAS_BBP_ANALYZER_HH

// C++標準ライブラリ
#include <cstdint>  // std::uint8_t, std::uint16_t, std::uint32_t
#include <cstring>  // std::memset
#include <map>      // std::map

namespace atlas
{
//-----------------------------------------------------------------------------

//! ベイバトルパス（BBP）からの1回分のデータを扱うクラス
class BBPData
{
public:
    //! データ長
    static constexpr int LENGTH = 17;

    //! デフォルトコンストラクタ
    BBPData() noexcept = default;

    //! コピーコンストラクタ
    BBPData(const BBPData& rhs)
    {
        std::memcpy(_bytes, rhs._bytes, LENGTH);
    }

    //! コピー代入演算子
    inline BBPData& operator=(const BBPData& rhs)
    {
        if (this != &rhs)
        {
            std::memcpy(_bytes, rhs._bytes, LENGTH);
        }
        return *this;
    }

    //! 内容の初期化
    inline void init() noexcept
    {
        // 0で埋める
        std::memset(_bytes, 0, LENGTH);
    }

    //! ヘッダを返す
    inline std::uint8_t header() const noexcept
    {
        return _bytes[0];
    }

    //! 内部ポインタを返す
    inline const std::uint8_t* data() const noexcept
    {
        return _bytes;
    }

    //! 内部ポインタを返す
    inline std::uint8_t* data() noexcept
    {
        return _bytes;
    }

    /*!
        @brief  指定位置の要素を返す
        @param[in]  index  インデックス
    */
    inline std::uint8_t at(int index) const
    {
        return _bytes[index];
    }

    /*!
        @brief   指定位置の16ビットデータを16ビット整数値に変換して返す
        @param[in]  offset  先頭からのバイトオフセット値
    */
    inline std::uint16_t uint16(int offset) const noexcept
    {
        return static_cast<std::uint16_t>(_bytes[offset+1]) << 8 |
               static_cast<std::uint16_t>(_bytes[offset]);
    }

private:
    //! ベイバトルパスからのデータを格納するコンテナ
    std::uint8_t _bytes[LENGTH] = {0};
};

//! 解析の状態を表す定数
enum class BBPState
    : std::uint16_t
{
    ERROR           = 0xFFFF,   //!< データのチェックサムエラー
    NONE            = 0x0000,   //!< 変化なし
    BEY_ATTACHED    = 0x0004,   //!< ベイブレードがランチャーにセットされた
    BEY_DETACHED    = 0x0400,   //!< ベイブレードがランチャーから離れた
    ELR_ENABLED     = 0x0010,   //!< 電動ランチャー連動が有効になった
    ELR_DISABLED    = 0x1000,   //!< 電動ランチャー連動が無効になった
    SHOOT_ORDERED   = 0x1014,   //!< 電動ランチャーへのシュート指令が出された
    BEY_DETACHED_2  = 0x1410,   //!< 電動ランチャーへのシュート指令がキャンセルされた
    FINISHED        = 0xF000    //!< 解析が終了した
};

//! ベイがランチャーにセットされているか否かを返す
inline bool is_bey_attached(BBPState state)
{
    return (static_cast<std::uint16_t>(state) & 0x04) > 0;
}

//! ベイバトルパス（BBP）からのデータを解析するクラス
class BBPAnalyzer
{
public:
    //! デフォルトコンストラクタ
    BBPAnalyzer() = default;

    //! コピーコンストラクタ（禁止）
    BBPAnalyzer(const BBPAnalyzer& rhs) = delete;

    //! ムーブコンストラクタ
    BBPAnalyzer(BBPAnalyzer&& rhs) noexcept  = delete;

    //! デストラクタ
    ~BBPAnalyzer() noexcept = default;

    //! コピー代入演算子（禁止）
    BBPAnalyzer& operator=(const BBPAnalyzer& rhs) = delete;

    //! ムーブ代入演算子
    BBPAnalyzer& operator=(BBPAnalyzer&& rhs) noexcept = delete;

    /*!
        @brief  BBPからのデータの解析を行う
        @param[in]  data  17bitのデータ
        @return  解析の状況を返す
    */
    BBPState analyze(const BBPData& data);

    //! バトルパスに記録されたシュートパワー値を返す
    inline std::uint16_t bbp_sp() const noexcept
    {
        return _bbp_sp;
    }

    //! プロファイル中の最大シュートパワー値を返す
    inline std::uint16_t max_sp() const noexcept
    {
        return _max_sp;
    }

    //! 誤計測を排除した「真」のシュートパワー値を返す
    inline std::uint16_t true_sp() const noexcept
    {
        return _true_sp;
    }

    //! プロファイルの解析（誤計測検知）
    void calc_true_sp();

    //! 解析データのクリア
    void clear();

/*
    //! シュートの加速度を返す
    inline std::uint16_t acceleration() const
    {
        return _accel;
    }

    //! シュートの期待SP値を返す
    std::uint16_t exp_sp() const noexcept;
*/

private:
    /*
        ベイバトルパス(BBP)からのnotifyデータのヘッダー一覧。

        - A0 (160): BBPがベイブレードのマウントを検知した
        - B0 (176): SP一覧のうち、1-8番目のSP値
        - B1 (177): SP一覧のうち、9-16番目のSP値
        - B2 (178): SP一覧のうち、17-24番目のSP値
        - B3 (179): SP一覧のうち、25-32番目のSP値
        - B4 (180): SP一覧のうち、33-40番目のSP値
        - B5 (181): SP一覧のうち、41-48番目のSP値
        - B6 (182): SP一覧のうち、49, 50番目のSP値、BBPに保存されているシュート数
        - B7 (183): チェックサム
        - 70 (112): SPプロファイルのうち、チャンネル1-8
        - 71 (113): SPプロファイルのうち、チャンネル9-16
        - 72 (114): SPプロファイルのうち、チャンネル17-24
        - 73 (115): SPプロファイルのうち、チャンネル25-32。また、最終行に相当
    */
    static constexpr std::uint8_t HEADER_ATTACH_DETACH = 0xA0;
    static constexpr std::uint8_t HEADER_LIST_FIRST    = 0xB0;
    static constexpr std::uint8_t HEADER_LIST_LAST     = 0xB6;
    static constexpr std::uint8_t HEADER_CHECKSUM      = 0xB7;
    static constexpr std::uint8_t HEADER_PROF_FIRST    = 0x70;
    static constexpr std::uint8_t HEADER_PROF_LAST     = 0x73;
    static constexpr std::uint8_t HEADER_DATA_END      = 0x73;

    //! ベイバトルパスからのデータ一式を格納するコンテナ
    std::map<std::uint8_t, BBPData> _data_map;

    //! ベイの脱着フラグ（記録）
    std::uint8_t _prev_state_bey = 0;

    //! バトルパスに記録されたシュートパワー値
    std::uint16_t _bbp_sp = 0;

    //! 真のシュートパワー値
    std::uint16_t _true_sp = 0;

    //! 最大のシュートパワー値
    std::uint16_t _max_sp = 0;

    //! シュート加速度
    //double _accel = 0.0;
};

//-----------------------------------------------------------------------------
}
#endif

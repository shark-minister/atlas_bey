/*
    ATLAS - AuTomatic LAuncher System
    Copyright (C) 2025  Shark Minister
    All right reserved.

    https://x.com/shark_minister
*/
#ifndef ATLAS_FILESYS_HH
#define ATLAS_FILESYS_HH

// C++標準ライブラリ
#include <cstdint>      // std::uint8_t

// Arduino
#include <Arduino.h>
#include <SPIFFS.h>     // フラッシュメモリをファイル保存に使う

namespace atlas
{
//-----------------------------------------------------------------------------

class FileSys
{
private:
    fs::SPIFFSFS& _spiffs;

public:
    /*!
        @brief  コンストラクタ
        @param[in,out]  spiffs  fs::SPIFFSFSインスタンス
    */
    FileSys(fs::SPIFFSFS& spiffs = SPIFFS)
        : _spiffs(spiffs)
    {
    }

    /*!
        @brief  ファイル読み書きのためのシリアル通信を開始する
        @param[in]  format_on_fail  シリアル通信開始に失敗したときにフォーマットするか
        @return  開始の成否
    */
    bool begin(bool format_on_fail = false)
    {
        return _spiffs.begin(format_on_fail);
    }

    /*!
        @brief  ファイルを読み込む
        @param[in]   filename   ファイル名
        @param[out]  buf        読み込み先バッファ
        @param[in]   size       読み込むサイズ
        @return  読み込みの成否
    */
    bool read(const char* filename,
              void* buf,
              std::size_t size)
    {
        // ファイルが存在するかどうかの確認
        if (_spiffs.exists(filename))
        {
            // ファイルを開くことに成功すれば
            if (File file = _spiffs.open(filename, "r"))
            {
                // ファイルのサイズが、読み込もうとしているバッファのサイズと同じか
                if (file.size() == size)
                {
                    // ファイルの内容をバッファに読み込む
                    file.read(static_cast<std::uint8_t*>(buf), size);
                    file.close();
                    return true;
                }
                file.close();
            }
        }
        return false;
    }

    /*!
        @brief  ファイルに書き込む
        @param[in]   filename   ファイル名
        @param[out]  buf        バッファ
        @param[in]   size       書き込むサイズ
        @return  書き込みの成否
    */
    bool write(const char* filename,
               const void* buf,
               std::size_t size)
    {
        // ファイルを開くことに成功すれば
        if (File file = _spiffs.open(filename, "w"))
        {
            // バッファの内容をファイルに書き込む
            file.write(static_cast<const std::uint8_t*>(buf), size);
            file.close();
            return true;
        }
        return false;
    }
};

//-----------------------------------------------------------------------------
} // namespace atlas
#endif

/*
    © 2025,2026  @shark_minister
    Released under the MIT License, see accompaying LICENSE.txt.
*/
// Arduino
#include <Arduino.h>

// ATLAS
#include "atlas_manager.hh"

// セットアップ関数
void setup()
{
    atlas::ATLAS.setup();
}

// ループ関数
void loop()
{
    atlas::ATLAS.run();
    // おまじない
    delay(1);
}

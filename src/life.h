#pragma once
#include <pebble.h>
  
void init_life();

void deinit_life();

void live_life(GBitmap *output);

void seed_life(GBitmap *bitmap);
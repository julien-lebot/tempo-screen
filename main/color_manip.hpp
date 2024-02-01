#pragma once

#include "src/misc/lv_color.h"

void create_hue_shift(lv_color_t* palette, int min_lightness, int max_lightness, int hue_step);
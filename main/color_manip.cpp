#include <cmath>
#include "colorm.hpp"
#include "color_manip.hpp"

double adjust_hue(double val)
{
    if (val < 0.0)
    {
        val += std::ceil(-val / 360.0) * 360.0;
    }

    return std::fmod(val, 360.0);
}

double map(double n, double start1, double end1, double start2, double end2)
{
    return (n - start1) / (end1 - start1) * (end2 - start2) + start2;
}

// https://tympanus.net/codrops/2021/12/07/coloring-with-code-a-programmatic-approach-to-design/
void create_hue_shift(lv_color_t* palette, int min_lightness, int max_lightness, int hue_step)
{
    const colorm::Lch base_lch(colorm::Rgb(palette[4].red, palette[4].green, palette[4].blue));

    // Initial palette should look like this:
    // [] [] [] [] [base] [] [] [] []
    for (int i = 1; i < 5; ++i)
    {
        constexpr int palette_size = 9;
        const double hue_dark = adjust_hue(base_lch.hue() - hue_step * i);
        const double hue_light = adjust_hue(base_lch.hue() + hue_step * i);
        const double lightness_dark = map(i, 0, 4, base_lch.lightness(), min_lightness);
        const double lightness_light = map(i, 0, 4, base_lch.lightness(), max_lightness);
        const double chroma = base_lch.chroma();

        // (palette_size - 1) / 2 = 4
        // i =  1   2   3   4
        //     4-1 4-2 4-3 4-4
        // p =  3   2   1   0
        colorm::Rgb lighter_color(colorm::Lch(lightness_light, chroma, hue_light));
        const int lighter_index = (palette_size - 1) / 2 - i;
        palette[lighter_index].red = static_cast<uint8_t>(lighter_color.red8());
        palette[lighter_index].green = static_cast<uint8_t>(lighter_color.green8());
        palette[lighter_index].blue = static_cast<uint8_t>(lighter_color.blue8());

        // (palette_size - 1) / 2 = 4
        // i =  1   2   3   4
        //     4+1 4+2 4+3 4+4
        // p =  5   6   7   8
        colorm::Rgb darker_color(colorm::Lch(lightness_dark, chroma, hue_dark));
        const int darker_index = (palette_size - 1) / 2 + i;
        palette[darker_index].red = static_cast<uint8_t>(darker_color.red8());
        palette[darker_index].green = static_cast<uint8_t>(darker_color.green8());
        palette[darker_index].blue = static_cast<uint8_t>(darker_color.blue8());
    }
}
/*******************************************************************************
 * Size: 14 px
 * Bpp: 4
 * Opts: 
 ******************************************************************************/

#include "lvgl.h"

#ifndef COURIERPRIMEBOLD14
#define COURIERPRIMEBOLD14 1
#endif

#if COURIERPRIMEBOLD14

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+002D "-" */
    0x7a, 0xaa, 0xaa, 0x1c, 0xff, 0xff, 0xf2,

    /* U+0030 "0" */
    0x0, 0x6e, 0xfa, 0x0, 0x5, 0xfe, 0xcf, 0xb0,
    0xc, 0xf2, 0xb, 0xf2, 0xf, 0xd0, 0x6, 0xf6,
    0x1f, 0xc0, 0x5, 0xf7, 0xf, 0xd0, 0x6, 0xf6,
    0xc, 0xf1, 0xb, 0xf2, 0x5, 0xfe, 0xcf, 0xb0,
    0x0, 0x6e, 0xfa, 0x10,

    /* U+0031 "1" */
    0x0, 0x4d, 0x30, 0x1, 0xbf, 0xf4, 0x0, 0xaf,
    0xcf, 0x40, 0x2, 0x27, 0xf4, 0x0, 0x0, 0x7f,
    0x40, 0x0, 0x7, 0xf4, 0x0, 0x0, 0x7f, 0x40,
    0x3, 0x8b, 0xfa, 0x81, 0x8f, 0xff, 0xff, 0x30,

    /* U+0032 "2" */
    0x2a, 0xef, 0xa1, 0xa, 0xfb, 0xbf, 0xa0, 0xae,
    0x0, 0xed, 0x3, 0x50, 0x1f, 0xc0, 0x0, 0xc,
    0xf5, 0x0, 0xc, 0xf8, 0x0, 0xb, 0xf9, 0x2b,
    0x1a, 0xff, 0x9b, 0xf4, 0xdf, 0xff, 0xff, 0x30,

    /* U+0033 "3" */
    0x4c, 0xff, 0xb1, 0x9, 0xfa, 0xbf, 0xb0, 0x5a,
    0x0, 0xed, 0x0, 0x8, 0xbf, 0x80, 0x0, 0xef,
    0xf6, 0x0, 0x0, 0x1d, 0xf1, 0x0, 0x0, 0xcf,
    0x19, 0xfb, 0xcf, 0xc0, 0x5c, 0xfe, 0xa2, 0x0,

    /* U+0034 "4" */
    0x0, 0x0, 0xbc, 0x0, 0x0, 0x8, 0xfe, 0x0,
    0x0, 0x4f, 0xfe, 0x0, 0x1, 0xec, 0xbe, 0x0,
    0xa, 0xf2, 0xbe, 0x0, 0x2f, 0xff, 0xff, 0xf7,
    0x8, 0x88, 0xef, 0x83, 0x0, 0x28, 0xef, 0x81,
    0x0, 0x5f, 0xff, 0xf3,

    /* U+0035 "5" */
    0x4f, 0xff, 0xfa, 0x5, 0xfb, 0x99, 0x50, 0x6f,
    0x30, 0x0, 0x7, 0xfe, 0xfc, 0x20, 0x7f, 0xcc,
    0xfd, 0x0, 0x0, 0xc, 0xf1, 0x11, 0x0, 0xdf,
    0x1b, 0xfb, 0xdf, 0xc0, 0x4c, 0xfe, 0xa1, 0x0,

    /* U+0036 "6" */
    0x0, 0x4a, 0xef, 0x0, 0x8f, 0xfe, 0xc0, 0x4f,
    0xd3, 0x0, 0xa, 0xf5, 0x87, 0x0, 0xdf, 0xff,
    0xfc, 0xd, 0xf5, 0xa, 0xf4, 0xbf, 0x20, 0x7f,
    0x54, 0xfe, 0xbf, 0xe1, 0x6, 0xdf, 0xc3, 0x0,

    /* U+0037 "7" */
    0xaf, 0xff, 0xff, 0x5b, 0xf9, 0x9e, 0xf3, 0xbd,
    0x0, 0xfd, 0x4, 0x50, 0x6f, 0x70, 0x0, 0xc,
    0xf1, 0x0, 0x2, 0xfb, 0x0, 0x0, 0x9f, 0x40,
    0x0, 0xe, 0xe0, 0x0, 0x2, 0xe7, 0x0, 0x0,

    /* U+0038 "8" */
    0x7, 0xef, 0xb1, 0x4, 0xfd, 0xbf, 0xb0, 0x8f,
    0x30, 0xde, 0x5, 0xf7, 0x3e, 0xc0, 0xc, 0xff,
    0xf3, 0x8, 0xf8, 0x6e, 0xe0, 0xcf, 0x0, 0xaf,
    0x29, 0xfc, 0xaf, 0xf0, 0xa, 0xef, 0xc3, 0x0,

    /* U+0039 "9" */
    0x9, 0xef, 0xa1, 0x9, 0xfc, 0xcf, 0xb0, 0xee,
    0x0, 0xbf, 0x1d, 0xf2, 0x1d, 0xf3, 0x7f, 0xff,
    0xff, 0x40, 0x48, 0x6a, 0xf1, 0x0, 0x6, 0xfc,
    0x2, 0xce, 0xff, 0x30, 0x3f, 0xe9, 0x20, 0x0,

    /* U+0057 "W" */
    0x1f, 0xff, 0x50, 0xdf, 0xf7, 0xa, 0xf9, 0x20,
    0x6e, 0xd3, 0x4, 0xf4, 0x48, 0xd, 0x90, 0x2,
    0xf5, 0xbf, 0x2e, 0x80, 0x1, 0xf7, 0xff, 0x7f,
    0x60, 0x0, 0xfd, 0xfb, 0xdf, 0x50, 0x0, 0xef,
    0xb5, 0xff, 0x30, 0x0, 0xcf, 0x61, 0xff, 0x20,
    0x0, 0xae, 0x10, 0xaf, 0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 134, .box_w = 7, .box_h = 2, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 7, .adv_w = 134, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 43, .adv_w = 134, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 75, .adv_w = 134, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 107, .adv_w = 134, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 139, .adv_w = 134, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 175, .adv_w = 134, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 207, .adv_w = 134, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 239, .adv_w = 134, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 271, .adv_w = 134, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 303, .adv_w = 134, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 335, .adv_w = 134, .box_w = 10, .box_h = 9, .ofs_x = -1, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_0[] = {
    0x0, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9,
    0xa, 0xb, 0xc, 0x2a
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 45, .range_length = 43, .glyph_id_start = 1,
        .unicode_list = unicode_list_0, .glyph_id_ofs_list = NULL, .list_length = 12, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 1,
    .bpp = 4,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t CourierPrimeBold14 = {
#else
lv_font_t CourierPrimeBold14 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 9,          /*The maximum line height required by the font*/
    .base_line = 0,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
    .fallback = NULL,
    .user_data = NULL
};



#endif /*#if COURIERPRIMEBOLD14*/


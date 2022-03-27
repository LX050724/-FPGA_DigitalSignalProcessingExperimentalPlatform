#ifndef LV_USER_FONT_H
#define LV_USER_FONT_H

#include "src/font/lv_font.h"

typedef struct {
    uint16_t min;
    uint16_t max;
    uint8_t bpp;
    uint8_t reserved[3];
} x_header_t;

typedef struct {
    uint32_t pos;
} x_table_t;

typedef struct {
    uint8_t adv_w;
    uint8_t box_w;
    uint8_t box_h;
    int8_t ofs_x;
    int8_t ofs_y;
    uint8_t r;
} glyph_dsc_t;

typedef struct {
    x_header_t head;
    uint8_t *data;
    lv_font_t *font_obj;
    uint16_t min;
    uint16_t max;
    uint8_t bpp;
    lv_coord_t line_height;
    lv_coord_t base_line;
    const char *filename;
} x_file_t;

void lv_user_font_load(const char *dir);

#endif
//
// Created by yaoji on 2022/1/24.
//

#ifndef ZYNQ7020_SPINBOX_H
#define ZYNQ7020_SPINBOX_H

#include "lvgl.h"

#define SPINBOX_DATA_PRT 1
#define SPINBOX_CB_PRT 0

lv_obj_t *spinbox_create(lv_obj_t *parent, lv_obj_t *last_obj, lv_style_t *style, lv_align_t align,
                         const char *text, int32_t range_min, int32_t range_max,
                         uint8_t digit_count, uint8_t separator_position, void *ptr, uint8_t data_or_fun);

#endif //ZYNQ7020_SPINBOX_H

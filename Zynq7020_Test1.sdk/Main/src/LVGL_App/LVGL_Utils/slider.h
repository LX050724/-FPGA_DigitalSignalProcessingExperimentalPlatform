//
// Created by yaoji on 2022/1/26.
//

#ifndef ZYNQ7020_SLIDER_H
#define ZYNQ7020_SLIDER_H

#include "lvgl.h"

lv_obj_t *slider_create(lv_obj_t *parent, lv_obj_t *align_base, const char* fmt, int32_t min, int32_t max);

#endif //ZYNQ7020_SLIDER_H

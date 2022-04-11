//
// Created by yaoji on 2022/4/11.
//

#ifndef ZYNQ7020_BMP_ENCODER_H
#define ZYNQ7020_BMP_ENCODER_H

#include "lvgl.h"

int bmp_save(int w, int h, lv_color_t *img, const char *filename);

#endif //ZYNQ7020_BMP_ENCODER_H

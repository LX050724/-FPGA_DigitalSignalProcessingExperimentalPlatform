//
// Created by yaoji on 2022/1/16.
//

#ifndef ZYNQ7020_OSCILLOSCOPE_H
#define ZYNQ7020_OSCILLOSCOPE_H

#include "lvgl.h"

void Oscilloscope_create(lv_obj_t *parent);
void ADC_set_offset(int8_t offset);

#endif //ZYNQ7020_OSCILLOSCOPE_H

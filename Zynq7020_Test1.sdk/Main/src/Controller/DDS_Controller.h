//
// Created by yaoji on 2022/1/29.
//

#ifndef ZYNQ7020_DDS_CONTROLLER_H
#define ZYNQ7020_DDS_CONTROLLER_H

#include "DAC_Controller.h"

enum {
    TYPE_SINE,
    TYPE_SQUARE,
    TYPE_TRIANGLE,
    TYPE_RISING_RAMP,
    TYPE_FALLING_RAMP,
    TYPE_STAIR_STEP,
    TYPE_RAW_DATA
};

typedef struct {
    uint32_t type;
    uint32_t freq;
    uint32_t amplitude;
    uint32_t offset;
    uint32_t phase;
} DDS_base_t;

typedef struct {
    DDS_base_t base;
} DDS_sine_t;

typedef struct {
    DDS_base_t base;
} DDS_triangle_t;

typedef struct {
    DDS_base_t base;
} DDS_rising_ramp_t;

typedef struct {
    DDS_base_t base;
} DDS_falling_ramp_t;

typedef struct {
    DDS_base_t base;
    uint32_t duty_cycle;
} DDS_square_t;

typedef struct {
    DDS_base_t base;
    uint32_t rising;
    uint32_t falling;
} DDS_stair_step_t;

uint32_t DDS_get_type(void *param);

int DDS_wav_generator(void *param);
int DDS_wav_from_data(int8_t *data, int len);

#endif //ZYNQ7020_DDS_CONTROLLER_H

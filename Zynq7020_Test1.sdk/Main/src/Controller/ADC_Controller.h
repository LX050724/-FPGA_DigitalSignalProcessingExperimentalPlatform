//
// Created by yaoji on 2022/1/22.
//

#ifndef ZYNQ7020_ADC_CONTROLLER_H
#define ZYNQ7020_ADC_CONTROLLER_H

#include "xaxidma.h"

typedef enum {
    RISING_EDGE_TRIGGER = 0,
    FALLING_EDGE_TRIGGER = 1,
    AUTO_TRIGGER = 2,
} trigger_condition_e;

int ADC_init_dma_channel(XAxiDma *interface);
int ADC_get_data(bool *triggered);
void ADC_set_offset(int8_t offset);
int8_t ADC_get_offset();

void ADC_set_trigger_level(int16_t level);
void ADC_set_trigger_hysteresis(int16_t hysteresis);
void ADC_set_trigger_condition(trigger_condition_e condition);
void ADC_set_trigger_position(int16_t position);

int16_t ADC_get_trigger_level();
int16_t ADC_get_trigger_hysteresis();
trigger_condition_e ADC_get_trigger_condition();
int16_t ADC_get_trigger_position();

float ADC_get_period();
int ADC_get_max_min(float *max_p, float *min_p);
float ADC_get_mean();
float ADC_get_mean_cycle();
float ADC_get_rms();
float ADC_get_rms_cycle();

extern int16_t ADC_Data[4096];

#endif //ZYNQ7020_ADC_CONTROLLER_H

//
// Created by yaoji on 2022/1/22.
//

#ifndef ZYNQ7020_FFT_CONTROLLER_H
#define ZYNQ7020_FFT_CONTROLLER_H

#include "xaxidma.h"

typedef enum {
    FFT_SOURCE_ADC = 0,
    FFT_SOURCE_FIR = 1,
} fft_source_t;

int FFT_init_dma_channel(XAxiDma *interface);
int FFT_get_data();
void FFT_select_signal(fft_source_t source);

extern int16_t FFT_Data[4096];

#endif //ZYNQ7020_FFT_CONTROLLER_H

//
// Created by yaoji on 2022/1/22.
//

#ifndef ZYNQ7020_FFT_CONTROLLER_H
#define ZYNQ7020_FFT_CONTROLLER_H

#include "xaxidma.h"

int FFT_init_dma_channel(XAxiDma *interface);
int FFT_get_data();

extern float FFT_OriginalData[8192];

#endif //ZYNQ7020_FFT_CONTROLLER_H

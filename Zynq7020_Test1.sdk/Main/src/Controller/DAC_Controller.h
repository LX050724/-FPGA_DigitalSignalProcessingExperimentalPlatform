//
// Created by yaoji on 2022/1/22.
//

#ifndef ZYNQ7020_DAC_CONTROLLER_H
#define ZYNQ7020_DAC_CONTROLLER_H

#include "xaxidma.h"

#define DAC_CLK_FREQ 120000000

int DAC_init_dma_channel(XAxiDma *interface);
int DAC_start(uint8_t *data, size_t len);

#endif //ZYNQ7020_DAC_CONTROLLER_H

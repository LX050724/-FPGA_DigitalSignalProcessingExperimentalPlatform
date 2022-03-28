//
// Created by yaoji on 2022/1/22.
//

#ifndef ZYNQ7020_FIR_CONTROLLER_H
#define ZYNQ7020_FIR_CONTROLLER_H

#include <stdint-gcc.h>
#include "xaxidma.h"

int FIR_init_dma_channel(XAxiDma *interface);

/**
 * 重载滤波器系数
 * @param coe 滤波器系数，必须为长度65的奇对称系数
 * @return
 */
int FIR_reload_coe(int16_t *coe);

#endif //ZYNQ7020_FIR_CONTROLLER_H

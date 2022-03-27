//
// Created by yaoji on 2022/1/22.
//

#ifndef ZYNQ7020_MAIN_H
#define ZYNQ7020_MAIN_H

#include "xiicps.h"
#include "xgpiops.h"
#include "xaxidma.h"
#include "xaxis_switch.h"
#include "xqspips.h"
#include "xadcps.h"

extern XIicPs iic0, iic1;
extern XGpioPs gpio;
extern XAxiDma dma0, dma1;
extern XAxis_Switch axisSwitch;
extern XQspiPs QspiInstance;
extern XAdcPs xAdcPs;

#endif //ZYNQ7020_MAIN_H

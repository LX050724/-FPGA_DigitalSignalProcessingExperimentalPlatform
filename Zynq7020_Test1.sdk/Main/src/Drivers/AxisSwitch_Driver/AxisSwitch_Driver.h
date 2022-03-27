//
// Created by yaoji on 2022/3/27.
//

#ifndef ZYNQ7020_AXISSWITCH_DRIVER_H
#define ZYNQ7020_AXISSWITCH_DRIVER_H

typedef enum {
    FIR_RELOAD = 0,
    FIR_CONFIG = 1,
    FFT_CONFIG = 2,
} AxisSwitch_Channel;

int AxisSwitch_init();

/**
 * 切换axis通道
 * @param channel 目标通道
 * @retval TRUE 成功切换
 * @retval FALSE 切换失败
 */
int AxisSwitch_switch(AxisSwitch_Channel channel);

#endif //ZYNQ7020_AXISSWITCH_DRIVER_H

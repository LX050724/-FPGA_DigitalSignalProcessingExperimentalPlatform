//
// Created by yaoji on 2022/3/26.
//

#ifndef ZYNQ7020_AMPLITUDERESPONSE_H
#define ZYNQ7020_AMPLITUDERESPONSE_H

#include <stdint.h>

/**
 * 通过滤波器系数计算滤波器幅值响应
 * @param coe_input 滤波器系数输入
 * @param amp_output 幅值响应输出，单位分贝
 * @param len 滤波器系数长度
 * @return
 */
int AmplitudeResponse(const int16_t *coe_input, float *amp_output, int len);

#endif //ZYNQ7020_AMPLITUDERESPONSE_H

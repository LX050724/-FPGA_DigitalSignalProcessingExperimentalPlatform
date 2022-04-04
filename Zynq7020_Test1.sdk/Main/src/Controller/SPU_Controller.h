/**
 * @file SPU_Controller.h
 * @author yao
 * @date 2022年3月27日
 * @details 数字信号处理单元控制器
 */

#ifndef ZYNQ7020_SPU_CONTROLLER_H
#define ZYNQ7020_SPU_CONTROLLER_H

#include <stdint.h>

typedef enum {
    CHANNEL_INDEX_DAC,
    CHANNEL_INDEX_FFT,
    CHANNEL_INDEX_SCOPE,
    CHANNEL_INDEX_FIR,
} Channel_Index;

typedef enum {
    ADC_PackPulse,
    FFT_PackPulse,
} Pulse_Type;

typedef enum {
    FIR_RELOAD = 0,
    FIR_CONFIG = 1,
} FIR_Channel;

typedef enum {
    DAC_DDS = 0,
    DAC_FIR = 1,
} DAC_Channel;

typedef enum {
    FFT_ADC = 0,
    FFT_FIR = 1,
} FFT_Channel;

typedef enum {
    SCOPE_ADC = 0,
    SCOPE_FIR = 1,
} Scope_Channel;

void SPU_SwitchChannelSource(Channel_Index index, int channel);
void SPU_SendPackPulse(Pulse_Type pulseType);
void SPU_SetAdcOffset(int32_t offset);
void SPU_SetDacOffset(int32_t offset);
void SPU_SetFirShift(uint32_t shift);

#endif //ZYNQ7020_SPU_CONTROLLER_H

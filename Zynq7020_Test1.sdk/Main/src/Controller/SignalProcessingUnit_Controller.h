//
// Created by yaoji on 2022/3/27.
//

#ifndef ZYNQ7020_SIGNALPROCESSINGUNIT_CONTROLLER_H
#define ZYNQ7020_SIGNALPROCESSINGUNIT_CONTROLLER_H

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

void SignalProcessingUnit_switch_axis(Channel_Index index, int channel);
void SignalProcessingUnit_send_pulse(Pulse_Type pulseType);
void SignalProcessingUnit_set_ADC_Offset(int32_t offset);
void SignalProcessingUnit_set_DAC_Offset(int32_t offset);
void SignalProcessingUnit_set_FIR_shift(uint32_t shift);

#endif //ZYNQ7020_SIGNALPROCESSINGUNIT_CONTROLLER_H

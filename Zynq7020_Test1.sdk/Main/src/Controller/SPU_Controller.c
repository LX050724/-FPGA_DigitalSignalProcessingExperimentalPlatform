//
// Created by yaoji on 2022/3/30.
//

#include "SPU_Controller.h"

#include "xparameters.h"

typedef struct {
    volatile uint32_t ADC_Offset;
    volatile uint32_t DAC_Offset;
    volatile uint32_t DAC_SignalSwitch : 1;
    volatile uint32_t FFT_SignalSwitch : 1;
    volatile uint32_t ADC_SignalSwitch : 1;
    volatile uint32_t FIR_SignalSwitch : 1;
    volatile uint32_t ADC_PackPulse : 1;
    volatile uint32_t FFT_PackPulse : 1;
    volatile uint32_t FIR_Shift;
} AXI4IO_reg_t;

#define AXI4IO ((AXI4IO_reg_t *)XPAR_ADDA_AXI4_IO_0_S00_AXI_BASEADDR)

void SPU_SwitchChannelSource(Channel_Index index, int channel) {
    switch (index) {
        case CHANNEL_INDEX_DAC:
            AXI4IO->DAC_SignalSwitch = channel;
            break;
        case CHANNEL_INDEX_FFT:
            AXI4IO->FFT_SignalSwitch = channel;
            break;
        case CHANNEL_INDEX_SCOPE:
            AXI4IO->ADC_SignalSwitch = channel;
            break;
        case CHANNEL_INDEX_FIR:
            AXI4IO->FIR_SignalSwitch = channel;
            break;
    }
}

void SPU_SendPackPulse(Pulse_Type pulseType) {
    switch (pulseType) {
        case ADC_PackPulse:
            AXI4IO->ADC_PackPulse = 1;
            AXI4IO->ADC_PackPulse = 0;
            break;
        case FFT_PackPulse:
            AXI4IO->FFT_PackPulse = 1;
            AXI4IO->FFT_PackPulse = 0;
            break;
    }
}

void SPU_SetAdcOffset(int32_t offset) {
    AXI4IO->ADC_Offset = offset;
}

void SPU_SetDacOffset(int32_t offset) {
    AXI4IO->DAC_Offset = offset;
}

void SPU_SetFirShift(uint32_t shift) {
    AXI4IO->FIR_Shift = shift;
}

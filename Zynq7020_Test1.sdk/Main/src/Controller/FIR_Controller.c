//
// Created by yaoji on 2022/1/22.
//

#include "FIR_Controller.h"
#include "AxisSwitch_Driver/AxisSwitch_Driver.h"
#include "utils.h"
#include "FreeRTOS.h"
#include "task.h"
#include "DMA_Driver/DMA_Driver.h"

static XAxiDma *DmaInterface;
static uint8_t fir_config __attribute__((aligned(8)));

int FIR_init_dma_channel(XAxiDma *interface) {
    DmaInterface = interface;
    return XST_SUCCESS;
}

int FIR_reload_coe(int16_t *coe) {
    if (coe == NULL) return XST_INVALID_PARAM;
    for (int i = 0; i < 32; i++) {
        if (coe[i] != coe[64 - i])
            return XST_INVALID_PARAM;
    }
    CHECK_STATUS_RET(AxisSwitch_switch(FIR_RELOAD));
    CHECK_STATUS_RET(DMA_send_package(DmaInterface, (UINTPTR) coe, sizeof(uint16_t) * 33));
    CHECK_STATUS_RET(AxisSwitch_switch(FIR_CONFIG));
    CHECK_STATUS_RET(DMA_send_package(DmaInterface, (UINTPTR) &fir_config, 1));
    return XST_SUCCESS;
}


//
// Created by yaoji on 2022/1/22.
//

#include "FIR_Controller.h"
#include "AxisSwitch_Driver/AxisSwitch_Driver.h"
#include "utils.h"
#include "FreeRTOS.h"
#include "task.h"
#include "DMA_Driver/DMA_Driver.h"

static XAxiDma_BdRing *RingPtr;
static XAxiDma_Bd *BdPtr;

int FIR_init_dma_channel(XAxiDma *interface) {
    RingPtr = XAxiDma_GetTxRing(interface);
    return RingPtr ? XST_SUCCESS : XST_FAILURE;
}

int FIR_reload_coe(uint16_t *coe) {
    if (coe == NULL) return XST_INVALID_PARAM;
    for (int i = 0; i < 32; i++) {
        if (coe[i] != coe[64 - i])
            return XST_INVALID_PARAM;
    }
    AxisSwitch_switch(FIR_RELOAD);
    CHECK_STATUS_RET(DMA_send_package(RingPtr, (UINTPTR) coe, sizeof(uint16_t) * 33));
    AxisSwitch_switch(FIR_CONFIG);
    uint8_t fir_config = 0;
    CHECK_STATUS_RET(DMA_send_package(RingPtr, (UINTPTR) &fir_config, 1));
    return XST_SUCCESS;
}


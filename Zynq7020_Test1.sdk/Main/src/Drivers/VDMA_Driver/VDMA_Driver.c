/*
 * VDMA_Init.c
 *
 *  Created on: 2021年12月25日
 *      Author: yaoji
 */

#include <VDMA_Driver/VDMA_Driver.h>

#include "ScuGic_Driver/ScuGic_Driver.h"
#include "stdlib.h"
#include "utils.h"
#include "xaxivdma.h"
#include "xil_io.h"

static XAxiVdma vdma;

lv_color_t GRAM0[VDMA_V_ACTIVE][VDMA_H_ACTIVE] __attribute__((section(".GRAM0")));
lv_color_t GRAM1[VDMA_V_ACTIVE][VDMA_H_ACTIVE] __attribute__((section(".GRAM1")));

int VDMA_Init() {
    XAxiVdma_Config *config = XAxiVdma_LookupConfig(XPAR_AXIVDMA_0_DEVICE_ID);
    CHECK_STATUS_RET(XAxiVdma_CfgInitialize(&vdma, config, config->BaseAddress));
    CHECK_STATUS_RET(XAxiVdma_Selftest(&vdma));
    return XST_SUCCESS;
}

int VDMA_Start() {
    XAxiVdma_DmaSetup dma_setup = {0};
    dma_setup.EnableCircularBuf = FALSE;
    dma_setup.HoriSizeInput = VDMA_H_ACTIVE * sizeof(lv_color_t);
    dma_setup.VertSizeInput = VDMA_V_ACTIVE;
    dma_setup.Stride = VDMA_H_STRIDE * sizeof(lv_color_t);
    dma_setup.FrameStoreStartAddr[0] = (UINTPTR) GRAM0;
    dma_setup.FrameStoreStartAddr[1] = (UINTPTR) GRAM1;

    CHECK_STATUS_RET(XAxiVdma_StartReadFrame(&vdma, &dma_setup));
    return XST_SUCCESS;
}

void *VDMA_GetIdleBufferPtr() {
    uint8_t FrmStoreNum = XAxiVdma_CurrFrameStore(&vdma, XAXIVDMA_READ);
    return (FrmStoreNum == 0) ? (void *) &GRAM1 : (void *) &GRAM0;
}

void VDMA_SwitchBuffer() {
    uint8_t FrmStoreNum = XAxiVdma_CurrFrameStore(&vdma, XAXIVDMA_READ);
    VDMA_SetBufferIndex(FrmStoreNum == 0 ? 1 : 0);
}

void VDMA_SetBufferIndex(uint8_t index) {
    index &= XAXIVDMA_PARKPTR_READREF_MASK;
    uint32_t PARK_PTR_REG = Xil_In32(XPAR_AXIVDMA_0_BASEADDR + XAXIVDMA_PARKPTR_OFFSET);
    PARK_PTR_REG &= ~XAXIVDMA_PARKPTR_READREF_MASK;
    PARK_PTR_REG |= index << XAXIVDMA_READREF_SHIFT;
    Xil_Out32(XPAR_AXIVDMA_0_BASEADDR + XAXIVDMA_PARKPTR_OFFSET, PARK_PTR_REG);
}

int VDMA_InitIntterupt(XScuGic *InstancePtr, uint8_t Priority, uint32_t Int_Id,
                       void *GeneralCallBackFunc) {
    CHECK_STATUS_RET(XScuGic_Connect(InstancePtr, Int_Id, XAxiVdma_ReadIntrHandler, &vdma));
    XScuGic_SetPriorityTriggerType(InstancePtr, Int_Id, Priority, INT_TYPE_HIGHLEVEL);
    XScuGic_Enable(InstancePtr, Int_Id);

    XAxiVdma_SetCallBack(&vdma, XAXIVDMA_HANDLER_GENERAL, GeneralCallBackFunc, &vdma,
                         XAXIVDMA_READ);
    XAxiVdma_IntrEnable(&vdma, XAXIVDMA_IXR_ALL_MASK, XAXIVDMA_READ);
    return XST_SUCCESS;
}

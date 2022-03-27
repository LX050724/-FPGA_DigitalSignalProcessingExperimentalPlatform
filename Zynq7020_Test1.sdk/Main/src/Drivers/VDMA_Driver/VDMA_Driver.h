/*
 * VDMA_Init.h
 *
 *  Created on: 2021年12月25日
 *      Author: yaoji
 */

#include "src/misc/lv_color.h"
#include "xil_types.h"
#include "xscugic.h"

#ifndef SRC_VDMA_DRIVER_VDMA_INIT_H_
#define SRC_VDMA_DRIVER_VDMA_INIT_H_

// #define VDMA_H_ACTIVE 1280
// #define VDMA_V_ACTIVE 720
// #define VDMA_H_STRIDE 1280

#define VDMA_H_ACTIVE 1024
#define VDMA_V_ACTIVE 600
#define VDMA_H_STRIDE 1024

#define VDMA_BUFFER_SIZE (sizeof(lv_color_t) * VDMA_H_ACTIVE * VDMA_V_ACTIVE)

int VDMA_Init();

int VDMA_Start();

void *VDMA_GetIdleBufferPtr();

void VDMA_SwitchBuffer();

void VDMA_SetBufferIndex(uint8_t index);

int VDMA_InitIntterupt(XScuGic *InstancePtr, uint8_t Priority, uint32_t Int_Id,
                       void *GeneralCallBackFunc);

extern lv_color_t GRAM0[VDMA_V_ACTIVE][VDMA_H_ACTIVE];
extern lv_color_t GRAM1[VDMA_V_ACTIVE][VDMA_H_ACTIVE];

#endif /* SRC_VDMA_DRIVER_VDMA_INIT_H_ */

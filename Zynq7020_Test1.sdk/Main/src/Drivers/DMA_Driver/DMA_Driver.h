/*
 * DMA_Driver.h
 *
 *  Created on: 2022Äê1ÔÂ3ÈÕ
 *      Author: yaoji
 */

#ifndef SRC_DRIVERS_DMA_DRIVER_DMA_DRIVER_H_
#define SRC_DRIVERS_DMA_DRIVER_DMA_DRIVER_H_

#include "xaxidma.h"

int DMA_Init(XAxiDma *dma, uint32_t DeviceId);
int DMA_SetRxRing(XAxiDma *dma, XAxiDma_Bd *RxBdPtr, size_t BdSize);
int DMA_SetTxRing(XAxiDma *dma, XAxiDma_Bd *TxBdPtr, size_t BdSize);
int DMA_send_package(XAxiDma *InstancePtr, UINTPTR data, size_t size);
void XAxiDma_MM2SIntrHandler(void *param);

#endif /* SRC_DRIVERS_DMA_DRIVER_DMA_DRIVER_H_ */

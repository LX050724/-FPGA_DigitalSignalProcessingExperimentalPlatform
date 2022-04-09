/*
 * DMA_Driver.c
 *
 *  Created on: 2022年1月3日
 *      Author: yaoji
 */

#include "DMA_Driver/DMA_Driver.h"
#include "xil_io.h"
#include "check.h"
#include "Timer_Driver/Timer_Driver.h"
#include <FreeRTOS.h>
#include <task.h>


/**
 * @brief 初始化DMA
 * 
 * @param dma DMA对象指针
 * @param DeviceId DMA设备ID
 * @return int 
 */
int DMA_Init(XAxiDma *dma, uint32_t DeviceId) {
    XAxiDma_Config *config = XAxiDma_LookupConfig(DeviceId);
    CHECK_STATUS_RET(XAxiDma_CfgInitialize(dma, config));
    return XST_SUCCESS;
}

/**
 * @brief 设置DMA发送环，并设置中断计数为1，关闭定时器
 * 
 * @param dma DMA对象指针
 * @param RxBdPtr RxBd指针，需要64字节对齐
 * @param BdSize 描述符空间大小
 * @return int 
 */
int DMA_SetRxRing(XAxiDma *dma, XAxiDma_Bd *RxBdPtr, size_t BdSize) {
    XAxiDma_BdRing *RxRingPtr = XAxiDma_GetRxRing(dma);

    /* Set RX Timer and Counter */
    int Timer = 0;
    int Counter = 1;
    CHECK_STATUS_RET(XAxiDma_BdRingSetCoalesce(RxRingPtr, Counter, Timer));

    /* Setup RxBD space  */
    uint32_t BdCount = XAxiDma_BdRingCntCalc(XAXIDMA_BD_MINIMUM_ALIGNMENT, BdSize);
    CHECK_STATUS_RET(XAxiDma_BdRingCreate(RxRingPtr, (UINTPTR) RxBdPtr, (UINTPTR) RxBdPtr, XAXIDMA_BD_MINIMUM_ALIGNMENT,
                                          BdCount));
    return XST_SUCCESS;
}

/**
 * @brief 设置DMA接收环，并设置中断计数为1，关闭定时器
 * 
 * @param dma DMA对象指针
 * @param TxBdPtr RxBd指针，需要64字节对齐
 * @param BdSize 描述符空间大小
 * @return int 
 */
int DMA_SetTxRing(XAxiDma *dma, XAxiDma_Bd *TxBdPtr, size_t BdSize) {
    XAxiDma_BdRing *TxRingPtr = XAxiDma_GetTxRing(dma);

    /* Set TX Timer and Counter */
    int Timer = 0;
    int Counter = 1;
    CHECK_STATUS_RET(XAxiDma_BdRingSetCoalesce(TxRingPtr, Counter, Timer));

    /* Setup TxBD space  */
    uint32_t BdCount = XAxiDma_BdRingCntCalc(XAXIDMA_BD_MINIMUM_ALIGNMENT, BdSize);
    CHECK_STATUS_RET(XAxiDma_BdRingCreate(TxRingPtr, (UINTPTR) TxBdPtr, (UINTPTR) TxBdPtr, XAXIDMA_BD_MINIMUM_ALIGNMENT,
                                          BdCount));
    return XST_SUCCESS;
}

void XAxiDma_MM2SIntrHandler(void *param) {
    XAxiDma *dma = param;
    uint32_t IrqMask = XAxiDma_IntrGetIrq(dma, XAXIDMA_DMA_TO_DEVICE);
    XAxiDma_BdRing *TxRingPtr = XAxiDma_GetTxRing(dma);

    if (IrqMask & XAXIDMA_IRQ_IOC_MASK) xil_printf("XAXIDMA_IRQ_IOC_MASK\r\n");

    if (IrqMask & XAXIDMA_IRQ_DELAY_MASK) xil_printf("XAXIDMA_IRQ_DELAY_MASK\r\n");

    if (IrqMask & XAXIDMA_IRQ_ERROR_MASK) {
        xil_printf("XAXIDMA_IRQ_ERROR_MASK\r\n");
        uint32_t BdRingErr = XAxiDma_BdRingGetError(TxRingPtr);

        uint32_t mask[] = {
                XAXIDMA_ERR_INTERNAL_MASK, XAXIDMA_ERR_SLAVE_MASK, XAXIDMA_ERR_DECODE_MASK,
                XAXIDMA_ERR_SG_INT_MASK, XAXIDMA_ERR_SG_SLV_MASK, XAXIDMA_ERR_SG_DEC_MASK,
        };

        const char *str[] = {
                "XAXIDMA_ERR_INTERNAL_MASK", "XAXIDMA_ERR_SLAVE_MASK", "XAXIDMA_ERR_DECODE_MASK",
                "XAXIDMA_ERR_SG_INT_MASK", "XAXIDMA_ERR_SG_SLV_MASK", "XAXIDMA_ERR_SG_DEC_MASK",
        };

        for (int i = 0; i < 6; i++) {
            if (BdRingErr & mask[i])
                xil_printf("%s\r\n", str[i]);
        }
    }
}

int DMA_send_package(XAxiDma *InstancePtr, UINTPTR data, size_t size) {
    int status = XST_SUCCESS;
    vPortEnterCritical();
    Xil_DCacheFlushRange(data, size);
    status = XAxiDma_SimpleTransfer(InstancePtr, data, size, XAXIDMA_DMA_TO_DEVICE);

    int start_time_ms = getTime_millis();
    while (XAxiDma_Busy(InstancePtr, XAXIDMA_DMA_TO_DEVICE)) {
        if (getTime_millis() - start_time_ms > 5) {
            vPortExitCritical();
            return XST_FAILURE;
        }
    }

    vPortExitCritical();
    return status;
}

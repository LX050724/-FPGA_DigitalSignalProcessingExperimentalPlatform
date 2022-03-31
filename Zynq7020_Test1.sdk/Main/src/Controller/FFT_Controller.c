//
// Created by yaoji on 2022/1/22.
//

#include "FFT_Controller.h"
#include "xaxidma.h"
#include "SPU_Controller.h"
#include "utils.h"
#include "FreeRTOS.h"
#include "task.h"

#define AXI4_IO_FFT_SOURCE_MASK 0x00000002

int16_t FFT_Data[4096];

static XAxiDma_BdRing *RingPtr;
static XAxiDma_Bd *BdPtr;
static int16_t FFT_OriginalData[8192] __attribute__((aligned(8)));

/**
 * 初始化FFT使用的DMA通道
 * @param interface DMA接口
 * @return
 */
int FFT_init_dma_channel(XAxiDma *interface) {
    XAxiDma_SelectCyclicMode(interface, XAXIDMA_DEVICE_TO_DMA, TRUE);
    RingPtr = XAxiDma_GetRxRing(interface);
    XAxiDma_BdRingEnableCyclicDMA(RingPtr);

    CHECK_STATUS_RET(XAxiDma_BdRingAlloc(RingPtr, 1, &BdPtr));
    CHECK_STATUS_RET(XAxiDma_BdSetBufAddr(BdPtr, (UINTPTR) FFT_OriginalData));
    CHECK_STATUS_RET(XAxiDma_BdSetLength(BdPtr, sizeof(FFT_OriginalData), RingPtr->MaxTransferLen));
    XAxiDma_BdSetCtrl(BdPtr, XAXIDMA_BD_CTRL_ALL_MASK);
    XAxiDma_BdWrite(BdPtr, XAXIDMA_BD_NDESC_OFFSET, BdPtr);
    XAxiDma_BdSetId(BdPtr, (UINTPTR) FFT_OriginalData);

    /* 将描述符链表起始地址下载至DMA寄存器中 */
    CHECK_STATUS_RET(XAxiDma_BdRingToHw(RingPtr, 1, BdPtr));

    /* 启动DMA接收 */
    CHECK_STATUS_RET(XAxiDma_BdRingStart(RingPtr));

    /* 向FFT Packager发送启动信号 */
    SPU_SendPulse(FFT_PackPulse);

    return XST_SUCCESS;
}

/**
 * 获取FFT数据并转化为单边谱
 * @return
 */
int FFT_get_data() {
    int status = XST_SUCCESS;
    XAXIDMA_CACHE_INVALIDATE(BdPtr);
    uint32_t receive_len = XAxiDma_BdGetActualLength(BdPtr, 0xffff);
    if (receive_len == sizeof(FFT_OriginalData)) {
        Xil_DCacheInvalidateRange((INTPTR) FFT_OriginalData, sizeof(FFT_OriginalData));
        memcpy(FFT_Data, FFT_OriginalData, sizeof(FFT_Data));
    } else {
        xil_printf("warning: FFT data length is incorrect\r\n");
        status = XST_DATA_LOST;
    }

    /* 向FFT Packager发送启动信号 */
    SPU_SendPulse(FFT_PackPulse);

    return status;
}

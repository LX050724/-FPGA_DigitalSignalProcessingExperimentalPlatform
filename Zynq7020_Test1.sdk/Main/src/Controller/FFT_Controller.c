//
// Created by yaoji on 2022/1/22.
//

#include "FFT_Controller.h"
#include "xaxidma.h"
#include "SPU_Controller.h"
#include "check.h"
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOS_Mem/FreeRTOS_Mem.h"

static XAxiDma *dma;
float FFT_OriginalData[8192] __attribute__((aligned(8)));


/**
 * 初始化FFT使用的DMA通道
 * @param interface DMA接口
 * @return
 */
int FFT_init_dma_channel(XAxiDma *interface) {
	dma = interface;
	CHECK_STATUS_RET(XAxiDma_SimpleTransfer(dma, (UINTPTR)FFT_OriginalData, sizeof(FFT_OriginalData), XAXIDMA_DEVICE_TO_DMA));
    /* 向FFT Packager发送启动信号 */
    SPU_SendPackPulse(FFT_PackPulse);
    return XST_SUCCESS;
}

/**
 * 获取FFT数据并转化为单边谱
 * @return
 */
int FFT_get_data() {
	int status = XST_SUCCESS;
	if (!XAxiDma_Busy(dma, XAXIDMA_DEVICE_TO_DMA)) {
		os_DCacheInvalidateRange((INTPTR) FFT_OriginalData, sizeof(FFT_OriginalData));
		CHECK_STATUS_RET(XAxiDma_SimpleTransfer(dma, (UINTPTR)FFT_OriginalData, sizeof(FFT_OriginalData), XAXIDMA_DEVICE_TO_DMA));
	} else status = XST_DEVICE_BUSY;
	/* 向FFT Packager发送启动信号 */
	SPU_SendPackPulse(FFT_PackPulse);
	return status;
}

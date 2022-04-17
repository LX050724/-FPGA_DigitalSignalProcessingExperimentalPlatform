//
// Created by yaoji on 2022/1/22.
//

#include "DAC_Controller.h"
#include "SPU_Controller.h"
#include "check.h"

xSemaphoreHandle DAC_Mutex;

static XAxiDma_BdRing *TxRingPtr;
static XAxiDma_Bd *TxBdPtr;
static int DAC_running = 0;

int DAC_init_dma_channel(XAxiDma *interface) {
    if (!XAxiDma_HasSg(interface)) {
        return XST_NOT_SGDMA;
    }
    DAC_Mutex = xSemaphoreCreateMutex();

    SPU_SetDacOffset(127);

    XAxiDma_SelectCyclicMode(interface, XAXIDMA_DMA_TO_DEVICE, TRUE);
    TxRingPtr = XAxiDma_GetTxRing(interface);

    return XST_SUCCESS;
}

/**
 * 启动或切换DAC缓冲区，切换时间约为10us
 * @param data 缓冲区指针，地址8字节对齐
 * @param len 缓冲区长度
 * @return
 */
int DAC_start(uint8_t *data, size_t len) {
    static uint8_t *last_data = NULL;
    static size_t last_len = 0;

    int status = XST_SUCCESS;

    if (len == 0) return XST_INVALID_PARAM;
    vPortEnterCritical();
    /* 如果DMA正在运行, 停止DMA并更换缓冲区地址 */
    if (DAC_running) {
        if ((last_data == data) && (len == last_len)) goto end;
        last_data = data;
        last_len = len;
        uint32_t CR = XAxiDma_ReadReg(TxRingPtr->ChanBase, XAXIDMA_CR_OFFSET);
        uint32_t SR = XAxiDma_ReadReg(TxRingPtr->ChanBase, XAXIDMA_SR_OFFSET);
        uint32_t err = SR & XAXIDMA_ERR_ALL_MASK;
        if (err) {
//            uint32_t CDESC = XAxiDma_ReadReg(TxRingPtr->ChanBase, XAXIDMA_CDESC_OFFSET);
//            uint32_t TDESC = XAxiDma_ReadReg(TxRingPtr->ChanBase, XAXIDMA_TDESC_OFFSET);
            xil_printf("DAC Controller: err %03x\r\n", err);
        }

        XAxiDma_WriteReg(TxRingPtr->ChanBase, XAXIDMA_CR_OFFSET, CR & ~XAXIDMA_CR_RUNSTOP_MASK);

        while (XAxiDma_BdRingHwIsStarted(TxRingPtr));

        XAXIDMA_CACHE_INVALIDATE(TxBdPtr);
        xil_printf("DAC Controller: change buf addr from 0x%p [%d] to 0x%p [%d]\r\n",
                   XAxiDma_BdGetBufAddr(TxBdPtr), XAxiDma_BdGetLength(TxBdPtr, TxRingPtr->MaxTransferLen), data, len);

        CHECK_STATUS_GOTO(status, end, XAxiDma_BdSetBufAddr(TxBdPtr, (UINTPTR) data));
        CHECK_STATUS_GOTO(status, end, XAxiDma_BdSetLength(TxBdPtr, len, TxRingPtr->MaxTransferLen));
        XAXIDMA_CACHE_FLUSH(TxBdPtr);

        CR = XAxiDma_ReadReg(TxRingPtr->ChanBase, XAXIDMA_CR_OFFSET);
        XAxiDma_WriteReg(TxRingPtr->ChanBase, XAXIDMA_CR_OFFSET, CR | XAXIDMA_CR_RUNSTOP_MASK);
        uint32_t TDESC = XAxiDma_ReadReg(TxRingPtr->ChanBase, XAXIDMA_TDESC_OFFSET);
        XAxiDma_WriteReg(TxRingPtr->ChanBase, XAXIDMA_TDESC_OFFSET, TDESC);
    } else {
        last_len = len;
        last_data = data;
        xil_printf("DAC Controller: init channel 0x%p [%d]\r\n", data, len);
        CHECK_STATUS_GOTO(status, end, XAxiDma_BdRingAlloc(TxRingPtr, 1, &TxBdPtr));
        CHECK_STATUS_GOTO(status, end, XAxiDma_BdSetBufAddr(TxBdPtr, (UINTPTR) data));
        CHECK_STATUS_GOTO(status, end, XAxiDma_BdSetLength(TxBdPtr, len, TxRingPtr->MaxTransferLen));
        XAxiDma_BdSetCtrl(TxBdPtr, XAXIDMA_BD_CTRL_TXEOF_MASK | XAXIDMA_BD_CTRL_TXSOF_MASK);
        /* 设置下一描述符地址为自身地址形成单节环形链表 */
        XAxiDma_BdWrite(TxBdPtr, XAXIDMA_BD_NDESC_OFFSET, TxBdPtr);
        XAxiDma_BdSetId(TxBdPtr, (UINTPTR) data);

        /* 将描述符地址下载至DMA寄存器中 */
        CHECK_STATUS_GOTO(status, end, XAxiDma_BdRingToHw(TxRingPtr, 1, TxBdPtr));

        /* 启用DMA循环模式 */
        XAxiDma_BdRingEnableCyclicDMA(TxRingPtr);
        CHECK_STATUS_GOTO(status, end, XAxiDma_BdRingStart(TxRingPtr));
        DAC_running = 1;
    }
    end:
    vPortExitCritical();
    return status;
}

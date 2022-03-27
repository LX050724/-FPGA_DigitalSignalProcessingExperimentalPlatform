//
// Created by yaoji on 2022/1/22.
//

#include "DAC_Controller.h"
#include "AXI4_IO.h"
#include "utils.h"

static XAxiDma_BdRing *TxRingPtr;
static XAxiDma_Bd *TxBdPtr;

int DAC_init_dma_channel(XAxiDma *interface) {
    if (!XAxiDma_HasSg(interface)) {
        return XST_NOT_SGDMA;
    }

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
    /* 如果DMA正在运行, 停止DMA并更换缓冲区地址 */
    if (XAxiDma_BdRingHwIsStarted(TxRingPtr)) {
        uint32_t CR = XAxiDma_ReadReg(TxRingPtr->ChanBase, XAXIDMA_CR_OFFSET);
        XAxiDma_WriteReg(TxRingPtr->ChanBase, XAXIDMA_CR_OFFSET, CR & ~XAXIDMA_CR_RUNSTOP_MASK);

        while (XAxiDma_BdRingHwIsStarted(TxRingPtr));

        XAXIDMA_CACHE_INVALIDATE(TxBdPtr);
        xil_printf("DAC Controller: change buf addr from 0x%p [%d] to 0x%p [%d]\r\n",
                   XAxiDma_BdGetBufAddr(TxBdPtr), XAxiDma_BdGetLength(TxBdPtr, TxRingPtr->MaxTransferLen), data, len);

        CHECK_STATUS_RET(XAxiDma_BdSetBufAddr(TxBdPtr, data));
        CHECK_STATUS_RET(XAxiDma_BdSetLength(TxBdPtr, len, TxRingPtr->MaxTransferLen));
        XAXIDMA_CACHE_FLUSH(TxBdPtr);

        CR = XAxiDma_ReadReg(TxRingPtr->ChanBase, XAXIDMA_CR_OFFSET);
        XAxiDma_WriteReg(TxRingPtr->ChanBase, XAXIDMA_CR_OFFSET, CR | XAXIDMA_CR_RUNSTOP_MASK);
        uint32_t TDESC = XAxiDma_ReadReg(TxRingPtr->ChanBase, XAXIDMA_TDESC_OFFSET);
        XAxiDma_WriteReg(TxRingPtr->ChanBase, XAXIDMA_TDESC_OFFSET, TDESC);
    } else {
        xil_printf("DAC Controller: init channel 0x%p [%d]\r\n", data, len);
        CHECK_STATUS_RET(XAxiDma_BdRingAlloc(TxRingPtr, 1, &TxBdPtr));
        CHECK_STATUS_RET(XAxiDma_BdSetBufAddr(TxBdPtr, data));
        CHECK_STATUS_RET(XAxiDma_BdSetLength(TxBdPtr, len, TxRingPtr->MaxTransferLen));
        XAxiDma_BdSetCtrl(TxBdPtr, XAXIDMA_BD_CTRL_TXEOF_MASK | XAXIDMA_BD_CTRL_TXSOF_MASK);
        /* 设置下一描述符地址为自身地址形成单节环形链表 */
        XAxiDma_BdWrite(TxBdPtr, XAXIDMA_BD_NDESC_OFFSET, TxBdPtr);
        XAxiDma_BdSetId(TxBdPtr, (UINTPTR) data);

        /* 将描述符地址下载至DMA寄存器中 */
        CHECK_STATUS_RET(XAxiDma_BdRingToHw(TxRingPtr, 1, TxBdPtr));

        /* 启用DMA循环模式 */
        XAxiDma_BdRingEnableCyclicDMA(TxRingPtr);
        CHECK_STATUS_RET(XAxiDma_BdRingStart(TxRingPtr));
    }
    return XST_SUCCESS;
}

/*
 * I2C_Driver.c
 *
 *  Created on: 2021��12��30��
 *      Author: yaoji
 */

#include "Timer_Driver/Timer_Driver.h"
#include "check.h"
#include "xiicps.h"
#include "xil_cache.h"

int I2C_Init(XIicPs *InstancePtr, uint16_t DeviceId, uint32_t FsclHz) {
    XIicPs_Config *config = XIicPs_LookupConfig(DeviceId);
    CHECK_STATUS_RET(XIicPs_CfgInitialize(InstancePtr, config, config->BaseAddress));
    CHECK_STATUS_RET(XIicPs_SelfTest(InstancePtr));
    CHECK_STATUS_RET(XIicPs_SetSClk(InstancePtr, FsclHz));
    return XST_SUCCESS;
}

/**
 * @brief
 *
 * @param InstancePtr
 * @param SlaveAddr
 * @param RegAddr
 * @param data
 * @param len
 * @return int
 */
int I2C_ReadReg_16BitAddr(XIicPs *InstancePtr, uint8_t SlaveAddr, uint16_t RegAddr, uint8_t *data,
                          uint32_t len) {
    uint8_t iic_buf[2];
    uint64_t Start_Time;

    iic_buf[0] = RegAddr >> 8 & 0xff;
    iic_buf[1] = RegAddr & 0xff;
    Xil_DCacheFlushRange((INTPTR) iic_buf, 2);
    CHECK_STATUS_RET(XIicPs_MasterSendPolled(InstancePtr, iic_buf, 2, SlaveAddr));
    Start_Time = getTime_millis();
    while (XIicPs_BusIsBusy(InstancePtr))
        if ((getTime_millis() - Start_Time) > 2) return XST_SEND_ERROR;
    CHECK_STATUS_RET(XIicPs_MasterRecvPolled(InstancePtr, data, len, SlaveAddr));
    Start_Time = getTime_millis();
    while (XIicPs_BusIsBusy(InstancePtr) && (getTime_millis() - Start_Time) < 2)
        if ((getTime_millis() - Start_Time) > 2) return XST_RECV_ERROR;
    return XST_SUCCESS;
}

/**
 * @brief
 *
 * @param InstancePtr
 * @param SlaveAddr
 * @param RegAddr
 * @param data
 * @param len
 * @return int
 */
int I2C_WriteReg_16BitAddr(XIicPs *InstancePtr, uint8_t SlaveAddr, uint16_t RegAddr, uint8_t *data,
                           uint32_t len) {
    uint8_t iic_buf[130];
    uint64_t Start_Time;
    if (len > 128) return XST_FAILURE;
    iic_buf[0] = RegAddr >> 8 & 0xff;
    iic_buf[1] = RegAddr & 0xff;
    memcpy(iic_buf + 2, data, len);
    Xil_DCacheFlushRange((INTPTR) iic_buf, len + 2);
    CHECK_STATUS_RET(XIicPs_MasterSendPolled(InstancePtr, iic_buf, len + 2, SlaveAddr));
    Start_Time = getTime_millis();
    while (XIicPs_BusIsBusy(InstancePtr))
        if ((getTime_millis() - Start_Time) > 2) return XST_SEND_ERROR;
    return XST_SUCCESS;
}

/**
 * @brief
 *
 * @param InstancePtr
 * @param SlaveAddr
 * @param RegAddr
 * @param data
 * @param len
 * @return int
 */
int I2C_ReadReg_8BitAddr(XIicPs *InstancePtr, uint8_t SlaveAddr, uint8_t RegAddr, uint8_t *data,
                         uint32_t len) {
    uint64_t Start_Time;
    CHECK_STATUS_RET(XIicPs_MasterSendPolled(InstancePtr, &RegAddr, 1, SlaveAddr));
    Start_Time = getTime_millis();
    while (XIicPs_BusIsBusy(InstancePtr))
        if ((getTime_millis() - Start_Time) > 2) return XST_SEND_ERROR;
    CHECK_STATUS_RET(XIicPs_MasterRecvPolled(InstancePtr, data, len, SlaveAddr));
    Start_Time = getTime_millis();
    while (XIicPs_BusIsBusy(InstancePtr))
        if ((getTime_millis() - Start_Time) > 2) return XST_RECV_ERROR;
    return XST_SUCCESS;
}

/**
 * @brief
 *
 * @param InstancePtr
 * @param SlaveAddr
 * @param RegAddr
 * @param data
 * @param len
 * @return int
 */
int I2C_WriteReg_8BitAddr(XIicPs *InstancePtr, uint8_t SlaveAddr, uint8_t RegAddr, uint8_t *data,
                          uint32_t len) {
    uint8_t iic_buf[129];
    uint64_t Start_Time;
    if (len > 128) return XST_FAILURE;
    iic_buf[0] = RegAddr;
    memcpy(iic_buf + 1, data, len);
    CHECK_STATUS_RET(XIicPs_MasterSendPolled(InstancePtr, iic_buf, len + 1, SlaveAddr));
    Start_Time = getTime_millis();
    while (XIicPs_BusIsBusy(InstancePtr))
        if ((getTime_millis() - Start_Time) > 2) return XST_SEND_ERROR;
    return XST_SUCCESS;
}

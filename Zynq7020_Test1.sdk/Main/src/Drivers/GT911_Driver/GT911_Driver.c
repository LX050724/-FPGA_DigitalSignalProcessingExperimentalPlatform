/*
 * GT911_Driver.c
 *
 *  Created on: 2021年12月30日
 *      Author: yaoji
 */

#ifndef SRC_GT911_DRIVER_GT911_DRIVER_C_
#define SRC_GT911_DRIVER_GT911_DRIVER_C_

#include "GT911_Driver/GT911_Driver.h"

#include "FreeRTOS.h"
#include "sleep.h"
#include "task.h"
#include "utils.h"
#include "xgpiops.h"
#include "xil_cache.h"

/**
 * @brief
 *
 * @param InstancePtr
 * @param DeviceAddr
 */
void GT911_Reset(GT911_Typedef *InstancePtr, uint8_t DeviceAddr) {
    XGpioPs_SetDirectionPin(InstancePtr->GPIOInstancePtr, InstancePtr->INT_PinNumber, 1);
    XGpioPs_SetOutputEnablePin(InstancePtr->GPIOInstancePtr, InstancePtr->INT_PinNumber, 1);
    XGpioPs_SetDirectionPin(InstancePtr->GPIOInstancePtr, InstancePtr->RESET_PinNumber, 1);
    XGpioPs_SetOutputEnablePin(InstancePtr->GPIOInstancePtr, InstancePtr->RESET_PinNumber, 1);
    XGpioPs_WritePin(InstancePtr->GPIOInstancePtr, InstancePtr->RESET_PinNumber, 0);
    XGpioPs_WritePin(InstancePtr->GPIOInstancePtr, InstancePtr->INT_PinNumber, 0);
    vTaskDelay(1);
    if (DeviceAddr == GT911_DEVICE_ADDR_1) {
        XGpioPs_WritePin(InstancePtr->GPIOInstancePtr, InstancePtr->INT_PinNumber, 1);
    }
    vTaskDelay(1);
    XGpioPs_WritePin(InstancePtr->GPIOInstancePtr, InstancePtr->RESET_PinNumber, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
    InstancePtr->DeviceIicAddr = DeviceAddr;
    XGpioPs_SetDirectionPin(InstancePtr->GPIOInstancePtr, InstancePtr->INT_PinNumber, 0);
}

/**
 * @brief
 *
 * @param InstancePtr
 * @param DeviceAddr
 * @return int
 */
int GT911_Init(GT911_Typedef *InstancePtr, uint8_t DeviceAddr) {
    uint8_t buf[5] = {1, 0, 0, 0, 0};
    GT911_Reset(InstancePtr, DeviceAddr);
    CHECK_STATUS_RET(
            I2C_WriteReg_16BitAddr(InstancePtr->I2CInstancePtr, DeviceAddr, 0x8040, buf, 1));
    CHECK_STATUS_RET(
            I2C_ReadReg_16BitAddr(InstancePtr->I2CInstancePtr, DeviceAddr, 0x8140, buf + 1, 4));
    CHECK_STATUS_RET(
            I2C_WriteReg_16BitAddr(InstancePtr->I2CInstancePtr, DeviceAddr, 0x8040, buf, 1));
    xil_printf("GT911_Init: Product ID = %c%c%c%c\r\n", buf[1], buf[2], buf[3], buf[4]);
    CHECK_STATUS_RET(
            I2C_ReadReg_16BitAddr(InstancePtr->I2CInstancePtr, DeviceAddr, 0x8048, buf, 4));
    InstancePtr->X_Max = buf[0] | buf[1] << 8;
    InstancePtr->Y_Max = buf[2] | buf[3] << 8;
    xil_printf("GT911_Init: X_Max=%d, Y_Max=%d\r\n", InstancePtr->X_Max, InstancePtr->Y_Max);
    vTaskDelay(pdMS_TO_TICKS(10));
    return XST_SUCCESS;
}

/**
 * @brief
 *
 * @param InstancePtr
 * @param touchPoint
 * @return uint8_t
 */
uint8_t GT911_ReadPoint(GT911_Typedef *InstancePtr, GT911_TouchPointTypedef *touchPoint) {
    uint8_t status = 0, NumOfTouchPoints = 0;
    if (I2C_ReadReg_16BitAddr(InstancePtr->I2CInstancePtr, InstancePtr->DeviceIicAddr, 0x814E,
                              &status, 1) != XST_SUCCESS)
        return 0;
    if (status & 0x80) {
        NumOfTouchPoints = status & 0x0f;
        touchPoint->num = NumOfTouchPoints;
        if (I2C_ReadReg_16BitAddr(InstancePtr->I2CInstancePtr, InstancePtr->DeviceIicAddr, 0x814F,
                                  (uint8_t *) &touchPoint->touchPoints,
                                  NumOfTouchPoints * 8) != XST_SUCCESS)
            return 0;
    } else
        touchPoint->num = 0;
    status = 0;
    I2C_WriteReg_16BitAddr(InstancePtr->I2CInstancePtr, InstancePtr->DeviceIicAddr, 0x814E,
                           (uint8_t *) &status, 1);
    return touchPoint->num;
}

#endif /* SRC_GT911_DRIVER_GT911_DRIVER_C_ */

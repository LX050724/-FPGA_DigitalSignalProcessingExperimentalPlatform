/*
 * GT911_Driver.h
 *
 *  Created on: 2021年12月30日
 *      Author: yaoji
 */

#include "I2C_Driver/I2C_Driver.h"
#include "xgpiops.h"

#define GT911_DEVICE_ADDR_1 0x14
#define GT911_DEVICE_ADDR_2 0x5D

typedef struct {
    XIicPs *I2CInstancePtr;
    XGpioPs *GPIOInstancePtr;
    uint32_t RESET_PinNumber;
    uint32_t INT_PinNumber;
    uint8_t DeviceIicAddr;
    uint16_t X_Max, Y_Max;
} GT911_Typedef;

typedef struct {
    uint8_t num;
    struct TouchPoints {
        uint8_t trackId;
        uint16_t x;
        uint16_t y;
        uint16_t size;
        uint8_t Reserved;
    } __attribute__((packed)) touchPoints[5];
} GT911_TouchPointTypedef;

int GT911_Init(GT911_Typedef *InstancePtr, uint8_t DeviceAddr);

uint8_t GT911_ReadPoint(GT911_Typedef *InstancePtr, GT911_TouchPointTypedef *touchPoint);

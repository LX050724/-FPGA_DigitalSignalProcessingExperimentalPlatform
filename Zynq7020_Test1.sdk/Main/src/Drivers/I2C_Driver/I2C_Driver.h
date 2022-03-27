/*
 * I2C_Driver.h
 *
 *  Created on: 2021年12月30日
 *      Author: yaoji
 */

#ifndef SRC_I2C_DRIVER_I2C_DRIVER_H_
#define SRC_I2C_DRIVER_I2C_DRIVER_H_

#include "xiicps.h"

int I2C_Init(XIicPs *InstancePtr, uint16_t DeviceId, uint32_t FsclHz);

int I2C_ReadReg_16BitAddr(XIicPs *InstancePtr, uint8_t SlaveAddr, uint16_t RegAddr, uint8_t *data,
                          uint32_t len);
int I2C_WriteReg_16BitAddr(XIicPs *InstancePtr, uint8_t SlaveAddr, uint16_t RegAddr, uint8_t *data,
                           uint32_t len);
int I2C_ReadReg_8BitAddr(XIicPs *InstancePtr, uint8_t SlaveAddr, uint8_t RegAddr, uint8_t *data,
                         uint32_t len);
int I2C_WriteReg_8BitAddr(XIicPs *InstancePtr, uint8_t SlaveAddr, uint8_t RegAddr, uint8_t *data,
                          uint32_t len);

#endif /* SRC_I2C_DRIVER_I2C_DRIVER_H_ */

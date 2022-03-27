/*
 * DS1337_Driver.h
 *
 *  Created on: 2021年12月30日
 *      Author: yaoji
 */

#ifndef SRC_DS1337_DRIVER_DS1337_DRIVER_H_
#define SRC_DS1337_DRIVER_DS1337_DRIVER_H_

#include "I2C_Driver/I2C_Driver.h"
#include "time.h"

#define DS1337_I2C_ADDRESS 0x68

void DS1337_GetTime(XIicPs *InstancePtr, struct tm *t);
void DS1337_SetTime(XIicPs *InstancePtr, struct tm *t);
const char *DS1337_WeekStr(uint8_t week);
void DS1337_SetDefaultInstance(XIicPs *InstancePtr);

#endif /* SRC_DS1337_DRIVER_DS1337_DRIVER_H_ */

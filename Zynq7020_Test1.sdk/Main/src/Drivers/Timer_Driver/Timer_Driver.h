/*
 * Timer_Driver.h
 *
 *  Created on: 2021年12月26日
 *      Author: yaoji
 */

#ifndef SRC_TIMER_DRIVER_TIMER_DRIVER_H_
#define SRC_TIMER_DRIVER_TIMER_DRIVER_H_

#include "xil_types.h"

#ifndef __USE_RTOS  // RTOS占用定时器，使用RTOS禁用定时器

int Timer_init(uint32_t period_ms);

int Timer_Setup_Intr_System(uint8_t Priority, void* Handler);

void Timer_start();

#endif

uint64_t getTime_millis();

#endif /* SRC_TIMER_DRIVER_TIMER_DRIVER_H_ */

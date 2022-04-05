/*
 * my_lvgl_init.h
 *
 *  Created on: 2021年12月25日
 *      Author: yaoji
 */

#ifndef SRC_LVGL_ZYNQ_INIT_ZYNQ_LVGL_INIT_H_
#define SRC_LVGL_ZYNQ_INIT_ZYNQ_LVGL_INIT_H_

#include "lvgl.h"
#include "xgpiops.h"
#include "xiicps.h"

#ifdef __USE_RTOS
#include "FreeRTOS.h"
#include "semphr.h"
extern xSemaphoreHandle LVGL_Mutex;
#endif

void zynq_lvgl_init(XIicPs *_iic, XGpioPs *_gpio);

void zynq_disp_flush_ready(void *);


#endif /* SRC_LVGL_ZYNQ_INIT_ZYNQ_LVGL_INIT_H_ */

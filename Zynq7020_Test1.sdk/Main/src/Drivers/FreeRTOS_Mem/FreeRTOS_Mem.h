/*
 * FreeRTOS_Mem.h
 *
 *  Created on: 2021年12月31日
 *      Author: yaoji
 */

#ifndef SRC_DRIVERS_FREERTOS_MEM_FREERTOS_MEM_H_
#define SRC_DRIVERS_FREERTOS_MEM_FREERTOS_MEM_H_

#include "FreeRTOS.h"
#include <stdint.h>
#include <stddef.h>

void *os_malloc(size_t __size);

void *os_realloc(void *__r, size_t __size);

void os_free(void *__r);

void os_DCacheInvalidateRange(void *adr, uint32_t len);

void os_DCacheFlushRange(void *adr, uint32_t len);

#endif /* SRC_DRIVERS_FREERTOS_MEM_FREERTOS_MEM_H_ */

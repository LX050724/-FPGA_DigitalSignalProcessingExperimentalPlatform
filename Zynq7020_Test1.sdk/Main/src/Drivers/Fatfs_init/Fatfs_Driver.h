/*
 * Fatfs_Driver.h
 *
 *  Created on: 2021年12月29日
 *      Author: yaoji
 */

#include "ff.h"
#include "FreeRTOS_Mem/FreeRTOS_Mem.h"

#define SD_INDEX 0
#define EMMC_INDEX 1

int Fatfs_Init();
int Fatfs_GetVolSize(const char *path, FSIZE_t *total_size, FSIZE_t *free_size);
FRESULT Fatfs_GetMountStatus(int index);

char *UTF8_TO_GBK(const char *utf8_str);
char *GBK_TO_UTF8(const char *gbk_str);
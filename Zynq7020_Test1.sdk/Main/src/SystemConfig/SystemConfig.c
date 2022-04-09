#include "SystemConfig.h"

#include "Fatfs_init/Fatfs_Driver.h"
#include "Flash_Driver/qspi_g128_flash.h"
#include "FreeRTOS.h"
#include "FreeRTOS_Mem/FreeRTOS_Mem.h"
#include "cJSON.h"
#include "semphr.h"
#include "task.h"
#include "check.h"
#include "xil_io.h"
#include <Timer_Driver/Timer_Driver.h>
#include <stdlib.h>
#include <stdio.h>

static const char *path;

BootMod_t GetBootMod() {
    uint32_t BootModeRegister = Xil_In32(BOOT_MODE_REG);
    BootModeRegister &= BOOT_MODES_MASK;
    return BootModeRegister;
}

int SystemConfig_Init() {
    /* 判断系统启动方式 */
    BootMod_t bootmod = GetBootMod();

    if (bootmod == QSPI_MODE) {
        xil_printf("Boot mod is QSPI");
    } else if (bootmod == SD_MODE) {
        xil_printf("Boot mod is SD");
    }
}

static void ProgramFLASH_task(void *param) {
    FRESULT res;
    FIL bin_file;
    int start_time = getTime_millis();
    res = f_open(&bin_file, path, FA_READ);
    if (res != FR_OK) {
        xil_printf("open file %s failed res=%d\r\n", param, res);
    }

    FSIZE_t size = f_size(&bin_file);
    xil_printf("read file size = %d\r\n", size);
    uint8_t *buf1 = os_malloc(size);
    uint8_t *buf2 = os_malloc(size + 16);
    uint8_t *buf3 = os_malloc(size + 16);
    if (buf1 == NULL || buf2 == NULL || buf3 == NULL) {
        xil_printf("error malloc failed\r\n");
        goto end;
    }
    UINT rdsize;
    f_read(&bin_file, buf1, size, &rdsize);
    if (rdsize != size) {
        xil_printf("error ");
    }
    xil_printf("size = %d, rdsize = %d\r\n", (int) size, (int) rdsize);

    update_flash(buf1, buf2, buf3, size);
    xil_printf("end used time %dms\r\n", (int) (getTime_millis() - start_time));

    end:
    path = NULL;
    os_free(buf1);
    os_free(buf2);
    os_free(buf3);
    f_close(&bin_file);
    vTaskDelete(NULL);
}

/**
 * @brief 创建FLASH编程任务，非阻塞函数
 *
 * @param bootFile 文件路径，必须是静态内存
 * @return int 成功创建任务返回XST_SUCCESS
 *             如果正在有编程任务进行(path != NULL)或创建失败则返回XST_FAILURE
 */
int ProgramFLASH(const char *bootFile) {
    if (path != NULL) return XST_FAILURE;
    TaskHandle_t handel;
    path = bootFile;
    if (xTaskCreate(ProgramFLASH_task, "ProgFLASH", 1024, NULL, 1, &handel) != pdPASS) {
        xil_printf("error create ProgramFLASH task failed\r\n");
        return XST_FAILURE;
    }
    return XST_SUCCESS;
}

/**
 * @brief 判断任务是否完成
 * 
 * @return int 
 */
int ProgramFLASH_isfinished() { return path == NULL; }


static char FirmwareVersionStr[] = "000000000000";

static void Get_Compile_Date_Base(uint8_t *Year, uint8_t *Month, uint8_t *Day) {
    const char *pMonth[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    const char Date[] = __DATE__;
    uint8_t i;
    for (i = 0; i < 12; i++) {
        if (memcmp(Date, pMonth[i], 3) == 0) {
            *Month = i + 1;
            break;
        }
    }
    *Year = (uint8_t) atoi(Date + 9);
    *Day = (uint8_t) atoi(Date + 4);
}

static void get_Compile_Time_Base(uint8_t *Hour, uint8_t *Min, uint8_t *Sec) {
	const char Time[] = __TIME__;
    *Hour = (uint8_t) atoi(Time);
    *Min = (uint8_t) atoi(Time + 3);
    *Sec = (uint8_t) atoi(Time + 6);
}

const char *getFirmwareVersion() {
    if (FirmwareVersionStr[0] == '0') {
        uint8_t Year, Month, Day, Hour, Min, Sec;
        Get_Compile_Date_Base(&Year, &Month, &Day);
        get_Compile_Time_Base(&Hour, &Min, &Sec);
        sprintf(FirmwareVersionStr, "%04d%02d%02d%02d%02d", Year + 2000, Month, Day, Hour, Min);
    }
    return FirmwareVersionStr;
}

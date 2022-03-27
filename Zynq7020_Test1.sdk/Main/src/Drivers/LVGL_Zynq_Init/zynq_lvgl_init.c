/*
 * my_lvgl_init.c
 *
 *  Created on: 2021骞�12鏈�25鏃�
 *      Author: yaoji
 */

#include "LVGL_Zynq_Init/zynq_lvgl_init.h"

#include "Fatfs_init/Fatfs_Driver.h"
#include "FreeRTOS_Mem/FreeRTOS_Mem.h"
#include "GT911_Driver/GT911_Driver.h"
#include "VDMA_Driver/VDMA_Driver.h"
#include "task.h"
#include "xil_cache.h"
#include "utils.h"

static lv_indev_drv_t touch_drv;
static lv_indev_t *indev_touchpad = NULL;

static lv_disp_drv_t disp_drv;
static lv_disp_draw_buf_t disp_draw_buf;
static lv_disp_t *disp = NULL;

static GT911_Typedef gt911;

static void zynq_flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
static void zynq_monitor_cb(lv_disp_drv_t *disp_drv, uint32_t time, uint32_t px);
static void zynq_lv_log_print(const char *buf);
static void zynq_touch_read(struct _lv_indev_drv_t *indev_drv, lv_indev_data_t *data);

#ifdef __USE_RTOS
xSemaphoreHandle LVGL_Mutex = NULL;
static TaskHandle_t rtos_TaskHandle;

static void zynq_lv_timerTask(void *pvParameters) {
    LVGL_Mutex = xSemaphoreCreateMutex();
    CHECK_FATAL_ERROR(LVGL_Mutex == NULL);

    portTickType xLastExecutionTime = xTaskGetTickCount();
    for (;;) {
        xSemaphoreTake(LVGL_Mutex, portMAX_DELAY);
        lv_timer_handler();
        xSemaphoreGive(LVGL_Mutex);
        vTaskDelayUntil(&xLastExecutionTime, pdMS_TO_TICKS(75));
    }
}

#endif

void zynq_lvgl_init(XIicPs *iic, XGpioPs *gpio) {
    lv_init();
    lv_img_cache_set_size(1);
    lv_log_register_print_cb(zynq_lv_log_print);

    if (Fatfs_GetMountStatus(SD_INDEX) == FR_OK)
        lv_user_font_load("0:/LVGL_FONT");
    else if (Fatfs_GetMountStatus(EMMC_INDEX) == FR_OK)
        lv_user_font_load("1:/LVGL_FONT");
    else
        LV_LOG_ERROR("没有挂载有效的存储设备, 无法加载字体");

    lv_disp_drv_init(&disp_drv);
    lv_disp_draw_buf_init(&disp_draw_buf, GRAM0, GRAM1, VDMA_H_ACTIVE * VDMA_V_ACTIVE);

    disp_drv.hor_res = VDMA_H_ACTIVE;
    disp_drv.ver_res = VDMA_V_ACTIVE;
    disp_drv.physical_hor_res = -1;
    disp_drv.physical_ver_res = -1;
    disp_drv.offset_x = 0;
    disp_drv.offset_y = 0;
    disp_drv.draw_buf = &disp_draw_buf;
    disp_drv.direct_mode = TRUE;
    disp_drv.full_refresh = TRUE;
    disp_drv.flush_cb = zynq_flush_cb;
    //	disp_drv.monitor_cb = zynq_monitor_cb;

    disp = lv_disp_drv_register(&disp_drv);

    gt911.GPIOInstancePtr = gpio;
    gt911.I2CInstancePtr = iic;
    gt911.INT_PinNumber = 56;
    gt911.RESET_PinNumber = 57;
    GT911_Init(&gt911, GT911_DEVICE_ADDR_1);

    lv_indev_drv_init(&touch_drv);
    touch_drv.disp = disp;
    touch_drv.type = LV_INDEV_TYPE_POINTER;
    touch_drv.read_cb = zynq_touch_read;
    indev_touchpad = lv_indev_drv_register(&touch_drv);
    LV_UNUSED(indev_touchpad);

#ifdef __USE_RTOS
    xTaskCreate(zynq_lv_timerTask, "LVGL Task", 1024, NULL, 1, &rtos_TaskHandle);
#endif
}

static void zynq_flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
    Xil_DCacheFlushRange((INTPTR) color_p, VDMA_BUFFER_SIZE);
    VDMA_SetBufferIndex((void *) color_p == (void *) GRAM0 ? 0 : 1);
}

static void zynq_monitor_cb(lv_disp_drv_t *disp_drv, uint32_t time, uint32_t px) {
    lv_mem_monitor_t mon;
    lv_mem_monitor(&mon);
    xil_printf("refreshed in %3d ms, memory used %dbyte %3d%%\r\n", time, mon.used_cnt,
               mon.used_pct);
}

void zynq_disp_flush_ready(void *unused) {
    LV_UNUSED(unused);
    lv_disp_flush_ready(&disp_drv);
}

static void zynq_lv_log_print(const char *buf) {
    char *gbk = UTF8_TO_GBK(buf);
    print(gbk);
    print("\r\n");
    os_free(gbk);
}

static void zynq_touch_read(struct _lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {
    static lv_coord_t last_x = 0;
    static lv_coord_t last_y = 0;
    GT911_TouchPointTypedef t = {0};

    /*Save the pressed coordinates and the state*/
    if (GT911_ReadPoint(&gt911, &t) > 0) {
        if (gt911.X_Max != VDMA_H_ACTIVE || gt911.Y_Max != VDMA_V_ACTIVE) {
            last_x = t.touchPoints[0].x * VDMA_H_ACTIVE / gt911.X_Max;
            last_y = t.touchPoints[0].y * VDMA_V_ACTIVE / gt911.Y_Max;
        } else {
            last_x = t.touchPoints[0].x;
            last_y = t.touchPoints[0].y;
        }
        data->continue_reading = FALSE;
        data->state = LV_INDEV_STATE_PR;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }

    /*Set the last pressed coordinates*/
    data->point.x = last_x;
    data->point.y = last_y;
}

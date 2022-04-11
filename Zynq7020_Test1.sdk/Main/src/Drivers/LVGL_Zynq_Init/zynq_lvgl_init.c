/*
 * my_lvgl_init.c
 *
 *  Created on: 2021年12月25日
 *      Author: yaoji
 */

#include "LVGL_Zynq_Init/zynq_lvgl_init.h"

#include "Fatfs_init/Fatfs_Driver.h"
#include "FreeRTOS_Mem/FreeRTOS_Mem.h"
#include "GT911_Driver/GT911_Driver.h"
#include "VDMA_Driver/VDMA_Driver.h"
#include "task.h"
#include "xil_cache.h"
#include "check.h"
#include "main.h"
#include "zynq_lvgl_snapshot.h"

static lv_indev_drv_t touch_drv;
static lv_indev_drv_t btn_drv;

static GT911_Typedef gt911;

static lv_indev_t *indev_touchpad = NULL;
static lv_disp_drv_t disp_drv;
static lv_disp_draw_buf_t disp_draw_buf;
static lv_disp_t *disp = NULL;

static void zynq_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p);
static void zynq_monitor_cb(lv_disp_drv_t *drv, uint32_t time, uint32_t px);
static void zynq_lv_log_print(const char *buf);
static void zynq_touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data);
static void zynq_btn_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);

#ifdef __USE_RTOS
xSemaphoreHandle LVGL_Mutex = NULL;
static TaskHandle_t rtos_TaskHandle;

static void zynq_lv_timerTask(void *pvParameters) {
    portTickType xLastExecutionTime = xTaskGetTickCount();
    for (;;) {
        xSemaphoreTake(LVGL_Mutex, portMAX_DELAY);
        lv_timer_handler();
        xSemaphoreGive(LVGL_Mutex);
        if (xSemaphoreTake(key_handle, 0) == pdTRUE) {
            zynq_lvgl_snapshot(disp_draw_buf.buf_act);
        }
        vTaskDelayUntil(&xLastExecutionTime, pdMS_TO_TICKS(75));
    }
}

#endif

int zynq_lvgl_init(XIicPs *_iic, XGpioPs *_gpio) {
    int ret = XST_SUCCESS;
    LVGL_Mutex = xSemaphoreCreateMutex();
    CHECK_FATAL_ERROR(LVGL_Mutex == NULL);

    lv_init();
    lv_img_cache_set_size(1);
    lv_log_register_print_cb(zynq_lv_log_print);

    if (Fatfs_GetMountStatus(SD_INDEX) == FR_OK) {
        if (lv_user_font_load("0:/LVGL_FONT") != XST_SUCCESS) ret = XST_FAILURE;
    } else if (Fatfs_GetMountStatus(EMMC_INDEX) == FR_OK) {
        if (lv_user_font_load("1:/LVGL_FONT") != XST_SUCCESS) ret = XST_FAILURE;
    } else {
        LV_LOG_ERROR("没有挂载有效的存储设备, 无法加载字体");
        ret = XST_FAILURE;
    }

    // 字体加载成功切换默认字体
    if (ret == XST_SUCCESS) defualt_font = &msyhl_16;

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

    gt911.GPIOInstancePtr = _gpio;
    gt911.I2CInstancePtr = _iic;
    gt911.INT_PinNumber = GT911_RST_GPIO_PIN;
    gt911.RESET_PinNumber = GT911_INT_GPIO_PIN;
    GT911_Init(&gt911, GT911_DEVICE_ADDR_1);

    lv_indev_drv_init(&touch_drv);
    touch_drv.disp = disp;
    touch_drv.type = LV_INDEV_TYPE_POINTER;
    touch_drv.read_cb = zynq_touch_read;
    indev_touchpad = lv_indev_drv_register(&touch_drv);
    LV_UNUSED(indev_touchpad);

//    由于LVGL将按键当成触摸屏的点击，所以这里的按键没有任何卵用
//    XGpioPs_SetDirectionPin(_gpio, BTN_GPIO_PIN, GPIO_DIR_INPUT);
//    lv_indev_drv_init(&btn_drv);
//    btn_drv.disp = disp;
//    btn_drv.type = LV_INDEV_TYPE_BUTTON;
//    btn_drv.read_cb = zynq_btn_read;
//    btn_drv.user_data = _gpio;
//    lv_indev_drv_register(&btn_drv);

#ifdef __USE_RTOS
    xTaskCreate(zynq_lv_timerTask, "LVGL Task", 2048,
                NULL, 5, &rtos_TaskHandle);
#endif
    return ret;
}

static void zynq_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p) {
    os_DCacheFlushRange(color_p, VDMA_BUFFER_SIZE);
    VDMA_SetBufferIndex((void *) color_p == (void *) GRAM0 ? 0 : 1);
}

static void zynq_monitor_cb(lv_disp_drv_t *drv, uint32_t time, uint32_t px) {
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
#ifdef PRINT_ENCODE_GBK
    char *gbk = UTF8_TO_GBK(buf);
    print(gbk);
    os_free(gbk);
#else
    print(buf);
#endif
    outbyte('\r');
}

static void zynq_btn_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {
    data->state = XGpioPs_ReadPin(indev_drv->user_data, BTN_GPIO_PIN) ?
                  LV_INDEV_STATE_RELEASED : LV_INDEV_STATE_PRESSED;
}


static void zynq_touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data) {
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
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }

    /*Set the last pressed coordinates*/
    data->point.x = last_x;
    data->point.y = last_y;
}

//
// Created by yaoji on 2022/4/11.
//

#include "zynq_lvgl_snapshot.h"
#include "LVGL_Utils/MessageBox.h"
#include "lvgl.h"
#include "task.h"
#include "queue.h"
#include "FreeRTOS_Mem/FreeRTOS_Mem.h"
#include "VDMA_Driver/VDMA_Driver.h"
#include "BMP_encoder/bmp_encoder.h"
#include "DS1337_Driver/DS1337_Driver.h"
#include "ff.h"
#include "zynq_lvgl_init.h"

#pragma pack(1)
typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} lv_color24_t;
#pragma pack()

static QueueHandle_t snapshot_queue;
static TaskHandle_t snapshot_task_handle;

static void zynq_lvgl_snapshot_task(void *p) {
    lv_color_t *screen;


    for (;;) {
        if (xQueuePeek(snapshot_queue, &screen, portMAX_DELAY) == pdTRUE) {

            char filename[64] = {0};
            FILINFO file_info;
            FRESULT res = f_stat("0:/ScreenShot", &file_info);
            if (res != FR_OK) {
                if (res == FR_NO_FILE) res = f_mkdir("0:/ScreenShot");
                if (res != FR_OK) vTaskDelete(NULL);
            }
            struct tm t;
            int index = 0;
            xSemaphoreTake(LVGL_Mutex, portMAX_DELAY);
            lv_obj_t *messagebox = MessageBox_wait("请稍等", "正在保存截图");
            xSemaphoreGive(LVGL_Mutex);
            DS1337_GetTime(NULL, &t);
            do {
                sprintf(filename, "0:/ScreenShot/%04d-%02d-%02d_%02d-%02d-%02d_%d.bmp",
                        t.tm_year + 1900, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, index);
                res = f_stat(filename, &file_info);
                if (res == FR_OK) index++;
                else if (res == FR_NO_FILE) break;
                else {
                    lv_msgbox_close(messagebox);
                    goto err;
                }
            } while (1);
            int save_res = bmp_save(VDMA_H_ACTIVE, VDMA_V_ACTIVE, screen, filename);
            xSemaphoreTake(LVGL_Mutex, portMAX_DELAY);
            lv_msgbox_close(messagebox);
            if (save_res == XST_SUCCESS) {
                MessageBox_info("屏幕截图", "OK", "屏幕截图保存至 %s", filename);
                LV_LOG_INFO("save screen shot %s", filename);
            } else {
                MessageBox_info("屏幕截图", "OK", "屏幕截图保存失败");
                LV_LOG_ERROR("save screen shot error");
            }
            xSemaphoreGive(LVGL_Mutex);
            err:
            xQueueReceive(snapshot_queue, &screen, portMAX_DELAY);
            os_free(screen);
        }
    }
}

BaseType_t zynq_lvgl_snapshot(lv_color_t *gram) {
    // 没有加载中文字体不开起截图功能
    if (defualt_font != &msyhl_16) return pdFAIL;
    if (snapshot_task_handle == NULL) {
        snapshot_queue = xQueueCreate(1, sizeof(lv_img_dsc_t *));
        configASSERT(snapshot_queue);
        TaskStatus_t status;
        vTaskGetInfo(NULL, &status, pdFALSE, eInvalid);
        configASSERT(xTaskCreate(zynq_lvgl_snapshot_task, "snapshot", 512,
                                 NULL, status.uxCurrentPriority, &snapshot_task_handle));
    }
    lv_color_t *screen = os_malloc(VDMA_BUFFER_SIZE);
    LV_ASSERT_MALLOC(screen);
    memcpy(screen, gram, VDMA_BUFFER_SIZE);
    return xQueueSendToBack(snapshot_queue, &screen, 0);
}

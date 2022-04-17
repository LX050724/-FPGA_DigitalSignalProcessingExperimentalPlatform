#include "Setup.h"

#include "Fatfs_init/Fatfs_Driver.h"
#include "DS1337_Driver/DS1337_Driver.h"
#include "LVGL_Utils/MessageBox.h"
#include "SystemConfig/SystemConfig.h"
#include "LwIP_apps/sntp/sntp_user.h"
#include "cJSON.h"
#include "check.h"
#include "lwip/netif.h"
#include "LwIP_init/LwIP_init.h"
#include "XADC_Driver/XADC_Driver.h"
#include "Controller/SPU_Controller.h"
#include "main.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "Controller/ADC_Controller.h"
#include "Controller/DAC_Controller.h"

static lv_style_t style_title, style_sec_title, style_content;

static lv_obj_t *sd_info;
static lv_obj_t *emmc_info;
static lv_obj_t *time_info;
static lv_obj_t *net_info;
static lv_obj_t *sensor_info;


static void refresh_timer_cb(lv_timer_t *timer);
static void calibration_time_btn_cb(lv_event_t *e);
static void wait_timer_cb(lv_timer_t *timer);
static void flash_MsgBox_event_cb(uint16_t index, void *userdata);
static void flash_btn_event_cb(lv_event_t *e);

static void signal_dropdown_cb(lv_event_t *event);

void Setup_create(lv_obj_t *parent) {
    const char *boot_str[] = {
            "JTAG_MODE", "QSPI_MODE", "NOR_FLASH_MODE", "", "NAND_FLASH_MODE", "SD_MODE", "MMC_MODE",
    };

    lv_style_init(&style_title);
    lv_style_set_text_color(&style_title, lv_palette_main(LV_PALETTE_LIGHT_BLUE));
    lv_style_set_text_line_space(&style_title, 3);
    lv_style_set_text_font(&style_title, &msyhl_24);

    lv_style_init(&style_sec_title);
    lv_style_set_pad_left(&style_sec_title, 15);
    lv_style_set_text_color(&style_sec_title, lv_palette_main(LV_PALETTE_LIGHT_BLUE));

    lv_style_init(&style_content);
    lv_style_set_pad_left(&style_content, 30);

    lv_obj_t *signal_select_title = lv_label_create(parent);
    lv_obj_add_style(signal_select_title, &style_title, 0);
    lv_label_set_text_static(signal_select_title, "信号源选择");

    /* 信号源选择容器 */
    static lv_coord_t col_dsc[] = {140, 140, 140, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] = {50, 50, LV_GRID_TEMPLATE_LAST};
    lv_obj_t *signal_select_cont = lv_obj_create(parent);
    lv_obj_set_style_grid_column_dsc_array(signal_select_cont, col_dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(signal_select_cont, row_dsc, 0);
    lv_obj_set_size(signal_select_cont, LV_HOR_RES * 0.5, LV_VER_RES * 0.3);
    lv_obj_set_layout(signal_select_cont, LV_LAYOUT_GRID);
    lv_obj_add_style(signal_select_cont, &style_content, 0);
    lv_obj_align_to(signal_select_cont, signal_select_title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);


    lv_obj_t *scope_drop_label = lv_label_create(signal_select_cont);
    lv_label_set_text_static(scope_drop_label, "示波器信号源:");
    lv_obj_set_grid_cell(scope_drop_label, LV_GRID_ALIGN_STRETCH, 0, 1,
                         LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_t *scope_drop = lv_dropdown_create(signal_select_cont);
    lv_dropdown_set_options_static(scope_drop, "ADC\nFIR");
    lv_obj_add_event_cb(scope_drop, signal_dropdown_cb, LV_EVENT_VALUE_CHANGED, (void *) CHANNEL_INDEX_SCOPE);
    lv_obj_set_grid_cell(scope_drop, LV_GRID_ALIGN_STRETCH, 0, 1,
                         LV_GRID_ALIGN_STRETCH, 1, 1);

    lv_obj_t *fft_drop_label = lv_label_create(signal_select_cont);
    lv_label_set_text_static(fft_drop_label, "频谱仪信号源:");
    lv_obj_set_grid_cell(fft_drop_label, LV_GRID_ALIGN_STRETCH, 1, 1,
                         LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_t *fft_drop = lv_dropdown_create(signal_select_cont);
    lv_dropdown_set_options_static(fft_drop, "ADC\nFIR");
    lv_obj_add_event_cb(fft_drop, signal_dropdown_cb, LV_EVENT_VALUE_CHANGED, (void *) CHANNEL_INDEX_FFT);
    lv_obj_set_grid_cell(fft_drop, LV_GRID_ALIGN_STRETCH, 1, 1,
                         LV_GRID_ALIGN_STRETCH, 1, 1);

    lv_obj_t *dac_drop_label = lv_label_create(signal_select_cont);
    lv_label_set_text_static(dac_drop_label, "DAC信号源:");
    lv_obj_set_grid_cell(dac_drop_label, LV_GRID_ALIGN_STRETCH, 2, 1,
                         LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_t *dac_drop = lv_dropdown_create(signal_select_cont);
    lv_dropdown_set_options_static(dac_drop, "DDS\nFIR");
    lv_obj_add_event_cb(dac_drop, signal_dropdown_cb, LV_EVENT_VALUE_CHANGED, (void *) CHANNEL_INDEX_DAC);
    lv_obj_set_grid_cell(dac_drop, LV_GRID_ALIGN_STRETCH, 2, 1,
                         LV_GRID_ALIGN_STRETCH, 1, 1);
    /* 信号源选择容器 END */

    lv_obj_t *fw_title = lv_label_create(parent);
    lv_obj_add_style(fw_title, &style_title, 0);
    lv_label_set_text_static(fw_title, "固件信息");
    lv_obj_t *fw_info = lv_label_create(parent);
    lv_obj_align_to(fw_title, signal_select_cont, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);

    lv_label_set_text_fmt(fw_info,
                          "FreeRTOS版本: V10.0.0\n"
                          "LVGL版本: %d.%d.%d\n"
                          "cJSON版本: %d.%d.%d\n"
                          "固件编译日期: %s\n"
                          "启动方式: %s\n",
                          lv_version_major(), lv_version_minor(), lv_version_patch(),
                          CJSON_VERSION_MAJOR, CJSON_VERSION_MINOR, CJSON_VERSION_PATCH, __DATE__,
                          boot_str[GetBootMod()]);
    lv_obj_align_to(fw_info, fw_title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    lv_obj_add_style(fw_info, &style_content, 0);

    lv_obj_t *flash_btn = lv_btn_create(parent);
    lv_label_set_text_static(lv_label_create(flash_btn), "更新FLASH固件");
    lv_obj_add_event_cb(flash_btn, flash_btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_align_to(flash_btn, fw_info, LV_ALIGN_OUT_RIGHT_TOP, 30, -10);

    lv_obj_t *fs_title = lv_label_create(parent);
    lv_obj_add_style(fs_title, &style_title, 0);
    lv_label_set_text_static(fs_title, "文件系统信息");
    lv_obj_align_to(fs_title, fw_info, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);

    lv_obj_t *sd_title = lv_label_create(parent);
    lv_label_set_text_static(sd_title, "SD Card:");
    lv_obj_add_style(sd_title, &style_sec_title, 0);
    lv_obj_align_to(sd_title, fs_title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    sd_info = lv_label_create(parent);
    lv_obj_add_style(sd_info, &style_content, 0);
    lv_obj_align_to(sd_info, sd_title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);


    lv_obj_t *emmc_title = lv_label_create(parent);
    lv_label_set_text_static(emmc_title, "EMMC:");
    lv_obj_add_style(emmc_title, &style_sec_title, 0);
    lv_obj_align_to(emmc_title, sd_info, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    emmc_info = lv_label_create(parent);
    lv_obj_add_style(emmc_info, &style_content, 0);
    lv_obj_align_to(emmc_info, emmc_title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    lv_obj_t *time_title = lv_label_create(parent);
    lv_label_set_text_static(time_title, "时间设置");
    lv_obj_add_style(time_title, &style_title, 0);
    lv_obj_align_to(time_title, emmc_info, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);

    time_info = lv_label_create(parent);
    lv_obj_align_to(time_info, time_title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    lv_obj_add_style(time_info, &style_content, 0);
    lv_label_set_text_static(time_info, "当前时间: 00:00:00 2022年12月30日 星期四");

    lv_obj_t *calibration_time_btn = lv_btn_create(parent);
    lv_obj_t *calibration_time_btn_label = lv_label_create(calibration_time_btn);
    lv_label_set_text_static(calibration_time_btn_label, "联网自动校准");
    lv_obj_add_event_cb(calibration_time_btn, calibration_time_btn_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_align_to(calibration_time_btn, time_info, LV_ALIGN_OUT_RIGHT_TOP, 30, -10);

    lv_obj_t *net_title = lv_label_create(parent);
    lv_label_set_text_static(net_title, "网络信息");
    lv_obj_add_style(net_title, &style_title, 0);
    lv_obj_align_to(net_title, time_info, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);

    net_info = lv_label_create(parent);
    lv_label_set_text_static(net_info, "\n\n\n\n\n");
    lv_obj_add_style(net_info, &style_content, 0);
    lv_obj_align_to(net_info, net_title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    lv_obj_t *sensor_title = lv_label_create(parent);
    lv_label_set_text_static(sensor_title, "传感器信息");
    lv_obj_add_style(sensor_title, &style_title, 0);
    lv_obj_align_to(sensor_title, net_info, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);

    sensor_info = lv_table_create(parent);
    lv_table_set_col_cnt(sensor_info, 4);
    lv_table_set_row_cnt(sensor_info, 8);
    lv_table_set_cell_value(sensor_info, 0, 1, "当前");
    lv_table_set_cell_value(sensor_info, 0, 2, "最大");
    lv_table_set_cell_value(sensor_info, 0, 3, "最小");
    lv_table_set_cell_value(sensor_info, 1, 0, "温度");
    lv_table_set_cell_value(sensor_info, 2, 0, "VCCINT");
    lv_table_set_cell_value(sensor_info, 3, 0, "VCCAUX");
    lv_table_set_cell_value(sensor_info, 4, 0, "VCCBRAM");
    lv_table_set_cell_value(sensor_info, 5, 0, "VCCPINT");
    lv_table_set_cell_value(sensor_info, 6, 0, "VCCPAUX");
    lv_table_set_cell_value(sensor_info, 7, 0, "VCCO_DDR");
    lv_obj_align_to(sensor_info, sensor_title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    lv_timer_create(refresh_timer_cb, 300, parent);
}

static void wait_timer_cb(lv_timer_t *timer) {
    if (ProgramFLASH_isfinished()) {
        lv_msgbox_close(timer->user_data);
        lv_timer_del(timer);
        MessageBox_info("完成", "关闭", "FLASH写入完成");
    }
}

static void flash_MsgBox_event_cb(uint16_t index, void *userdata) {
    LV_UNUSED(userdata);
    if (index != 0) return;
    if (ProgramFLASH("0:/BOOT.BIN") != XST_SUCCESS) {
        LV_LOG_ERROR("FLASH编程错误");
        MessageBox_info("错误", "关闭", "创建FLASH编程任务失败");
    } else {
        lv_obj_t *messagebox = MessageBox_wait("请稍等", "正在写入FLASH . . .");
        lv_timer_create(wait_timer_cb, 500, messagebox);
    }
}

static void flash_btn_event_cb(lv_event_t *e) {
    LV_UNUSED(e);
    if (Fatfs_GetMountStatus(0) == FR_OK) {
        MessageBox_question(
                "写入固件",
                "是", "否", flash_MsgBox_event_cb, NULL,
                "    该操作会将SD卡中的BOOT.bin文件写入到QSPI Flash中, 写入期间不要断电\n"
                "    如果写入失败则将启动模式改为SD卡, "
                "使用一个存有固件的SD卡启动设备重新进行此步骤即可\n"
                "    是否开始刷入固件?");
    } else {
        MessageBox_info("错误", "取消", "未挂载SD卡, 无法更新FLASH固件");
    }
}

static void calibration_time_btn_cb(lv_event_t *e) {
    LV_UNUSED(e);
    sntp_start();
}

static void refresh_timer_cb(lv_timer_t *timer) {
    if (!lv_obj_is_visible(timer->user_data))
        return;
    FSIZE_t total_size = 0, free_size = 0;
    if (Fatfs_GetMountStatus(0) == FR_OK) {
        Fatfs_GetVolSize("0:/", &total_size, &free_size);
        lv_label_set_text_fmt(sd_info, "总大小 %dMB, 剩余 %dMB, 已使用 %d%%",
                              (int) ((float) total_size / 1024.0f), (int) ((float) free_size / 1024.0f),
                              (int) ((float) (total_size - free_size) / (float) total_size * 100));
    } else {
        lv_label_set_text_fmt(sd_info, "SD卡未挂载, 代码%d", Fatfs_GetMountStatus(0));
    }

    if (Fatfs_GetMountStatus(1) == FR_OK) {
        Fatfs_GetVolSize("1:/", &total_size, &free_size);
        lv_label_set_text_fmt(emmc_info, "总大小 %dMB, 剩余 %dMB, 已使用 %d%%",
                              (int) ((float) total_size / 1024), (int) ((float) free_size / 1024),
                              (int) ((float) (total_size - free_size) / (float) total_size * 100));
    } else {
        lv_label_set_text_fmt(emmc_info, "EMMC未挂载, 代码%d", Fatfs_GetMountStatus(1));
    }

    int speed = network_linkSpeed();
    if (speed > 0) {
        char ip_str[3][IP4ADDR_STRLEN_MAX];
        memcpy(ip_str[0], ip4addr_ntoa(&netif_default->ip_addr), IP4ADDR_STRLEN_MAX);
        memcpy(ip_str[1], ip4addr_ntoa(&netif_default->gw), IP4ADDR_STRLEN_MAX);
        memcpy(ip_str[2], ip4addr_ntoa(&netif_default->netmask), IP4ADDR_STRLEN_MAX);
        lv_label_set_text_fmt(net_info, "IP: %s\n网关: %s\n子网掩码: %s\nMAC: %02X:%02X:%02X:%02X:%02X:%02X\n连接速度: %dMbps",
                              ip_str[0], ip_str[1], ip_str[2],
                              netif_default->hwaddr[0], netif_default->hwaddr[1],
                              netif_default->hwaddr[2], netif_default->hwaddr[3],
                              netif_default->hwaddr[4], netif_default->hwaddr[5],
                              speed);
    } else {
        lv_label_set_text(net_info, "网线未连接");
    }

    struct tm t;
    DS1337_GetTime(NULL, &t);
    lv_label_set_text_fmt(time_info, "当前时间: %2d:%02d:%02d %4d年%2d月%2d日 %s",
                          t.tm_hour, t.tm_min, t.tm_sec, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                          DS1337_WeekStr(t.tm_wday));

    XADC_SensorsData data;
    XADC_GetAll(&xAdcPs, &data);

    lv_table_set_cell_value_fmt(sensor_info, 1, 1, "%0d.%03d", PRINT_FLOAT(data.Temperature_Current));
    lv_table_set_cell_value_fmt(sensor_info, 1, 2, "%0d.%03d", PRINT_FLOAT(data.Temperature_Max));
    lv_table_set_cell_value_fmt(sensor_info, 1, 3, "%0d.%03d", PRINT_FLOAT(data.Temperature_Min));

    lv_table_set_cell_value_fmt(sensor_info, 2, 1, "%0d.%03dV", PRINT_FLOAT(data.VCCINT_Current));
    lv_table_set_cell_value_fmt(sensor_info, 2, 2, "%0d.%03dV", PRINT_FLOAT(data.VCCINT_Max));
    lv_table_set_cell_value_fmt(sensor_info, 2, 3, "%0d.%03dV", PRINT_FLOAT(data.VCCINT_Min));

    lv_table_set_cell_value_fmt(sensor_info, 3, 1, "%0d.%03dV", PRINT_FLOAT(data.VCCAUX_Current));
    lv_table_set_cell_value_fmt(sensor_info, 3, 2, "%0d.%03dV", PRINT_FLOAT(data.VCCAUX_Max));
    lv_table_set_cell_value_fmt(sensor_info, 3, 3, "%0d.%03dV", PRINT_FLOAT(data.VCCAUX_Min));

    lv_table_set_cell_value_fmt(sensor_info, 4, 1, "%0d.%03dV", PRINT_FLOAT(data.VBRAM_Current));
    lv_table_set_cell_value_fmt(sensor_info, 4, 2, "%0d.%03dV", PRINT_FLOAT(data.VBRAM_Max));
    lv_table_set_cell_value_fmt(sensor_info, 4, 3, "%0d.%03dV", PRINT_FLOAT(data.VBRAM_Min));

    lv_table_set_cell_value_fmt(sensor_info, 5, 1, "%0d.%03dV", PRINT_FLOAT(data.VCCPINT_Current));
    lv_table_set_cell_value_fmt(sensor_info, 5, 2, "%0d.%03dV", PRINT_FLOAT(data.VCCPINT_Max));
    lv_table_set_cell_value_fmt(sensor_info, 5, 3, "%0d.%03dV", PRINT_FLOAT(data.VCCPINT_Min));

    lv_table_set_cell_value_fmt(sensor_info, 6, 1, "%0d.%03dV", PRINT_FLOAT(data.VCCPAUX_Current));
    lv_table_set_cell_value_fmt(sensor_info, 6, 2, "%0d.%03dV", PRINT_FLOAT(data.VCCPAUX_Max));
    lv_table_set_cell_value_fmt(sensor_info, 6, 3, "%0d.%03dV", PRINT_FLOAT(data.VCCPAUX_Min));

    lv_table_set_cell_value_fmt(sensor_info, 7, 1, "%0d.%03dV", PRINT_FLOAT(data.VCCPDRO_Current));
    lv_table_set_cell_value_fmt(sensor_info, 7, 2, "%0d.%03dV", PRINT_FLOAT(data.VCCPDRO_Max));
    lv_table_set_cell_value_fmt(sensor_info, 7, 3, "%0d.%03dV", PRINT_FLOAT(data.VCCPDRO_Min));
}

static void signal_dropdown_cb(lv_event_t *event) {
    lv_obj_t *dropdown = lv_event_get_target(event);
    Channel_Index channelIndex = (Channel_Index) lv_event_get_user_data(event);
    if (channelIndex == CHANNEL_INDEX_SCOPE) {
        if (xSemaphoreTake(ADC_Mutex, 0)) {
            SPU_SwitchChannelSource(CHANNEL_INDEX_SCOPE, lv_dropdown_get_selected(dropdown));
            xSemaphoreGive(ADC_Mutex);
        } else lv_dropdown_set_selected(dropdown, !lv_dropdown_get_selected(dropdown));
    } else if (channelIndex == CHANNEL_INDEX_DAC) {
        if (xSemaphoreTake(DAC_Mutex, 0)) {
            SPU_SwitchChannelSource(CHANNEL_INDEX_DAC, lv_dropdown_get_selected(dropdown));
            xSemaphoreGive(DAC_Mutex);
        } else lv_dropdown_set_selected(dropdown, !lv_dropdown_get_selected(dropdown));
    } else SPU_SwitchChannelSource(channelIndex, lv_dropdown_get_selected(dropdown));
}
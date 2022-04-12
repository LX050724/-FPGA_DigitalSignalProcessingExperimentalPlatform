//
// Created by yaoji on 2022/1/16.
//

#include "DigitalFilter.h"
#include <arm_math.h>
#include "LVGL_Utils/FileSelectBox.h"
#include "Fatfs_init/Fatfs_Driver.h"
#include "FileDecoder/FileDecoder.h"
#include "AmplitudeResponse/AmplitudeResponse.h"
#include "LVGL_Utils/MessageBox.h"
#include "Controller/FIR_Controller.h"
#include "Controller/SPU_Controller.h"
#include "MainWindow.h"

static lv_obj_t *file_table;
static lv_obj_t *path_label;
static lv_obj_t *amp_chart;
static lv_obj_t *info_label;
static lv_obj_t *reload_btn;
static lv_obj_t *fraction_bit_textarea;

static lv_chart_series_t *amp_series;
static lv_chart_series_t *coe_series;
static int16_t coe_raw_data[65] __attribute__((aligned(8)));
static int coe_len;
static int16_t coe_data[65];
static int16_t amp_data[65];
static float amp_float_data[64];

static void file_table_path_change_cb(lv_obj_t *obj, const char *path);
static void file_table_click_cb(lv_obj_t *obj, const char *path, const char *filename);
static void reload_btn_cb(lv_event_t *event);
static void fraction_bit_textarea_event_cb(lv_event_t *event);

void DigitalFilter_create(lv_obj_t *parent) {
    path_label = lv_label_create(parent);
    lv_label_set_long_mode(path_label, LV_LABEL_LONG_SCROLL);
    lv_obj_set_width(path_label, LV_HOR_RES * 0.8);

    file_table = lv_file_select_box_create(parent);
    lv_table_set_row_cnt(file_table, 1);
    lv_table_set_col_cnt(file_table, 1);
    lv_obj_set_width(file_table, LV_HOR_RES * 0.3);
    lv_obj_set_height(file_table, LV_VER_RES * 0.7);
    lv_table_set_col_width(file_table, 0, LV_HOR_RES * 0.28);
    lv_obj_align_to(file_table, path_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    lv_file_select_box_set_click_callback(file_table, file_table_click_cb);
    lv_file_select_box_set_path_change_callback(file_table, file_table_path_change_cb);
    lv_obj_update_layout(file_table);

    amp_chart = lv_chart_create(parent);
    lv_obj_set_height(amp_chart, LV_VER_RES * 0.5);
    lv_obj_set_width(amp_chart, LV_HOR_RES * 0.6);
    lv_chart_set_type(amp_chart, LV_CHART_TYPE_LINE);
    lv_obj_set_style_size(amp_chart, 0, LV_PART_INDICATOR);
    lv_chart_set_div_line_count(amp_chart, 5, 16);
    amp_series = lv_chart_add_series(amp_chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
    coe_series = lv_chart_add_series(amp_chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_SECONDARY_Y);
    lv_chart_hide_series(amp_chart, amp_series, true);
    lv_chart_hide_series(amp_chart, coe_series, true);
    lv_chart_set_ext_y_array(amp_chart, amp_series, amp_data);
    lv_chart_set_ext_y_array(amp_chart, coe_series, coe_data);
    lv_chart_set_point_count(amp_chart, 65);
    lv_chart_set_range(amp_chart, LV_CHART_AXIS_PRIMARY_Y, -5000, 5000);
    lv_chart_set_range(amp_chart, LV_CHART_AXIS_SECONDARY_Y, -5000, 5000);
    lv_obj_align_to(amp_chart, file_table, LV_ALIGN_OUT_RIGHT_TOP, 20, 0);
    lv_chart_refresh(amp_chart);

    info_label = lv_label_create(parent);
    lv_label_set_text_static(info_label, "");
    lv_obj_set_width(info_label, lv_obj_get_width(amp_chart));
    lv_obj_align_to(info_label, amp_chart, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    reload_btn = lv_btn_create(parent);
    lv_obj_t *btn_text = lv_label_create(reload_btn);
    lv_label_set_text_static(btn_text, "重载滤波器");
    lv_obj_clear_flag(reload_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align_to(reload_btn, info_label, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 20);
    lv_obj_add_event_cb(reload_btn, reload_btn_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *fraction_bit_label = lv_label_create(parent);
    lv_label_set_text_static(fraction_bit_label, "输出分数位数:");
    lv_obj_align_to(fraction_bit_label, info_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 30);

    fraction_bit_textarea = lv_textarea_create(parent);
    lv_textarea_set_accepted_chars(fraction_bit_textarea, "0123456789");
    lv_textarea_set_text(fraction_bit_textarea, "14");
    lv_textarea_set_max_length(fraction_bit_textarea, 2);
    lv_textarea_set_one_line(fraction_bit_textarea, true);
    lv_obj_align_to(fraction_bit_textarea, fraction_bit_label, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    SPU_SetFirShift(14);

    lv_obj_t *keyboard = lv_keyboard_create(lv_scr_act());
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_NUMBER);
    lv_obj_set_style_text_font(keyboard, &lv_font_montserrat_16, 0);

    lv_obj_add_event_cb(fraction_bit_textarea, fraction_bit_textarea_event_cb, LV_EVENT_ALL, keyboard);

    char *buf = UTF8_TO_GBK("0:/数字滤波器");
    DIR dir;
    FRESULT res = f_opendir(&dir, buf);
    if (res == FR_OK) {
        lv_file_select_box_set_dir(file_table, "0:/数字滤波器/");
        f_closedir(&dir);
    } else if (res == FR_NO_PATH) {
        res = f_mkdir(buf);
        if (res != FR_OK) {
            LV_LOG_ERROR("创建文件夹 \"0:/数字滤波器\" 失败");
        } else {
            lv_file_select_box_set_dir(file_table, "0:/数字滤波器/");
        }
    }
    lv_mem_free(buf);
}

static void file_table_path_change_cb(lv_obj_t *obj, const char *path) {
    LV_UNUSED(obj);
    lv_label_set_text_static(path_label, path);
}

static void file_table_click_cb(lv_obj_t *obj, const char *path, const char *filename) {
    LV_UNUSED(obj);
    FDType type = FileDecoder_get_file_type(filename);
    if (type == FDType_coe) {
        size_t len;
        int16_t *data;
        FDStatus status = FileDecoder_open(path, NULL, NULL, (int8_t **) &data, &len);
        if (status == FDStatus_ok) {
            AmplitudeResponse(data, amp_float_data, len);
            float amp_max, amp_min;
            uint32_t index;
            arm_max_f32(amp_float_data, 64, &amp_max, &index);
            arm_min_f32(amp_float_data, 64, &amp_min, &index);
            for (int i = 0; i < 64; i++)
                amp_data[i] = (amp_float_data[i] - amp_min) * 10000 / (amp_max - amp_min) - 5000;
            amp_data[64] = amp_data[63];
            memset(coe_data, 0, sizeof(coe_data));
            for (int i = 0; i < 64 && i < len; i++)
                coe_data[i] = data[i] * (10000.0 / INT16_MAX);
            memcpy(coe_raw_data, data, sizeof(int16_t) * len);
            coe_len = len;
            lv_chart_refresh(amp_chart);
            lv_chart_hide_series(amp_chart, amp_series, false);
            lv_chart_hide_series(amp_chart, coe_series, false);
            lv_mem_free(data);
            lv_label_set_text_fmt(info_label, "%s: 长度 %d, 最大衰减 %.2fdB, 最小衰减 %.2fdB", filename, len, amp_min,
                                  amp_max);
            lv_obj_add_flag(reload_btn, LV_OBJ_FLAG_CLICKABLE);
        } else {
            MessageBox_info("错误", "关闭", FileDecoder_status_string(status));
        }
    }
}

void reload_btn_cb(lv_event_t *event) {
    LV_UNUSED(event);
    int fraction_bit = strtol(lv_textarea_get_text(fraction_bit_textarea), NULL, 10);
    if (fraction_bit > 24) {
        MessageBox_info("错误", "关闭", "分数长度大于24");
        return;
    }
    if (coe_len == 65) {
        for (int i = 0; i < 32; i++) {
            if (coe_raw_data[i] != coe_raw_data[64 - i]) {
                MessageBox_info("错误", "关闭", "FIR系数不符合偶对称条件");
                return;
            }
        }
        int ret = FIR_reload_coe(coe_raw_data);
        if (ret == XST_SUCCESS) {
            SPU_SetFirShift(fraction_bit);
            LV_LOG_USER("FIR: file='%s', fraction_bit=%d", lv_label_get_text(path_label), fraction_bit);
            MessageBox_info("完成", "关闭", "滤波器系数重载完成");
        } else {
            MessageBox_info("错误", "关闭", "重载FIR系数时发生错误 code:%d", ret);
        }
    } else {
        MessageBox_info("错误", "关闭", "FIR系数长度不等于65");
    }
}

static void fraction_bit_textarea_event_cb(lv_event_t *event) {
    lv_event_code_t code = lv_event_get_code(event);
    lv_obj_t *textarea = lv_event_get_target(event);
    lv_obj_t *keyboard = lv_event_get_user_data(event);

    if (code == LV_EVENT_FOCUSED) {
        if (lv_indev_get_type(lv_indev_get_act()) != LV_INDEV_TYPE_KEYPAD) {
            lv_keyboard_set_textarea(keyboard, textarea);
            lv_obj_set_style_max_height(keyboard, LV_HOR_RES * 2 / 3, 0);
            lv_obj_set_style_max_width(keyboard, LV_HOR_RES * 0.75, 0);
            lv_obj_update_layout(tabview);
            // 设置对象的高度，当唤出键盘时应该偏移背景
            lv_obj_set_height(tabview, LV_VER_RES - lv_obj_get_height(keyboard));
            // 清除标志
            lv_obj_clear_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
            // 滚动到一个对象，直到它在其父对象上可见。对父母的父母做同样的事情，依此类推。即使对象具有嵌套的可滚动父对象，它也会滚动到视图中
            lv_obj_scroll_to_view_recursive(keyboard, LV_ANIM_ON);
            lv_obj_scroll_to_view_recursive(textarea, LV_ANIM_ON);
        }
    } else if (code == LV_EVENT_DEFOCUSED) {
        lv_keyboard_set_textarea(keyboard, NULL);
        lv_obj_set_height(tabview, LV_VER_RES);
        lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    } else if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        lv_obj_set_height(tabview, LV_VER_RES);
        lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);            // 标志为隐藏对象(使对象隐藏，就像它根本不存在一样)
        lv_obj_clear_state(textarea, LV_STATE_FOCUSED);    // 删除对象的一种或多种状态。其他状态位将保持不变。
        lv_indev_reset(NULL, textarea);                    // 忘记最后一次点击的对象，使其再次成为可关注的对象
    }
}


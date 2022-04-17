//
// Created by yaoji on 2022/1/16.
//

#include <math.h>
#include "NetworkAnalyzer.h"
#include "LVGL_Utils/Chart_zoom_plugin.h"
#include "Controller/ADC_Controller.h"
#include "Controller/DDS_Controller.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "LVGL_Zynq_Init/zynq_lvgl_init.h"
#include "LVGL_Utils/MessageBox.h"

static lv_obj_t *chart;
static lv_chart_series_t *ser1;
static lv_obj_t *start_btn;
static lv_obj_t *start_btn_text;

#define MAX_POINTS 1024

static lv_coord_t data[MAX_POINTS];
static TaskHandle_t scan_task_handel;
static xSemaphoreHandle complete;
static int scan_length, plan;

static uint32_t excitation_voltage = 10e3;
static uint32_t start_freq = 1e3, end_freq = 5e6;

static void scan_task(void *param);
static void start_click_cb(lv_event_t *event);
static void scroll_btn_cb(lv_event_t *event);
static void freq_slider_value_change_cb(lv_event_t *event);
static void length_slider_value_change_cb(lv_event_t *event);
void voltage_slider_value_change_cb(lv_event_t *event);
static void timer_cb(lv_timer_t *timer);


void NetworkAnalyzer_create(lv_obj_t *parent) {
    complete = xSemaphoreCreateBinary();
    lv_obj_t *tv = lv_tileview_create(parent);
    lv_obj_t *tile1 = lv_tileview_add_tile(tv, 0, 0, LV_DIR_BOTTOM);
    lv_obj_t *tile2 = lv_tileview_add_tile(tv, 0, 1, LV_DIR_TOP);

    chart = lv_chart_create(tile1);
    lv_obj_set_size(chart, 900, 400);
    lv_obj_align_to(chart, parent, LV_ALIGN_TOP_MID, -10, 0);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
    lv_obj_set_style_size(chart, 0, LV_PART_INDICATOR);
    ser1 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_ext_y_array(chart, ser1, data);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, -3000, 0);
    lv_chart_set_point_count(chart, MAX_POINTS);
    lv_chart_set_div_line_count(chart, 12 + 1, 15 + 1);

    /* 安装缩放插件 */
    lv_chart_install_zoom_plugin(chart);

    start_btn = lv_btn_create(tile1);
    start_btn_text = lv_label_create(start_btn);
    lv_label_set_text_static(start_btn_text, "启动扫描");
    lv_obj_align_to(start_btn, chart, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 30);
    lv_obj_add_event_cb(start_btn, start_click_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *setting_btn = lv_btn_create(tile1);
    lv_obj_t *setting_btn_text = lv_label_create(setting_btn);
    lv_label_set_text_static(setting_btn_text, "\uf077上滑显示参数\uf077");
    lv_obj_align_to(setting_btn, start_btn, LV_ALIGN_OUT_LEFT_MID, -20, 0);
    lv_obj_add_event_cb(setting_btn, scroll_btn_cb, LV_EVENT_CLICKED, tv);

    /**
     * 水平缩放
     */
    lv_obj_t *zoom_x_slider = lv_slider_create(tile1);
    lv_slider_set_range(zoom_x_slider, 256, 2048);
    lv_coord_t zoom_x_slider_width =
            lv_obj_get_width(chart) - lv_obj_get_width(start_btn) - lv_obj_get_width(setting_btn) - 40;
    lv_obj_set_width(zoom_x_slider, zoom_x_slider_width);
    lv_obj_align_to(zoom_x_slider, setting_btn, LV_ALIGN_OUT_LEFT_MID, -20, 0);
    lv_obj_add_event_cb(zoom_x_slider, lv_chart_zoom_slider_x_cb, LV_EVENT_VALUE_CHANGED, chart);

    lv_obj_t *zoom_x_label = lv_label_create(tile1);
    lv_label_set_text_static(zoom_x_label, "水平缩放");
    lv_obj_align_to(zoom_x_label, zoom_x_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);


    lv_obj_t *range_label = lv_label_create(tile2);
    lv_label_set_text_static(range_label, "扫描范围");
    lv_obj_set_pos(range_label, 0, 30);

    lv_obj_t *range_slider = lv_slider_create(tile2);
    lv_obj_set_width(range_slider, lv_obj_get_width(tile2) - lv_obj_get_width(range_label) - 120);
    lv_slider_set_mode(range_slider, LV_SLIDER_MODE_RANGE);
    lv_slider_set_range(range_slider, start_freq / 1000, end_freq / 1000);
    lv_slider_set_value(range_slider, 5000, LV_ANIM_OFF);
    lv_slider_set_left_value(range_slider, 1, LV_ANIM_OFF);
    lv_obj_align_to(range_slider, range_label, LV_ALIGN_OUT_RIGHT_MID, 20, 0);

    lv_obj_t *range_val_label = lv_label_create(tile2);
    lv_obj_align_to(range_val_label, range_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);

    lv_obj_add_event_cb(range_slider, freq_slider_value_change_cb, LV_EVENT_VALUE_CHANGED, range_val_label);
    lv_event_send(range_slider, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t *length_label = lv_label_create(tile2);
    lv_label_set_text_static(length_label, "扫描点数");
    lv_obj_align_to(length_label, range_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 50);

    lv_obj_t *length_slider = lv_slider_create(tile2);
    lv_slider_set_range(length_slider, 128, MAX_POINTS);
    lv_slider_set_value(length_slider, MAX_POINTS, LV_ANIM_OFF);
    lv_obj_set_width(length_slider, lv_obj_get_width(range_slider));
    lv_obj_align_to(length_slider, length_label, LV_ALIGN_OUT_RIGHT_MID, 20, 0);

    lv_obj_t *length_slider_label = lv_label_create(tile2);
    lv_obj_align_to(length_slider_label, length_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);
    lv_obj_add_event_cb(length_slider, length_slider_value_change_cb, LV_EVENT_VALUE_CHANGED, length_slider_label);
    lv_event_send(length_slider, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t *voltage_label = lv_label_create(tile2);
    lv_label_set_text_static(voltage_label, "激励电压");
    lv_obj_align_to(voltage_label, length_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 50);

    lv_obj_t *voltage_slider = lv_slider_create(tile2);
    lv_slider_set_range(voltage_slider, 0, excitation_voltage);
    lv_slider_set_value(voltage_slider, excitation_voltage, LV_ANIM_OFF);
    lv_obj_set_width(voltage_slider, lv_obj_get_width(range_slider));
    lv_obj_align_to(voltage_slider, voltage_label, LV_ALIGN_OUT_RIGHT_MID, 20, 0);

    lv_obj_t *voltage_slider_label = lv_label_create(tile2);
    lv_obj_align_to(voltage_slider_label, voltage_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);
    lv_obj_add_event_cb(voltage_slider, voltage_slider_value_change_cb, LV_EVENT_VALUE_CHANGED, voltage_slider_label);
    lv_event_send(voltage_slider, LV_EVENT_VALUE_CHANGED, NULL);

    lv_timer_create(timer_cb, 10, parent);
}

static void scan_task(void *param) {
    LV_UNUSED(param);
    DDS_sine_t ddsSine = {.base.type = TYPE_SINE};
    ddsSine.base.amplitude = excitation_voltage;
    float _start_freq = start_freq;
    float _end_freq = end_freq;
    int _scan_length = scan_length;
    plan = 0;

    if (xSemaphoreTake(ADC_Mutex, 100) != pdTRUE || xSemaphoreTake(DAC_Mutex, 100) != pdTRUE) {
        if (xSemaphoreTake(LVGL_Mutex, 100) == pdTRUE) {
            MessageBox_info("错误", "关闭", "无法获取硬件资源");
            xSemaphoreGive(LVGL_Mutex);
        }
        goto end;
    }
    for (int i = 0; i < _scan_length; i++) {
        ddsSine.base.freq = _start_freq + (_end_freq - _start_freq) * (i + 1) / _scan_length;
        DDS_wav_generator(&ddsSine);
        vTaskDelay(10);
        ADC_get_data_now(NULL, 10);
        float rms = ADC_get_rms();
        data[i] = 20 * log10(rms / ddsSine.base.amplitude) * 100;
        plan = 100 * (i + 1) / _scan_length;
    }
    xSemaphoreGive(ADC_Mutex);
    xSemaphoreGive(DAC_Mutex);
    end:
    xSemaphoreGive(complete);
    vTaskDelete(NULL);
}

static void start_click_cb(lv_event_t *event) {
    LV_UNUSED(event);
    lv_obj_clear_flag(start_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_chart_set_point_count(chart, scan_length);
    xTaskCreate(scan_task, "scan_task", 1024, NULL, 4, &scan_task_handel);
}

static void freq_slider_value_change_cb(lv_event_t *event) {
    lv_obj_t *range_slider = lv_event_get_target(event);
    lv_obj_t *range_val_label = lv_event_get_user_data(event);
    float left = lv_slider_get_left_value(range_slider);
    float right = lv_slider_get_value(range_slider);
    start_freq = left * 1000;
    end_freq = right * 1000;
    int left_dec = 0, right_dec = 0;
    char *left_unit_str = "KHz", *right_unit_str = "KHz";
    if (left > 1e3) {
        left_unit_str = "MHz";
        left /= 1000;
        left_dec = 2;
    }
    if (right > 1e3) {
        right_unit_str = "MHz";
        right /= 1000;
        right_dec = 2;
    }
    lv_label_set_text_fmt(range_val_label, "%.*f%s —— %.*f%s",
                          left_dec, left, left_unit_str,
                          right_dec, right, right_unit_str);
}

static void scroll_btn_cb(lv_event_t *event) {
    lv_obj_t *tv = lv_event_get_user_data(event);
    lv_obj_set_tile_id(tv, 0, 1, LV_ANIM_ON);
}

void timer_cb(lv_timer_t *timer) {
    if (xSemaphoreTake(complete, 0) == pdTRUE) {
        lv_label_set_text_static(start_btn_text, "启动扫描");
        lv_obj_add_flag(start_btn, LV_OBJ_FLAG_CLICKABLE);
        MessageBox_info("完成", "关闭", "扫描完成");
    }

    if (!lv_obj_is_visible(timer->user_data))
        return;

    if (!lv_obj_has_flag(start_btn, LV_OBJ_FLAG_CLICKABLE)) {
        lv_label_set_text_fmt(start_btn_text, "%d%%", plan);
        lv_chart_refresh(chart);
    }
}

void length_slider_value_change_cb(lv_event_t *event) {
    lv_obj_t *slider = lv_event_get_target(event);
    lv_obj_t *label = lv_event_get_user_data(event);
    lv_coord_t val = lv_slider_get_value(slider);
    lv_label_set_text_fmt(label, "%d", val);
    scan_length = val;
}

void voltage_slider_value_change_cb(lv_event_t *event) {
    lv_obj_t *slider = lv_event_get_target(event);
    lv_obj_t *label = lv_event_get_user_data(event);
    excitation_voltage = lv_slider_get_value(slider);
    lv_label_set_text_fmt(label, "%.3fV", excitation_voltage / 1000.0f);
}

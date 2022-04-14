//
// Created by yaoji on 2022/1/16.
//

#include "SpectrumAnalyzer.h"

#include "Controller/FFT_Controller.h"

#include "xaxidma.h"
#include "check.h"
#include "LVGL_Utils/Chart_zoom_plugin.h"
#include <arm_math.h>

static lv_obj_t *chart;
static lv_chart_series_t *ser1;
static lv_obj_t *freq_label[16];
extern XAxiDma dma1;

static void fft_timer_cb(lv_timer_t *timer);
static void chart_change_event_cb(lv_event_t *event);

static int16_t data[4096];

void SpectrumAnalyzer_create(lv_obj_t *parent) {
    chart = lv_chart_create(parent);
    lv_obj_set_size(chart, 900, 400);
    lv_obj_align_to(chart, parent, LV_ALIGN_TOP_MID, -10, 0);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
    lv_obj_set_style_size(chart, 0, LV_PART_INDICATOR);
    ser1 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_ext_y_array(chart, ser1, data);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, -12000, 0);
    lv_chart_set_point_count(chart, 4096);
    lv_chart_set_div_line_count(chart, 12 + 1, 15 + 1);

    /* 安装缩放插件 */
    lv_chart_install_zoom_plugin(chart);

    /**
     * 水平缩放
     */
    lv_obj_t *zoom_x_slider = lv_slider_create(parent);
    lv_slider_set_range(zoom_x_slider, 256, 2048);
    lv_obj_set_width(zoom_x_slider, lv_obj_get_width(chart));
    lv_obj_align_to(zoom_x_slider, chart, LV_ALIGN_OUT_BOTTOM_MID, 0, 30);
    lv_obj_add_event_cb(zoom_x_slider, lv_chart_zoom_slider_x_cb, LV_EVENT_VALUE_CHANGED, chart);

    lv_obj_t *zoom_x_label = lv_label_create(parent);
    lv_label_set_text_static(zoom_x_label, "水平缩放");
    lv_obj_align_to(zoom_x_label, zoom_x_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);

    /**
     * 添加刻度标签
     */
    for (int i = 0; i <= 15; i++) {
        freq_label[i] = lv_label_create(chart);
        lv_label_set_text_fmt(freq_label[i], "%dMhz", i);
    }
    lv_obj_add_event_cb(chart, chart_change_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(zoom_x_slider, chart_change_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_event_send(zoom_x_slider, LV_EVENT_VALUE_CHANGED, NULL);

    lv_timer_create(fft_timer_cb, 1, parent);
}

static inline float inRange(float _min, float _v, float _max) {
    return _v >= _max ? _max :
           _v <= _min ? _min : _v;
}

static void fft_timer_cb(lv_timer_t *timer) {
    if (!lv_obj_is_visible(timer->user_data))
        return;
    if (FFT_get_data() == XST_SUCCESS) {
        for (int i = 0; i < 4096; i++) {
            data[i] = inRange(-120.0, FFT_OriginalData[i], 0.0) * 100;
        }
        lv_chart_refresh(chart);
    }
}

void chart_change_event_cb(lv_event_t *event) {
    lv_event_code_t code = lv_event_get_code(event);
    if (code == LV_EVENT_SCROLL_BEGIN ||
        code == LV_EVENT_SCROLL ||
        code == LV_EVENT_VALUE_CHANGED) {
        lv_coord_t offset = lv_chart_get_window_width(chart);
        for (int i = 0; i <= 15; i++) {
            lv_point_t point;
            lv_chart_get_point_pos_by_id(chart, ser1, i * 4095 / 15, &point);
            lv_obj_align_to(freq_label[i], chart, LV_ALIGN_TOP_LEFT, point.x - offset * 0.04, 0);
        }
    }
}

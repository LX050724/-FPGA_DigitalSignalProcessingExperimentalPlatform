//
// Created by yaoji on 2022/1/16.
//

#include "SpectrumAnalyzer.h"

#include "Controller/FFT_Controller.h"

#include "xaxidma.h"
#include "utils.h"
#include <arm_math.h>

static lv_obj_t *chart;

extern XAxiDma dma1;

static void fft_timer_cb(lv_timer_t *timer);

static int16_t data[4096];

void SpectrumAnalyzer_create(lv_obj_t *parent) {
    chart = lv_chart_create(parent);
    lv_obj_set_size(chart, 800, 400);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
    lv_chart_series_t *ser1 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);

    lv_chart_set_ext_y_array(chart, ser1, data);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, -12000, 0);
    lv_chart_set_point_count(chart, 4096);
    lv_chart_set_div_line_count(chart, 12 + 1, 15 + 1);

    lv_timer_create(fft_timer_cb, 1, parent);
}

static inline double inRange(double _min, double _v, double _max) {
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

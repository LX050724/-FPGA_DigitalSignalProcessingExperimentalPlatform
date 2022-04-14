//
// Created by yaoji on 2022/1/16.
//

#include "Oscilloscope.h"
#include "Controller/ADC_Controller.h"
#include "LVGL_Utils/slider.h"
#include "math.h"
#include "LVGL_Utils/Chart_zoom_plugin.h"

static lv_obj_t *chart;
static lv_chart_cursor_t *cursor_ver;
static lv_chart_cursor_t *cursor_hor;
static lv_chart_series_t *series;
static lv_obj_t *zoom_x_slider;
static lv_obj_t *zoom_y_slider;
static lv_obj_t *measure_text_label;


static union {
    struct {
        uint8_t max_mix;
        uint8_t vpp;
        uint8_t freq;
        uint8_t period;
        uint8_t rms;
        uint8_t rms_cycle;
        uint8_t mean;
        uint8_t mean_cycle;
    } item;
    uint8_t all[8];
} measure_switch;

static void adc_timer_cb(lv_timer_t *timer);
static void trigger_dd_cb(lv_event_t *e);
static void trigger_level_slider_cb(lv_event_t *e);
static void measure_checkbox_cb(lv_event_t *e);
static void trigger_position_slider_cb(lv_event_t *e);
static void scroll_btn_cb(lv_event_t *e);

void Oscilloscope_create(lv_obj_t *parent) {
    lv_obj_t *tv = lv_tileview_create(parent);
    lv_obj_t *tile1 = lv_tileview_add_tile(tv, 0, 0, LV_DIR_BOTTOM);
    lv_obj_t *tile2 = lv_tileview_add_tile(tv, 0, 1, LV_DIR_TOP);

    /**
     * 图表控件
     */
    chart = lv_chart_create(tile1);
    lv_obj_set_size(chart, 900, 400);
    lv_obj_align_to(chart, tile1, LV_ALIGN_TOP_MID, 0, 0);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
    lv_obj_set_style_size(chart, 0, LV_PART_INDICATOR);
    series = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_ext_y_array(chart, series, ADC_Data);
    lv_chart_set_point_count(chart, 4096);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, -5000, 5000);
    cursor_hor = lv_chart_add_cursor(chart, lv_palette_main(LV_PALETTE_BLUE), LV_DIR_HOR);
    cursor_ver = lv_chart_add_cursor(chart, lv_palette_main(LV_PALETTE_YELLOW), LV_DIR_VER);
    lv_chart_install_zoom_plugin(chart);
    /**
     * x轴缩放控件组
     */
    zoom_x_slider = lv_slider_create(tile1);
    lv_slider_set_range(zoom_x_slider, 256, 4096);
    lv_obj_set_width(zoom_x_slider, LV_HOR_RES * 0.35);
    lv_obj_align_to(zoom_x_slider, tile1, LV_ALIGN_BOTTOM_LEFT, 15, -20);
    lv_obj_add_event_cb(zoom_x_slider, lv_chart_zoom_slider_x_cb, LV_EVENT_VALUE_CHANGED, chart);

    lv_obj_t *zoom_x_label = lv_label_create(tile1);
    lv_label_set_text_static(zoom_x_label, "水平缩放");
    lv_obj_align_to(zoom_x_label, zoom_x_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);

    /**
     * y轴缩放控件组
     */
    zoom_y_slider = lv_slider_create(tile1);
    lv_slider_set_range(zoom_y_slider, 256, 4096);
    lv_obj_set_width(zoom_y_slider, LV_HOR_RES * 0.35);
    lv_obj_align_to(zoom_y_slider, tile1, LV_ALIGN_BOTTOM_RIGHT, -15, -20);
    lv_obj_add_event_cb(zoom_y_slider, lv_chart_zoom_slider_y_cb, LV_EVENT_VALUE_CHANGED, chart);

    lv_obj_t *zoom_y_label = lv_label_create(tile1);
    lv_label_set_text_static(zoom_y_label, "垂直缩放");
    lv_obj_align_to(zoom_y_label, zoom_y_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);

    /**
     * 上滑显示参数标签
     */
    lv_obj_t *scroll_btn = lv_btn_create(tile1);
    lv_obj_t *scroll_label = lv_label_create(scroll_btn);
    lv_label_set_text_static(scroll_label, "\uf077上滑显示参数\uf077");
    lv_obj_align_to(scroll_btn, tile1, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_add_event_cb(scroll_btn, scroll_btn_cb, LV_EVENT_CLICKED, tv);

    /**
     * 调整触发方式控件组
     */
    lv_obj_t *trigger_label = lv_label_create(tile2);
    lv_label_set_text_static(trigger_label, "触发方式:");
    lv_obj_set_pos(trigger_label, 0, 20);

    lv_obj_t *trigger_dd = lv_dropdown_create(tile2);
    lv_dropdown_set_options_static(trigger_dd, "上升沿触发\n下降沿触发\n无触发");
    lv_obj_add_event_cb(trigger_dd, trigger_dd_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_align_to(trigger_dd, trigger_label, LV_ALIGN_LEFT_MID, LV_HOR_RES * 0.15, 0);

    /**
     * 调整触发电平控件组
     */
    lv_obj_t *trigger_level_label = lv_label_create(tile2);
    lv_label_set_text_static(trigger_level_label, "触发电平:");
    lv_obj_align_to(trigger_level_label, trigger_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 50);
    lv_obj_t *trigger_level_slider = slider_create(tile2, trigger_level_label, "%ldmV", -5000, 5000);
    lv_obj_add_event_cb(trigger_level_slider, trigger_level_slider_cb, LV_EVENT_VALUE_CHANGED, NULL);

    /**
     * 调整触发位置控件组
     */
    lv_obj_t *trigger_position_label = lv_label_create(tile2);
    lv_label_set_text_static(trigger_position_label, "触发位置:");
    lv_obj_align_to(trigger_position_label, trigger_level_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 50);
    lv_obj_t *trigger_position_slider = slider_create(tile2, trigger_position_label, "%ld采样", -2048, 2047);
    lv_obj_add_event_cb(trigger_position_slider, trigger_position_slider_cb, LV_EVENT_VALUE_CHANGED, NULL);

    /**
     * 添加测量
     */
    lv_obj_t *measure_label = lv_label_create(tile2);
    lv_label_set_text_static(measure_label, "添加测量:");
    lv_obj_align_to(measure_label, trigger_position_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 50);

    lv_obj_t *check_box_container = lv_obj_create(tile2);
    static lv_coord_t col_dsc[] = {200, 200, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] = {50, 50, 50, 50, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_style_grid_column_dsc_array(check_box_container, col_dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(check_box_container, row_dsc, 0);
    lv_obj_set_layout(check_box_container, LV_LAYOUT_GRID);

    static const char *check_box_text[] = {"最大值最小值", "峰峰值", "频率", "周期",
                                           "均方根", "周期均方根", "平均值", "周期平均值"};
    for (int i = 0; i < 8; i++) {
        uint8_t col = i % 2;
        uint8_t row = i / 2;

        lv_obj_t *obj = lv_checkbox_create(check_box_container);
        lv_checkbox_set_text_static(obj, check_box_text[i]);
        lv_obj_add_event_cb(obj, measure_checkbox_cb, LV_EVENT_VALUE_CHANGED, &measure_switch.all[i]);

        lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, col, 1,
                             LV_GRID_ALIGN_STRETCH, row, 1);
    }
    lv_obj_align_to(check_box_container, measure_label, LV_ALIGN_TOP_LEFT, LV_HOR_RES * 0.15, 0);
    lv_obj_set_size(check_box_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    measure_text_label = lv_label_create(tile1);
    lv_label_set_text(measure_text_label, "");
    lv_label_set_recolor(measure_text_label, true);
    lv_obj_align_to(measure_text_label, chart, LV_ALIGN_TOP_LEFT, 0, 0);

    /**
     * 其他
     */
    lv_timer_create(adc_timer_cb, 1, parent);
}

/**
 * 定时器回调，用于实现ADC数据刷新
 * @param timer
 */
static void adc_timer_cb(lv_timer_t *timer) {
    if (!lv_obj_is_visible(timer->user_data))
        return;
    LV_UNUSED(timer);
    bool triggered;
    if (ADC_get_data(&triggered) == XST_SUCCESS) {
        int16_t trigger_level = ADC_get_trigger_level();
        float self_height = lv_obj_get_self_height(chart);
        lv_coord_t offset = (lv_obj_get_height(chart) - lv_chart_get_window_height(chart)) / 2;
        cursor_hor->pos.y = self_height * (1 - (trigger_level + 5000) / 10000.0) - lv_obj_get_scroll_top(chart) + offset;
        cursor_hor->pos_set = 1;

        if (triggered) {
            cursor_ver->pos_set = 0;
            cursor_ver->point_id = ADC_get_trigger_position();
            cursor_ver->ser = series;
        } else {
            cursor_ver->pos_set = 1;
            cursor_ver->pos.x = -100;
            cursor_ver->pos.y = -100;
        }

        float max, min;
        ADC_get_max_min(&max, &min);
        ADC_set_trigger_hysteresis((max - min) * 0.02);

        char *buf = lv_mem_alloc(512);
        LV_ASSERT_MALLOC(buf)
        char *buf2 = lv_mem_alloc(128);
        LV_ASSERT_MALLOC(buf2)
        lv_memset_00(buf, 512);
        lv_memset_00(buf2, 128);
        if (measure_switch.item.max_mix) {
            sprintf(buf2, "#EF00EF 最大值:# #03A9F4 %.2fV# #EF00EF 最小值:# #03A9F4 %.2fV#\n", max / 1000, min / 1000);
            strcat(buf, buf2);
        }
        if (measure_switch.item.vpp) {
            sprintf(buf2, "#EF00EF 峰峰值:# #03A9F4 %.2fV#\n", (max - min) / 1000);
            strcat(buf, buf2);
        }
        if (measure_switch.item.freq || measure_switch.item.period) {
            float period = ADC_get_period();
            if (measure_switch.item.freq) {
                float freq = 1 / period;
                float l10 = log10f(freq);
                if (l10 >= 6) {
                    sprintf(buf2, "#EF00EF 频率:# #03A9F4 %.3fMHz#\n", freq / 1e6);
                } else if (l10 >= 3) {
                    sprintf(buf2, "#EF00EF 频率:# #03A9F4 %.3fKHz#\n", freq / 1e3);
                } else {
                    sprintf(buf2, "#EF00EF 频率:# #03A9F4 %.2fHz#\n", freq);
                }
                strcat(buf, buf2);
            }
            if (measure_switch.item.period) {
                float l10 = log10f(period);
                if (l10 < -6) {
                    sprintf(buf2, "#EF00EF 周期:# #03A9F4 %.2fns#\n", period * 1e9);
                } else if (l10 < -3) {
                    sprintf(buf2, "#EF00EF 周期:# #03A9F4 %.3fus#\n", period * 1e6);
                } else {
                    sprintf(buf2, "#EF00EF 周期:# #03A9F4 %.3fms#\n", period * 1e3);
                }
                strcat(buf, buf2);
            }
        }
        if (measure_switch.item.mean) {
            sprintf(buf2, "#EF00EF 平均值:# #03A9F4 %.2fV#\n", ADC_get_mean() / 1000);
            strcat(buf, buf2);
        }
        if (measure_switch.item.mean_cycle) {
            sprintf(buf2, "#EF00EF 周期平均值:# #03A9F4 %.2fV#\n", ADC_get_mean_cycle() / 1000);
            strcat(buf, buf2);
        }
        if (measure_switch.item.rms) {
            sprintf(buf2, "#EF00EF 均方根:# #03A9F4 %.2fV#\n", ADC_get_rms() / 1000);
            strcat(buf, buf2);
        }
        if (measure_switch.item.rms_cycle) {
            sprintf(buf2, "#EF00EF 周期均方根:# #03A9F4 %.2fV#\n", ADC_get_rms_cycle() / 1000);
            strcat(buf, buf2);
        }
        lv_label_set_text(measure_text_label, buf);
        lv_mem_free(buf2);
        lv_mem_free(buf);
        lv_chart_refresh(chart);
    }
}

/**
 * 触发方式选择下拉菜单回调
 * @param e
 */
static void trigger_dd_cb(lv_event_t *e) {
    lv_obj_t *obj = lv_event_get_target(e);
    uint16_t selected = lv_dropdown_get_selected(obj);
    ADC_set_trigger_condition(selected);
}

static void trigger_level_slider_cb(lv_event_t *e) {
    lv_obj_t *obj = lv_event_get_target(e);
    int32_t value = lv_slider_get_value(obj);
    ADC_set_trigger_level(value);
}

static void measure_checkbox_cb(lv_event_t *e) {
    lv_obj_t *checkbox = lv_event_get_target(e);
    uint8_t *sw = lv_event_get_user_data(e);
    if (sw) *sw = lv_obj_get_state(checkbox) & LV_STATE_CHECKED ? 1 : 0;
}

static void trigger_position_slider_cb(lv_event_t *e) {
    lv_obj_t *obj = lv_event_get_target(e);
    int32_t value = lv_slider_get_value(obj);
    ADC_set_trigger_position(value + 2048);
}

static void scroll_btn_cb(lv_event_t *e) {
    lv_obj_t *tv = lv_event_get_user_data(e);
    lv_obj_set_tile_id(tv, 0, 1, LV_ANIM_ON);
}
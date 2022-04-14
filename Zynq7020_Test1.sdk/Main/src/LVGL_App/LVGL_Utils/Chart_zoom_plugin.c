//
// Created by yaoji on 2022/4/14.
//

#include "Chart_zoom_plugin.h"
#include "FreeRTOS_Mem/FreeRTOS_Mem.h"

typedef struct {
    lv_coord_t last_zoom_x, last_zoom_y;
    lv_coord_t window_width, window_height;
    float window_locate_x, window_locate_y;
} zoom_data_t;

static void chart_event_cb(lv_event_t *e) {
    lv_obj_t *chart = lv_event_get_target(e);
    zoom_data_t *data = lv_obj_get_user_data(chart);
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_SCROLL_BEGIN || code == LV_EVENT_SCROLL) {
        lv_coord_t self_height = lv_obj_get_self_height(chart);
        lv_coord_t self_width = lv_obj_get_self_width(chart);
        lv_coord_t scroll_top = lv_obj_get_scroll_top(chart);
        lv_coord_t scroll_bottom = lv_obj_get_scroll_bottom(chart);
        lv_coord_t scroll_left = lv_obj_get_scroll_left(chart);
        lv_coord_t scroll_right = lv_obj_get_scroll_right(chart);
        data->window_height = self_height - scroll_top - scroll_bottom;
        data->window_width = self_width - scroll_left - scroll_right;
        data->window_locate_x = (float) (scroll_left + data->window_width / 2) / self_width;
        data->window_locate_y = (float) (scroll_top + data->window_height / 2) / self_height;
    } else if (code == LV_EVENT_DRAW_PART_END) {
        lv_coord_t zoom_x = lv_chart_get_zoom_x(chart);
        lv_coord_t zoom_y = lv_chart_get_zoom_y(chart);
        if (data->last_zoom_x != zoom_x) {
            data->last_zoom_x = zoom_x;
            lv_coord_t scroll = data->window_locate_x * (data->window_width * (float) zoom_x / 256);
            lv_obj_scroll_to_x(chart, scroll - data->window_width / 2, LV_ANIM_OFF);
        }
        if (data->last_zoom_y != zoom_y) {
            data->last_zoom_y = zoom_y;
            lv_coord_t scroll = data->window_locate_y * (data->window_height * (float) zoom_y / 256);
            lv_obj_scroll_to_y(chart, scroll - data->window_height / 2, LV_ANIM_OFF);
        }
    } else if (code == LV_EVENT_DELETE) {
        os_free(data);
    }
}

void lv_chart_install_zoom_plugin(lv_obj_t *obj) {
    zoom_data_t *data = os_malloc(sizeof(zoom_data_t));
    LV_ASSERT_MALLOC(data);
    lv_obj_set_user_data(obj, data);
    lv_obj_add_event_cb(obj, chart_event_cb, LV_EVENT_ALL, NULL);
    lv_event_send(obj, LV_EVENT_SCROLL_BEGIN, NULL);
}

void lv_chart_zoom_slider_x_cb(lv_event_t *e) {
    lv_obj_t *obj = lv_event_get_target(e);
    float zoom = lv_slider_get_value(obj);
    lv_chart_set_zoom_x(lv_event_get_user_data(e), zoom);
}

void lv_chart_zoom_slider_y_cb(lv_event_t *e) {
    lv_obj_t *obj = lv_event_get_target(e);
    float zoom = lv_slider_get_value(obj);
    lv_chart_set_zoom_y(lv_event_get_user_data(e), zoom);
}

lv_coord_t lv_chart_get_window_width(lv_obj_t *obj) {
    lv_coord_t self_width = lv_obj_get_self_width(obj);
    lv_coord_t scroll_left = lv_obj_get_scroll_left(obj);
    lv_coord_t scroll_right = lv_obj_get_scroll_right(obj);
    return self_width - scroll_left - scroll_right;
}

lv_coord_t lv_chart_get_window_height(lv_obj_t *obj) {
    lv_coord_t self_height = lv_obj_get_self_height(obj);
    lv_coord_t scroll_top = lv_obj_get_scroll_top(obj);
    lv_coord_t scroll_bottom = lv_obj_get_scroll_bottom(obj);
    return  self_height - scroll_top - scroll_bottom;
}



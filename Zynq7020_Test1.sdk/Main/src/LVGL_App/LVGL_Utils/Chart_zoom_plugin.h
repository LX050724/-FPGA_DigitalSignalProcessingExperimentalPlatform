//
// Created by yaoji on 2022/4/14.
//

#ifndef ZYNQ7020_LV_CHART_ZOOM_PLUGIN_H
#define ZYNQ7020_LV_CHART_ZOOM_PLUGIN_H

#include "lvgl.h"

void lv_chart_install_zoom_plugin(lv_obj_t *obj);
void lv_chart_zoom_slider_x_cb(lv_event_t *e);
void lv_chart_zoom_slider_y_cb(lv_event_t *e);

lv_coord_t lv_chart_get_window_width(lv_obj_t *obj);
lv_coord_t lv_chart_get_window_height(lv_obj_t *obj);

#endif //ZYNQ7020_LV_CHART_ZOOM_PLUGIN_H

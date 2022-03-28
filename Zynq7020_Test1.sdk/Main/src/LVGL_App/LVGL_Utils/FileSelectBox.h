//
// Created by yaoji on 2022/3/28.
//

#ifndef ZYNQ7020_FILESELECTBOX_H
#define ZYNQ7020_FILESELECTBOX_H

#include "lvgl.h"

typedef void (*lv_file_sel_click_cb_t)(lv_obj_t *self, const char *path, const char *filename);
typedef void (*lv_file_sel_path_change_cb_t)(lv_obj_t *self, const char *path);

lv_obj_t *lv_file_select_box_create(lv_obj_t *parent);
void lv_file_select_box_set_click_callback(lv_obj_t *obj, lv_file_sel_click_cb_t callback);
void lv_file_select_box_set_path_change_callback(lv_obj_t *obj, lv_file_sel_path_change_cb_t callback);
void lv_file_select_box_set_dir(lv_obj_t *obj, const char *dir);
const char *lv_file_select_box_get_selected_file(lv_obj_t *obj);

#endif //ZYNQ7020_FILESELECTBOX_H

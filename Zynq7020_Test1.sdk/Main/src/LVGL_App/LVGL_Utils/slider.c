//
// Created by yaoji on 2022/1/26.
//

#include "slider.h"

static void slider_decrement_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_obj_t *slider = lv_event_get_user_data(e);
        int32_t value = lv_slider_get_value(slider) - ((code == LV_EVENT_SHORT_CLICKED) ? 1 : 10);
        lv_slider_set_value(slider, value, LV_ANIM_OFF);
        lv_event_send(slider, LV_EVENT_VALUE_CHANGED, NULL);
    }
}

static void slider_increment_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_obj_t *slider = lv_event_get_user_data(e);
        int32_t value = lv_slider_get_value(slider) + ((code == LV_EVENT_SHORT_CLICKED) ? 1 : 10);
        lv_slider_set_value(slider, value, LV_ANIM_OFF);
        lv_event_send(slider, LV_EVENT_VALUE_CHANGED, NULL);
    }
}

static void slider_value_changed_cb(lv_event_t *e) {
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_t *value_label = lv_event_get_user_data(e);
    const char* fmt = lv_obj_get_user_data(value_label);
    int32_t value = lv_slider_get_value(obj);
    lv_label_set_text_fmt(value_label, fmt, value);
}


lv_obj_t *slider_create(lv_obj_t *parent, lv_obj_t *align_base, const char* fmt, int32_t min, int32_t max) {
    lv_obj_t *sub_btn = lv_btn_create(parent);
    lv_obj_set_size(sub_btn, 50, 50);
    lv_obj_align_to(sub_btn, align_base, LV_ALIGN_LEFT_MID, LV_HOR_RES * 0.15, 0);
    lv_obj_set_style_bg_img_src(sub_btn, LV_SYMBOL_MINUS, 0);

    lv_obj_t *slider = lv_slider_create(parent);
    lv_slider_set_range(slider, min, max);
    lv_obj_add_flag(slider, LV_OBJ_FLAG_ADV_HITTEST);
    lv_obj_set_width(slider, 400);
    lv_obj_align_to(slider, sub_btn, LV_ALIGN_OUT_RIGHT_MID, 15, 0);

    lv_obj_t *value_label = lv_label_create(parent);
    lv_label_set_text_fmt(value_label, fmt, lv_slider_get_value(slider));
    lv_obj_set_user_data(value_label, fmt);
    lv_obj_align_to(value_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    lv_obj_t *add_btn = lv_btn_create(parent);
    lv_obj_set_size(add_btn, 50, 50);
    lv_obj_align_to(add_btn, slider, LV_ALIGN_OUT_RIGHT_MID, 15, 0);
    lv_obj_set_style_bg_img_src(add_btn, LV_SYMBOL_PLUS, 0);
    lv_obj_add_event_cb(add_btn, slider_increment_cb, LV_EVENT_ALL, slider);
    lv_obj_add_event_cb(sub_btn, slider_decrement_cb, LV_EVENT_ALL, slider);
    lv_obj_add_event_cb(slider, slider_value_changed_cb, LV_EVENT_VALUE_CHANGED, value_label);

    return slider;
}
//
// Created by yaoji on 2022/1/24.
//

#include "spinbox.h"


/**
 * spinbox数值改变回调函数
 * @param e
 */
static void spinbox_value_changed_cb(lv_event_t *e) {
    lv_obj_t *spinbox = lv_event_get_target(e);
    uint32_t *p = lv_event_get_user_data(e);
    if (p != NULL) {
        if (spinbox->user_data) {
            *p = lv_spinbox_get_value(spinbox);
        } else {
            void (*fun)(uint32_t) = p;
            fun(lv_spinbox_get_value(spinbox));
        }
    }
}

/**
 *
 * @param e
 */
static void lv_spinbox_increment_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *spinbox = lv_event_get_user_data(e);
    if (code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_spinbox_increment(spinbox);
    }
}

/**
 *
 * @param e
 */
static void lv_spinbox_decrement_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *spinbox = lv_event_get_user_data(e);
    if (code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_spinbox_decrement(spinbox);
    }
}

/**
 *
 * @param parent 父对象
 * @param last_obj 上一个对象
 * @param style 样式
 * @param align 对齐方式 仅支持 {@code LV_ALIGN_OUT_BOTTOM_LEFT} 和 {@code LV_ALIGN_OUT_LEFT_MID}
 * @param text 标签文本
 * @param range_min 数值区间下限
 * @param range_max 数值区间上限
 * @param digit_count 数字位数
 * @param separator_position 小数点位置
 * @param ptr 事件返回数据指针或回调函数指针
 * @param data_or_fun 数据或回调函数选择 @var true 数据指针, @var false 回调函数指针
 * @return
 */
lv_obj_t *spinbox_create(lv_obj_t *parent, lv_obj_t *last_obj, lv_style_t *style, lv_align_t align,
                         const char *text, int32_t range_min, int32_t range_max,
                         uint8_t digit_count, uint8_t separator_position, void *ptr, uint8_t data_or_fun) {
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text_static(label, text);
    if (style) lv_obj_add_style(label, style, 0);
    if (last_obj != NULL) {
        if (align == LV_ALIGN_OUT_BOTTOM_LEFT)
            lv_obj_align_to(label, last_obj, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 30);
        else lv_obj_align_to(label, last_obj, LV_ALIGN_OUT_LEFT_MID, LV_HOR_RES * 0.5, 0);
    }

    lv_obj_t *spinbox = lv_spinbox_create(parent);
    lv_spinbox_set_range(spinbox, range_min, range_max);
    lv_spinbox_set_digit_format(spinbox, digit_count, separator_position);
    lv_spinbox_step_prev(spinbox);
    lv_obj_set_width(spinbox, 100);
    lv_obj_align_to(spinbox, label, LV_ALIGN_OUT_LEFT_MID, LV_HOR_RES * 0.3, 0);

    lv_coord_t height = lv_obj_get_height(spinbox);

    lv_obj_t *add_btn = lv_btn_create(parent);
    lv_obj_set_size(add_btn, height, height);
    lv_obj_align_to(add_btn, spinbox, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_obj_set_style_bg_img_src(add_btn, LV_SYMBOL_PLUS, 0);
    lv_obj_add_event_cb(add_btn, lv_spinbox_increment_event_cb, LV_EVENT_ALL, spinbox);

    lv_obj_t *sub_btn = lv_btn_create(parent);
    lv_obj_set_size(sub_btn, height, height);
    lv_obj_align_to(sub_btn, spinbox, LV_ALIGN_OUT_LEFT_MID, -5, 0);
    lv_obj_set_style_bg_img_src(sub_btn, LV_SYMBOL_MINUS, 0);
    lv_obj_add_event_cb(sub_btn, lv_spinbox_decrement_event_cb, LV_EVENT_ALL, spinbox);

    lv_obj_add_event_cb(spinbox, spinbox_value_changed_cb, LV_EVENT_VALUE_CHANGED, ptr);
    spinbox->user_data = data_or_fun;
    return label;
}

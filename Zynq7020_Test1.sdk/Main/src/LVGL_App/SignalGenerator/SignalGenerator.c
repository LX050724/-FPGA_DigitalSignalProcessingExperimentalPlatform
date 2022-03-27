#include "SignalGenerator.h"

#include "FileDecoder/FileDecoder.h"
#include "Fatfs_init/Fatfs_Driver.h"
#include "FreeRTOS_Mem/FreeRTOS_Mem.h"
#include "LVGL_Utils/spinbox.h"
#include "Controller/DDS_Controller.h"
#include "xsdps.h"

#include "SignalGenerator_fromFile.h"

static DDS_sine_t DDS_sine = {.base = {.type = TYPE_SINE, .freq = 30}};
static DDS_triangle_t DDS_triangle = {.base = {.type = TYPE_TRIANGLE, .freq = 30}};
static DDS_falling_ramp_t DDS_falling_ramp = {.base = {.type = TYPE_FALLING_RAMP, .freq = 30}};
static DDS_rising_ramp_t DDS_rising_ramp = {.base = {.type = TYPE_RISING_RAMP, .freq = 30}};
static DDS_square_t DDS_square = {.base = {.type = TYPE_SQUARE, .freq = 30}};
static DDS_stair_step_t DDS_stair_step = {.base = {.type = TYPE_STAIR_STEP, .freq = 30}, .falling = 1, .rising = 1};

static lv_style_t style_label;

static void sine_create(lv_obj_t *parent);
static void triangle_create(lv_obj_t *parent);
static void falling_ramp_create(lv_obj_t *parent);
static void rising_ramp_create(lv_obj_t *parent);
static void square_create(lv_obj_t *parent);
static void stair_step_create(lv_obj_t *parent);
static void start_btn_click_cb(lv_event_t *event);

void SignalGenerator_create(lv_obj_t *parent) {
    lv_style_init(&style_label);
    lv_style_set_text_font(&style_label, &msyhl_24);

    lv_obj_t *tab_view = lv_tabview_create(parent, LV_DIR_LEFT, 100);

    sine_create(lv_tabview_add_tab(tab_view, "正弦波"));
    triangle_create(lv_tabview_add_tab(tab_view, "三角波"));
    square_create(lv_tabview_add_tab(tab_view, "方波"));
    rising_ramp_create(lv_tabview_add_tab(tab_view, "上升斜锯齿波"));
    falling_ramp_create(lv_tabview_add_tab(tab_view, "下降斜锯齿波"));
    stair_step_create(lv_tabview_add_tab(tab_view, "梯形台阶波"));
    fromFile_create(lv_tabview_add_tab(tab_view, "文件"));
}


/**
 * 公共函数部分
 */


/**
 * 创建启动波形输出按钮
 * @param parent 父对象指针
 * @param user_data 波形类型及参数指针
 */
static void start_btn_create(lv_obj_t *parent, void *user_data) {
    lv_obj_t *start_btn = lv_btn_create(parent);
    lv_obj_add_flag(start_btn, LV_OBJ_FLAG_FLOATING);
    lv_obj_align(start_btn, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
    lv_obj_t *start_btn_label = lv_label_create(start_btn);
    lv_obj_add_style(start_btn_label, &style_label, 0);
    lv_label_set_text_static(start_btn_label, "启动输出");
    lv_obj_add_event_cb(start_btn, start_btn_click_cb, LV_EVENT_CLICKED, user_data);
}

static void start_btn_click_cb(lv_event_t *event) {
    if (event->user_data) {
        DDS_wav_generator(event->user_data);
    }
}

/**
 * 正弦波选项卡部分
 */
static void sine_create(lv_obj_t *parent) {
    lv_obj_t *obj_left = spinbox_create(parent, NULL, &style_label, 0, "频率(Hz):",
                                        30, 5000000, 7, 0, &DDS_sine.base.freq, SPINBOX_DATA_PRT);

    spinbox_create(parent, obj_left, &style_label, LV_ALIGN_OUT_BOTTOM_LEFT, "峰峰值(V):",
                   0, 10000, 5, 2, &DDS_sine.base.amplitude, SPINBOX_DATA_PRT);

    lv_obj_t *obj_right = spinbox_create(parent, obj_left, &style_label, LV_ALIGN_OUT_LEFT_MID, "偏移(V):",
                                         -5000, 5000, 4, 1, &DDS_sine.base.offset, SPINBOX_DATA_PRT);

    spinbox_create(parent, obj_right, &style_label, LV_ALIGN_OUT_BOTTOM_LEFT, "相位(deg):",
                   -1800, 1800, 4, 3, &DDS_sine.base.phase, SPINBOX_DATA_PRT);

    start_btn_create(parent, &DDS_sine);
}

static void triangle_create(lv_obj_t *parent) {
    lv_obj_t *obj_left = spinbox_create(parent, NULL, &style_label, 0, "频率(Hz):",
                                        30, 5000000, 7, 0, &DDS_triangle.base.freq, SPINBOX_DATA_PRT);

    spinbox_create(parent, obj_left, &style_label, LV_ALIGN_OUT_BOTTOM_LEFT, "峰峰值(V):",
                   0, 10000, 5, 2, &DDS_triangle.base.amplitude, SPINBOX_DATA_PRT);

    lv_obj_t *obj_right = spinbox_create(parent, obj_left, &style_label, LV_ALIGN_OUT_LEFT_MID, "偏移(V):",
                                         -5000, 5000, 4, 1, &DDS_triangle.base.offset, SPINBOX_DATA_PRT);

    spinbox_create(parent, obj_right, &style_label, LV_ALIGN_OUT_BOTTOM_LEFT, "相位(deg):",
                   -1800, 1800, 4, 3, &DDS_triangle.base.phase, SPINBOX_DATA_PRT);


    start_btn_create(parent, &DDS_triangle);
}

static void falling_ramp_create(lv_obj_t *parent) {
    lv_obj_t *obj_left = spinbox_create(parent, NULL, &style_label, 0, "频率(Hz):",
                                        30, 5000000, 7, 0, &DDS_falling_ramp.base.freq, SPINBOX_DATA_PRT);

    spinbox_create(parent, obj_left, &style_label, LV_ALIGN_OUT_BOTTOM_LEFT, "峰峰值(V):",
                   0, 10000, 5, 2, &DDS_falling_ramp.base.amplitude, SPINBOX_DATA_PRT);

    lv_obj_t *obj_right = spinbox_create(parent, obj_left, &style_label, LV_ALIGN_OUT_LEFT_MID, "偏移(V):",
                                         -5000, 5000, 4, 1, &DDS_falling_ramp.base.offset, SPINBOX_DATA_PRT);

    spinbox_create(parent, obj_right, &style_label, LV_ALIGN_OUT_BOTTOM_LEFT, "相位(deg):",
                   -1800, 1800, 4, 3, &DDS_falling_ramp.base.phase, SPINBOX_DATA_PRT);


    start_btn_create(parent, &DDS_falling_ramp);
}

static void rising_ramp_create(lv_obj_t *parent) {
    lv_obj_t *obj_left = spinbox_create(parent, NULL, &style_label, 0, "频率(Hz):",
                                        30, 5000000, 7, 0, &DDS_rising_ramp.base.freq, SPINBOX_DATA_PRT);

    spinbox_create(parent, obj_left, &style_label, LV_ALIGN_OUT_BOTTOM_LEFT, "峰峰值(V):",
                   0, 10000, 5, 2, &DDS_rising_ramp.base.amplitude, SPINBOX_DATA_PRT);

    lv_obj_t *obj_right = spinbox_create(parent, obj_left, &style_label, LV_ALIGN_OUT_LEFT_MID, "偏移(V):",
                                         -5000, 5000, 4, 1, &DDS_rising_ramp.base.offset, SPINBOX_DATA_PRT);

    spinbox_create(parent, obj_right, &style_label, LV_ALIGN_OUT_BOTTOM_LEFT, "相位(deg):",
                   -1800, 1800, 4, 3, &DDS_rising_ramp.base.phase, SPINBOX_DATA_PRT);


    start_btn_create(parent, &DDS_rising_ramp);
}

static void square_create(lv_obj_t *parent) {
    lv_obj_t *obj_left = spinbox_create(parent, NULL, &style_label, 0, "频率(Hz):",
                                        30, 5000000, 7, 0, &DDS_square.base.freq, SPINBOX_DATA_PRT);

    lv_obj_t *obj_left_2 = spinbox_create(parent, obj_left, &style_label, LV_ALIGN_OUT_BOTTOM_LEFT, "峰峰值(V):",
                                          0, 10000, 5, 2, &DDS_square.base.amplitude, SPINBOX_DATA_PRT);

    lv_obj_t *obj_right = spinbox_create(parent, obj_left, &style_label, LV_ALIGN_OUT_LEFT_MID, "偏移(V):",
                                         -5000, 5000, 4, 1, &DDS_square.base.offset, SPINBOX_DATA_PRT);

    spinbox_create(parent, obj_right, &style_label, LV_ALIGN_OUT_BOTTOM_LEFT, "相位(deg):",
                   -1800, 1800, 4, 3, &DDS_square.base.phase, SPINBOX_DATA_PRT);

    spinbox_create(parent, obj_left_2, &style_label, LV_ALIGN_OUT_BOTTOM_LEFT, "占空比(%):",
                   0, 1000, 4, 3, &DDS_square.duty_cycle, SPINBOX_DATA_PRT);

    start_btn_create(parent, &DDS_square);
}

static void stair_step_create(lv_obj_t *parent) {
    lv_obj_t *obj_left = spinbox_create(parent, NULL, &style_label, 0, "频率(Hz):",
                                        30, 5000000, 7, 0, &DDS_stair_step.base.freq, SPINBOX_DATA_PRT);

    lv_obj_t *obj_left_2 = spinbox_create(parent, obj_left, &style_label, LV_ALIGN_OUT_BOTTOM_LEFT, "峰峰值(V):",
                                          0, 10000, 5, 2, &DDS_stair_step.base.amplitude, SPINBOX_DATA_PRT);

    lv_obj_t *obj_right = spinbox_create(parent, obj_left, &style_label, LV_ALIGN_OUT_LEFT_MID, "偏移(V):",
                                         -5000, 5000, 4, 1, &DDS_stair_step.base.offset, SPINBOX_DATA_PRT);

    lv_obj_t *obj_right_2 = spinbox_create(parent, obj_right, &style_label, LV_ALIGN_OUT_BOTTOM_LEFT, "相位(deg):",
                                           -1800, 1800, 4, 3, &DDS_stair_step.base.phase, SPINBOX_DATA_PRT);

    spinbox_create(parent, obj_left_2, &style_label, LV_ALIGN_OUT_BOTTOM_LEFT, "上升沿个数:",
                   1, 256, 3, 0, &DDS_stair_step.rising, SPINBOX_DATA_PRT);

    spinbox_create(parent, obj_right_2, &style_label, LV_ALIGN_OUT_BOTTOM_LEFT, "下降沿个数:",
                   1, 256, 3, 0, &DDS_stair_step.falling, SPINBOX_DATA_PRT);

    start_btn_create(parent, &DDS_stair_step);
}

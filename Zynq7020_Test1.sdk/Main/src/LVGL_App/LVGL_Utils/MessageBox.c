#include "MessageBox.h"

#include "lvgl.h"
#include <stdarg.h>

static uint8_t style_init = 0;
static lv_style_t style_title;

static void QuestEvent_cb(lv_event_t *e) {
    lv_obj_t *obj = lv_event_get_current_target(e);
    void (*callback)(uint16_t, void *) = obj->user_data;
    if (callback) callback(lv_msgbox_get_active_btn(obj), e->user_data);
    lv_msgbox_close(obj);
}

static void InfoEvent_cb(lv_event_t *e) {
    lv_obj_t *obj = lv_event_get_current_target(e);
    lv_msgbox_close(obj);
}

/**
 * @brief
 *
 * @param title
 * @param fmt
 * @param YesText
 * @param NoText
 * @param callback 回调函数，参数为触发的按键序号, 0: Yes, 1: No
 */
lv_obj_t *
MessageBox_question(const char *title, const char *YesText, const char *NoText, void (*callback)(uint16_t, void *),
                    void *userdata, const char *fmt, ...) {
    static const char *btns[3] = {NULL, NULL, ""};
    char buf[128] = {0};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    if (!style_init) {
        lv_style_init(&style_title);
        lv_style_set_text_color(&style_title, lv_palette_main(LV_PALETTE_LIGHT_BLUE));
        lv_style_set_text_font(&style_title, &msyhl_24);
        style_init = 1;
    }
    btns[0] = YesText;
    btns[1] = NoText;
    lv_obj_t *mbox1 = lv_msgbox_create(NULL, title, buf, btns, false);
    lv_obj_add_style(lv_msgbox_get_title(mbox1), &style_title, 0);
    mbox1->user_data = callback;
    lv_obj_add_event_cb(mbox1, QuestEvent_cb, LV_EVENT_VALUE_CHANGED, userdata);
    lv_obj_set_width(mbox1, LV_HOR_RES / 2);
    lv_obj_center(mbox1);
    return mbox1;
}

lv_obj_t *MessageBox_info(const char *title, const char *YesText, const char *fmt, ...) {
    char buf[128] = {0};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    static const char *btns[3] = {NULL, "", ""};
    if (!style_init) {
        lv_style_init(&style_title);
        lv_style_set_text_color(&style_title, lv_palette_main(LV_PALETTE_LIGHT_BLUE));
        lv_style_set_text_font(&style_title, &msyhl_24);
        style_init = 1;
    }
    btns[0] = YesText;
    lv_obj_t *mbox1 = lv_msgbox_create(NULL, title, buf, btns, false);
    lv_obj_add_style(lv_msgbox_get_title(mbox1), &style_title, 0);
    lv_obj_add_event_cb(mbox1, InfoEvent_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_width(mbox1, LV_HOR_RES / 2);
    lv_obj_center(mbox1);
    return mbox1;
}

lv_obj_t *MessageBox_wait(const char *title, const char *fmt, ...) {
    static const char *buttons[] = {"", "", ""};
    char buf[128] = {0};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    lv_obj_t *messagebox = lv_msgbox_create(NULL, title, buf, buttons, false);
    lv_obj_t *title_obj = lv_msgbox_get_title(messagebox);
    lv_obj_t *content = lv_msgbox_get_content(messagebox);
    lv_obj_t *spinner = lv_spinner_create(content, 1000, 45);
    lv_obj_align_to(spinner, lv_msgbox_get_text(messagebox), LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    lv_obj_center(messagebox);
    lv_obj_add_style(title_obj, &style_title, 0);
    lv_obj_set_width(title_obj, LV_HOR_RES / 2);
    return messagebox;
}
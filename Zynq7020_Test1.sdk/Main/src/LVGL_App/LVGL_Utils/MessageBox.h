#ifndef MESSAGEBOX_H_
#define MESSAGEBOX_H_

#include "lvgl.h"

lv_obj_t *MessageBox_question(const char *title, const char *YesText, const char *NoText,
                              void (*callback)(uint16_t, void *), void *userdata, const char *fmt, ...);

lv_obj_t *MessageBox_info(const char *title, const char *YesText, const char *fmt, ...);

lv_obj_t *MessageBox_wait(const char *title, const char *fmt, ...);
#endif

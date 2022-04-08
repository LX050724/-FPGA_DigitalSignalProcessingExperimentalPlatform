#ifndef MESSAGEBOX_H_
#define MESSAGEBOX_H_

#include "xil_types.h"

void QuestMessageBox(const char *title, const char *YesText, const char *NoText, void (*callback)(uint16_t, void *),
                     void *userdata, const char *fmt, ...);
void InfoMessageBox(const char *title, const char *YesText, const char *fmt, ...);

#endif

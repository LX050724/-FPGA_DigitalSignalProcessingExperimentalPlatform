#ifndef MESSAGEBOX_H_
#define MESSAGEBOX_H_

#include "xil_types.h"

void QuestMessageBox(const char *title, const char *text, const char *YesText, const char *NoText,
                     void (*callback)(uint16_t));
void InfoMessageBox(const char *title, const char *text, const char *YesText);

#endif

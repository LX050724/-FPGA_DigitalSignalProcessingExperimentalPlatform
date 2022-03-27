#ifndef SNTP_USER_H_
#define SNTP_USER_H_

#include "lwip/apps/sntp.h"

void sntp_start();
void sntp_user_set_rtc_time(time_t second);

#endif


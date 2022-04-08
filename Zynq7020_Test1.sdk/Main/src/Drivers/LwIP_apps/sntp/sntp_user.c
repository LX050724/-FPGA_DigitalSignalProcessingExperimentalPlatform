#include "sntp_user.h"
#include "lwip/inet.h"
#include "DS1337_Driver/DS1337_Driver.h"
#include "Fatfs_init/Fatfs_Driver.h"

void sntp_start() {
    ip_addr_t sntp_server = {0};
    inet_aton("203.107.6.88", &sntp_server);
    sntp_stop();
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setserver(0, &sntp_server);
    sntp_init();
}

void sntp_user_set_rtc_time(time_t second) {
    second += 8 * 3600;
    struct tm *t = localtime(&second);
    xil_printf("ntp time is %2d:%02d:%02d %4d-%2d-%2d %s\r\n", t->tm_hour, t->tm_min,
               t->tm_sec, t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, DS1337_WeekStr(t->tm_wday));
    DS1337_SetTime(NULL, t);
}

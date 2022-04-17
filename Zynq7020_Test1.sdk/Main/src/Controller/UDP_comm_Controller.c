/**
 * @file UDP_comm_Controller.c
 * @brief 协议格式 [状态 1 byte][ID 1 byte][DATA n byte......]
 */

#include <string.h>
#include "UDP_comm_Controller.h"
#include "SystemConfig/SystemConfig.h"
#include "Fatfs_init/Fatfs_Driver.h"
#include "cJSON.h"

static struct pbuf *get_firmware_version_id0(struct pbuf *p) {
    LWIP_UNUSED_ARG(p);
    const char *version = getFirmwareVersion();
    return send_data(0, version, strlen(version) + 1);
}

static struct pbuf *get_filename_id1(struct pbuf *p) {
    DIR dir;
    cJSON *array = cJSON_CreateArray();
    if (array == NULL) goto err;

    char *data = p->payload;
    if (data[p->len - 1] != 0) goto err;

    char *gbk = UTF8_TO_GBK(data + 1);
    FRESULT res = f_opendir(&dir, gbk);
    os_free(gbk);
    if (res != FR_OK) goto err;

    while (1) {
        FILINFO file_info;
        res = f_readdir(&dir, &file_info);
        if (res != FR_OK || file_info.fname[0] == 0) break;
        char *utf8 = GBK_TO_UTF8(file_info.fname);
        cJSON_AddItemToArray(array, cJSON_CreateString(utf8));
        os_free(utf8);
    }
    f_closedir(&dir);

    char *json_str = cJSON_PrintUnformatted(array);
    if (json_str == NULL) goto err;
    printf("%s\r\n", json_str);
    struct pbuf *ret = send_data(1, json_str, strlen(json_str));
    cJSON_Delete(array);
    cJSON_free(json_str);
    return ret;
    err:
    cJSON_Delete(array);
    return send_err(4, 1);
}

void udp_comm_controller_init() {
    udp_comm_RegMegProcessor(0, get_firmware_version_id0);
    udp_comm_RegMegProcessor(1, get_filename_id1);
}

//
// Created by yaoji on 2022/4/9.
//

#include <string.h>
#include "UDP_comm_Controller.h"
#include "SystemConfig/SystemConfig.h"

static struct pbuf *get_firmware_version_id0(struct pbuf *p) {
    const char *version = getFirmwareVersion();
    struct pbuf *ret_buf = pbuf_alloc(PBUF_TRANSPORT, strlen(version) + 2, PBUF_RAM);
    uint8_t *data = ret_buf->payload;
    data[0] = UDP_COMM_ACK;
    strcpy((char *)data + 1, version);
    return ret_buf;
}

void udp_comm_controller_init() {
    udp_comm_RegMegProcessor(0, get_firmware_version_id0);
}

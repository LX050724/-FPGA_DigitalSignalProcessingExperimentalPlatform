//
// Created by yaoji on 2022/4/9.
//

#ifndef ZYNQ7020_UDP_COMM_H
#define ZYNQ7020_UDP_COMM_H

#include <lwip/udp.h>

enum {
    UDP_COMM_ACK,
    UDP_COMM_ERR,
    UDP_COMM_NO_MSG_ID,
} UDP_COMM_CMD_CODE;

void udp_comm_start();
void udp_comm_RegMegProcessor(uint8_t message_id, struct pbuf *(*fun)(struct pbuf *));

#endif //ZYNQ7020_UDP_COMM_H

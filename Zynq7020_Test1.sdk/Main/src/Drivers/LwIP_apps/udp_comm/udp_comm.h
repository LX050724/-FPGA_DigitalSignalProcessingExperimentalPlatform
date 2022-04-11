//
// Created by yaoji on 2022/4/9.
//

#ifndef ZYNQ7020_UDP_COMM_H
#define ZYNQ7020_UDP_COMM_H

#include <lwip/udp.h>

enum {
    UDP_COMM_ACK = 1,
    UDP_COMM_ERR = 2,
    UDP_COMM_NO_MSG_ID = 3,
} UDP_COMM_CMD_CODE;

void udp_comm_start();
void udp_comm_RegMegProcessor(uint8_t message_id, struct pbuf *(*fun)(struct pbuf *));
struct pbuf *send_err(uint8_t err_id, uint8_t id);
struct pbuf *send_data(uint8_t id, const void *data, int len);

#endif //ZYNQ7020_UDP_COMM_H

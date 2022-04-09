//
// Created by yaoji on 2022/4/9.
//

#include "udp_comm.h"
#include "utils/Array.h"

#define UDP_COMM_PORT 70
#define UDP_COMM_RET_HEAD (0xff)

typedef struct {
    NodeBase parent;
    uint8_t message_id;
    struct pbuf *(*fun)(struct pbuf *);
} MsgProcessor;

static struct udp_pcb *udpPcb;
static void udp_comm_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                          const ip_addr_t *addr, u16_t port);

Array processorList;

void udp_comm_start() {
    udpPcb = udp_new_ip_type(IPADDR_TYPE_ANY);
    udp_bind(udpPcb, IP_ANY_TYPE, UDP_COMM_PORT);
    udp_recv(udpPcb, udp_comm_recv, NULL);
}

void udp_comm_RegMegProcessor(uint8_t message_id, struct pbuf *(*fun)(struct pbuf *)) {
    if (message_id == 0xff) return; //保留id
    MsgProcessor processor = {
            .fun = fun,
            .message_id = message_id,
            .parent.destructor = NULL,
    };
    Array_push(&processorList, &processor, sizeof(MsgProcessor));
}

static void udp_comm_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    uint8_t message_id = ((uint8_t *) p->payload)[0];
    for (int i = 0; i < processorList.len; i++) {
        MsgProcessor *processor = Array_get(&processorList, i);
        if (processor != NULL && processor->message_id == message_id) {
            if (processor->fun) {
                struct pbuf *tx_buf = processor->fun(p);
                if (tx_buf) {
                    udp_sendto(pcb, tx_buf, addr, port);
                    pbuf_free(tx_buf);
                }
                goto ret;
            }
        }
    }
    struct pbuf *ret_ubf = pbuf_alloc(PBUF_TRANSPORT, 2, PBUF_RAM);
    ((uint8_t *)ret_ubf->payload)[0] = UDP_COMM_RET_HEAD;
    ((uint8_t *)ret_ubf->payload)[1] = UDP_COMM_NO_MSG_ID;
    udp_sendto(pcb, ret_ubf, addr, port);
    pbuf_free(ret_ubf);
    ret:
    pbuf_free(p);
}
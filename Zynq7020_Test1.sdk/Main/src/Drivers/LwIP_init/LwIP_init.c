#include "FreeRTOS.h"
#include "task.h"
#include "lwip/dhcp.h"
#include "lwip/inet.h"
#include "lwip/init.h"
#include "lwip/sys.h"
#include "netif/xadapter.h"
#include "platform_config.h"
#include "utils.h"
#include "xparameters.h"
#include "LwIP_apps/sntp/sntp_user.h"
#include "xil_printf.h"
#include "netif/xemacpsif.h"

extern volatile int dhcp_timoutcntr;
err_t dhcp_start(struct netif *netif);

#define DEFAULT_IP_ADDRESS "192.168.20.10"
#define DEFAULT_IP_MASK "255.255.255.0"
#define DEFAULT_GW_ADDRESS "192.168.20.1"

struct netif server_netif;

static int complete_nw_thread;
static TaskHandle_t network_init_thread_handle;

#define THREAD_STACKSIZE 1024

static void print_ip(char *msg, ip_addr_t *ip) {
    xil_printf(msg);
    xil_printf("%d.%d.%d.%d\r\n", ip4_addr1(ip), ip4_addr2(ip), ip4_addr3(ip), ip4_addr4(ip));
}

static void print_ip_settings(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw) {
    print_ip("Board IP:       ", ip);
    print_ip("Netmask :       ", mask);
    print_ip("Gateway :       ", gw);
}

static void assign_default_ip(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw) {
    int err;

    xil_printf("Configuring default IP %s \r\n", DEFAULT_IP_ADDRESS);

    err = inet_aton(DEFAULT_IP_ADDRESS, ip);
    if (!err) xil_printf("Invalid default IP address: %d\r\n", err);

    err = inet_aton(DEFAULT_IP_MASK, mask);
    if (!err) xil_printf("Invalid default IP MASK: %d\r\n", err);

    err = inet_aton(DEFAULT_GW_ADDRESS, gw);
    if (!err) xil_printf("Invalid default gateway address: %d\r\n", err);
}

static void network_thread(void *p) {
#if ((LWIP_IPV6 == 0) && (LWIP_DHCP == 1))
    int mscnt = 0;
#endif

    /* the mac address of the board. this should be unique per board */
    u8_t mac_ethernet_address[] = {0x00, 0x0a, 0x35, 0x00, 0x01, 0x02};

    /* Add network interface to the netif_list, and set it as default */
    if (!xemac_add(&server_netif, NULL, NULL, NULL, mac_ethernet_address, PLATFORM_EMAC_BASEADDR)) {
        xil_printf("Error adding N/W interface\r\n");
        return;
    }

#if LWIP_IPV6 == 1
    server_netif.ip6_autoconfig_enabled = 1;
    netif_create_ip6_linklocal_address(&server_netif, 1);
    netif_ip6_addr_set_state(&server_netif, 0, IP6_ADDR_VALID);
    print_ipv6("\n\rlink local IPv6 address is:", &server_netif.ip6_addr[0]);
#endif /* LWIP_IPV6 */

    netif_set_default(&server_netif);

    /* specify that the network if is up */
    netif_set_up(&server_netif);

    /* start packet receive thread - required for lwIP operation */
    sys_thread_new("xemacif_input_thread", (void (*)(void *)) xemacif_input_thread, &server_netif,
                   THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);

    complete_nw_thread = 1;

    /* Resume the main thread; auto-negotiation is completed */
    vTaskResume(network_init_thread_handle);

    dhcp_start(&server_netif);
    while (1) {
        vTaskDelay(DHCP_FINE_TIMER_MSECS / portTICK_RATE_MS);
        dhcp_fine_tmr();
        mscnt += DHCP_FINE_TIMER_MSECS;
        if (mscnt >= DHCP_COARSE_TIMER_SECS * 1000) {
            dhcp_coarse_tmr();
            mscnt = 0;
        }
    }
}

static void network_init_thread(void *p) {
#if ((LWIP_IPV6 == 0) && (LWIP_DHCP == 1))
    int mscnt = 0;
#endif

#ifdef XPS_BOARD_ZCU102
    IicPhyReset();
#endif
    /* initialize lwIP before calling sys_thread_new */
    lwip_init();

    /* any thread using lwIP should be created using sys_thread_new */
    sys_thread_new("nw_thread", network_thread, NULL, THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);

    /* Suspend Task until auto-negotiation is completed */
    if (!complete_nw_thread) vTaskSuspend(NULL);

#if LWIP_IPV6 == 0
#if LWIP_DHCP == 1
    while (1) {
        vTaskDelay(DHCP_FINE_TIMER_MSECS / portTICK_RATE_MS);
        if (server_netif.ip_addr.addr) {
            xil_printf("DHCP request success\r\n");
            break;
        }
        mscnt += DHCP_FINE_TIMER_MSECS;
        if (mscnt >= 10000) {
            xil_printf("ERROR: DHCP request timed out\r\n");
            assign_default_ip(&(server_netif.ip_addr), &(server_netif.netmask), &(server_netif.gw));
            break;
        }
    }

#else
    assign_default_ip(&(server_netif.ip_addr), &(server_netif.netmask), &(server_netif.gw));
#endif

    print_ip_settings(&(server_netif.ip_addr), &(server_netif.netmask), &(server_netif.gw));
#endif /* LWIP_IPV6 */
    sntp_start();
    vTaskDelete(NULL);
    return;
}

void network_init() {
    if (xTaskCreate(network_init_thread, "network init thread", THREAD_STACKSIZE, NULL,
                    DEFAULT_THREAD_PRIO, &network_init_thread_handle) != pdPASS) {
        xil_printf("error create network_init_thread failed\r\n");
    }
}

static uint16_t network_state() {
    if (netif_default == NULL) return 0;
    struct xemac_s *xemac = netif_default->state;
    if (xemac == NULL) return 0;
    xemacpsif_s *xemacpsif = xemac->state;
    if (xemacpsif == NULL) return 0;
    uint16_t tmp;
    XEmacPs_PhyRead(&xemacpsif->emacps, xemacpsif->emacps.Config.BaseAddress, 0x1A, &tmp);
    return tmp;
}

/**
 * 检测网线是否连接以及连接速度
 * @return 网线是否连接
 * @retval 0 无效
 * @retval >0 连接速度
 * @retval -1 未连接
 */
int network_linkSpeed() {
    uint16_t PHYSR = network_state();
    if ((PHYSR & 0x04) != 0) {
        switch ((PHYSR >> 4) & 0x03) {
            case 0:
                return 10;
            case 1:
                return 100;
            case 2:
                return 1000;
            case 3:
                return 0;
        }
    } else return -1;
}

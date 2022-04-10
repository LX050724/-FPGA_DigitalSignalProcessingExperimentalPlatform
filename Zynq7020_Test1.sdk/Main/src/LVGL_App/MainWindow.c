/*
 * MainWindow.c
 *
 *  Created on: 2021年12月31日
 *      Author: yaoji
 */
#include "LVGL_Zynq_Init/zynq_lvgl_init.h"
#include "DS1337_Driver/DS1337_Driver.h"
#include "SignalGenerator/SignalGenerator.h"
#include "Setup/Setup.h"
#include "SpectrumAnalyzer/SpectrumAnalyzer.h"
#include "Oscilloscope/Oscilloscope.h"
#include "DigitalFilter/DigitalFilter.h"
#include "MainWindow.h"
#include "LwIP_init/LwIP_init.h"
#include "lwip/ip4.h"


lv_obj_t *tabview;

static lv_style_t style_text_muted;
static lv_style_t style_title;
static lv_style_t style_icon;
static lv_style_t style_bullet;

static lv_label_t *dateLable;
static lv_timer_t *timer;

static const lv_font_t *font_large = &msyhl_24;
static const lv_font_t *font_normal = &msyhl_16;

void timer_cb(lv_timer_t *obj);

void mainWindowInit() {
    xSemaphoreTake(LVGL_Mutex, portMAX_DELAY);
    /* 设置主题 */
    lv_theme_default_init(NULL, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
                          LV_THEME_DEFAULT_DARK, font_normal);
    lv_style_init(&style_text_muted);
    lv_style_set_text_opa(&style_text_muted, LV_OPA_50);

    lv_style_init(&style_title);
    lv_style_set_text_font(&style_title, font_large);

    lv_style_init(&style_icon);
    lv_style_set_text_color(&style_icon, lv_theme_get_color_primary(NULL));
    lv_style_set_text_font(&style_icon, font_large);

    lv_style_init(&style_bullet);
    lv_style_set_border_width(&style_bullet, 0);
    lv_style_set_radius(&style_bullet, LV_RADIUS_CIRCLE);

    /* 创建顶部Tab */
    tabview = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 70);
    lv_obj_t *tab_btns = lv_tabview_get_tab_btns(tabview);
    /* 设置左半空白 */
    lv_obj_set_style_pad_left(tab_btns, LV_HOR_RES * 0.4, 0);

    /* 设置左上角LVGL图标 */
    lv_obj_t *logo = lv_img_create(tab_btns);
    LV_IMG_DECLARE(img_fpga_dsp_icon);
    lv_img_set_src(logo, &img_fpga_dsp_icon);
    lv_obj_align(logo, LV_ALIGN_LEFT_MID, -LV_HOR_RES * 0.4 + 25, 0);

    /* 创建左上角标题 */
    lv_obj_t *label = lv_label_create(tab_btns);
    lv_obj_add_style(label, &style_title, 0);

    lv_label_set_text(label, "FPGA 数字信号处理实验平台");
    lv_obj_align_to(label, logo, LV_ALIGN_OUT_RIGHT_TOP, 10, 0);

    /* 创建Tab标签 */
    lv_obj_t *tabs[] = {
            lv_tabview_add_tab(tabview, "示波器"),
            lv_tabview_add_tab(tabview, "频谱仪"),
            lv_tabview_add_tab(tabview, "幅频响应测试"),
            lv_tabview_add_tab(tabview, "数字滤波器"),
            lv_tabview_add_tab(tabview, "信号发生器"),
            lv_tabview_add_tab(tabview, "设置"),
    };

//    for (int i = 0; i < sizeof(tabs) / sizeof(lv_obj_t *); i++)
//        lv_page_set_scroll_propagation(tabs[i], false);
    Oscilloscope_create(tabs[0]);
    SpectrumAnalyzer_create(tabs[1]);
    DigitalFilter_create(tabs[3]);
    SignalGenerator_create(tabs[4]);
    Setup_create(tabs[5]);
    xSemaphoreGive(LVGL_Mutex);
}


void errShowIP(lv_timer_t *pTimer) {
    LV_UNUSED(pTimer);
    int speed = network_linkSpeed();
    if (speed > 0) {
        char ip_str[3][IP4ADDR_STRLEN_MAX];
        memcpy(ip_str[0], ip4addr_ntoa(&netif_default->ip_addr), IP4ADDR_STRLEN_MAX);
        memcpy(ip_str[1], ip4addr_ntoa(&netif_default->gw), IP4ADDR_STRLEN_MAX);
        memcpy(ip_str[2], ip4addr_ntoa(&netif_default->netmask), IP4ADDR_STRLEN_MAX);
        lv_label_set_text_fmt(pTimer->user_data, "IP: %s\ngateway: %s\nmask: %s\nMAC: %02X:%02X:%02X:%02X:%02X:%02X\nspeed: %dMbps",
                              ip_str[0], ip_str[1], ip_str[2],
                              netif_default->hwaddr[0], netif_default->hwaddr[1],
                              netif_default->hwaddr[2], netif_default->hwaddr[3],
                              netif_default->hwaddr[4], netif_default->hwaddr[5],
                              speed);
    } else {
        lv_label_set_text(pTimer->user_data, "network cable is not connected");
    }
    lv_obj_update_layout(pTimer->user_data);
}

void errWindowInit() {
    lv_obj_t *err_label = lv_label_create(lv_scr_act());
    lv_label_set_text_static(err_label, "Error: Font file missing, use \"UploadTool\" to repair");
    lv_obj_center(err_label);
    lv_obj_t *net_info = lv_label_create(lv_scr_act());
    lv_obj_align_to(net_info, err_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
    lv_timer_create(errShowIP, 500, net_info);
}

//
// Created by yaoji on 2022/3/27.
//

#include "AxisSwitch_Driver.h"
#include "utils.h"
#include <xaxis_switch.h>

XAxis_Switch xAxisSwitch;

int AxisSwitch_init() {
    XAxis_Switch_Config *config = XAxisScr_LookupConfig(XPAR_AXIS_SWITCH_0_DEVICE_ID);
    CHECK_STATUS_RET(XAxisScr_CfgInitialize(&xAxisSwitch, config, config->BaseAddress));
    XAxisScr_MiPortDisableAll(&xAxisSwitch);
    return XST_SUCCESS;
}

int AxisSwitch_switch(AxisSwitch_Channel channel) {
    XAxisScr_MiPortDisableAll(&xAxisSwitch);
    XAxisScr_MiPortEnable(&xAxisSwitch, channel, 0);
    return XAxisScr_IsMiPortEnabled(&xAxisSwitch, channel, 0) ? XST_SUCCESS : XST_FAILURE;
}


/*
 * ScuGic_Init.c
 *
 *  Created on: 2021年12月25日
 *      Author: yaoji
 */

#include "ScuGic_Driver/ScuGic_Driver.h"

#include "utils.h"

const uint32_t InterruptID_Table[16] = {61, 62, 63, 64, 65, 66, 67, 68,
                                        84, 85, 86, 87, 88, 89, 90, 91};

/**
 * 如果没有使用实时系统，自定义XScuGic对象，否则使用系统定义的对象
 */
#ifndef __USE_RTOS
XScuGic xInterruptController;
#endif

/**
 * 初始化ScuGic中断控制器，如果使用实时系统则不进行任何操作
 */
int ScuGic_Init() {
#ifndef __USE_RTOS
    XScuGic_Config* config = XScuGic_LookupConfig(XPAR_PS7_SCUGIC_0_DEVICE_ID);
    CHECK_STATUS_RET(XScuGic_CfgInitialize(&xInterruptController, config, config->CpuBaseAddress));
    CHECK_STATUS_RET(XScuGic_SelfTest(&xInterruptController));
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
                                 (Xil_ExceptionHandler)XScuGic_InterruptHandler,
                                 &xInterruptController);
    Xil_ExceptionEnable();
#endif
    return XST_SUCCESS;
}

int ScuGic_SetInterrupt(uint32_t Int_id, Xil_InterruptHandler Handler, void *CallBackRef,
                        uint8_t Priority, uint8_t Trigger) {
    CHECK_STATUS_RET(XScuGic_Connect(&xInterruptController, Int_id, Handler, CallBackRef));
    XScuGic_SetPriorityTriggerType(&xInterruptController, Int_id, Priority, Trigger);
    XScuGic_Enable(&xInterruptController, Int_id);
    return XST_SUCCESS;
}

uint32_t ScuGic_GetPLIntrId(uint8_t index) {
    if (index < sizeof(InterruptID_Table) / sizeof(uint32_t))
        return InterruptID_Table[index];
    else {
        xil_printf("ERROR out of range\r\n");
        return 0;
    }
}

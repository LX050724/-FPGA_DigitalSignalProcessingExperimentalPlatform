/*
 * Timer_Driver.c
 *
 *  Created on: 2021年12月26日
 *      Author: yaoji
 */

#include "Timer_Driver/Timer_Driver.h"

#include "ScuGic_Driver/ScuGic_Driver.h"
#include "utils.h"
#include "xil_types.h"
#include "xscutimer.h"
#include "xtime_l.h"

#ifndef __USE_RTOS

static XScuTimer ScuTimer;

static void TimerIntrHandler(void *CallBackRef) {
    XScuTimer_ClearInterruptStatus(&ScuTimer);
    void (*callBack_fun)(void) = CallBackRef;
    callBack_fun();
}

void Timer_start() { XScuTimer_Start(&ScuTimer); }

int Timer_Setup_Intr_System(uint8_t Priority, void *Handler) {
    CHECK_STATUS_RET(ScuGic_SetInterrupt(XPAR_SCUTIMER_INTR, TimerIntrHandler, Handler, Priority,
                                         INT_TYPE_RISING_EDGE));
    XScuTimer_EnableInterrupt(&ScuTimer);
    return XST_SUCCESS;
}

int Timer_init(uint32_t period_ms) {
    uint32_t Load_Value = COUNTS_PER_SECOND / 1e3 * period_ms - 1;
    XScuTimer_Config *config = XScuTimer_LookupConfig(XPAR_PS7_SCUTIMER_0_DEVICE_ID);
    CHECK_STATUS_RET(XScuTimer_CfgInitialize(&ScuTimer, config, config->BaseAddr));
    CHECK_STATUS_RET(XScuTimer_SelfTest(&ScuTimer));
    XScuTimer_LoadTimer(&ScuTimer, Load_Value);
    XScuTimer_EnableAutoReload(&ScuTimer);
    return XST_SUCCESS;
}

#endif

uint64_t getTime_millis() {
    XTime Tick;
    XTime_GetTime(&Tick);
    return Tick / (COUNTS_PER_SECOND / 1000);
}

/******************************************************************************
 *
 * Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
 ******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <Fatfs_init/Fatfs_Driver.h>
#include <VDMA_Driver/VDMA_Driver.h>

#include "LwIP_init/LwIP_init.h"
#include "DMA_Driver/DMA_Driver.h"
#include "DS1337_Driver/DS1337_Driver.h"
#include "Flash_Driver/qspi_g128_flash.h"
#include "FreeRTOS.h"
#include "FreeRTOS_Mem/FreeRTOS_Mem.h"
#include "GT911_Driver/GT911_Driver.h"
#include "I2C_Driver/I2C_Driver.h"
#include "LVGL_Zynq_Init/zynq_lvgl_init.h"
#include "MainWindow.h"
#include "ScuGic_Driver/ScuGic_Driver.h"
#include "Timer_Driver/Timer_Driver.h"
#include "lwip/init.h"
#include "platform.h"
#include "sleep.h"
#include "timers.h"
#include "check.h"
#include "xil_printf.h"
#include "AXI4_IO.h"
#include "Controller/FFT_Controller.h"
#include "Controller/ADC_Controller.h"
#include "Controller/DAC_Controller.h"
#include "XADC_Driver/XADC_Driver.h"
#include "cJSON.h"
#include "AmplitudeResponse/AmplitudeResponse.h"
#include "xtime_l.h"
#include "Controller/FIR_Controller.h"
#include "main.h"
#include "Controller/UDP_comm_Controller.h"

#include <math.h>
#include <malloc.h>

XIicPs iic0, iic1;
XGpioPs gpio;
XAxiDma dma0, dma1, dma2;
XQspiPs QspiInstance;
XAdcPs xAdcPs;

static XAxiDma_Bd DMA0_TxBd[64] __attribute__((aligned(64)));
static XAxiDma_Bd DMA0_RxBd[64] __attribute__((aligned(64)));

static TaskHandle_t DefaultTaskHandle;
static void DefaultTask(void *pvParameters);

static TaskHandle_t LED_TaskHandle;
static void LED_Task(void *pvParameters);

int main() {
    init_platform();

    XGpioPs_Config *config = XGpioPs_LookupConfig(XPAR_XGPIOPS_0_DEVICE_ID);
    CHECK_STATUS(XGpioPs_CfgInitialize(&gpio, config, config->BaseAddr));

    /**
     * 
     */
    xTaskCreate(DefaultTask,      /* The function that implements the task. */
                "DefaultTask",    /* Text name for the task, provided to assist debugging only. */
                2048,             /* The stack allocated to the task. */
                NULL,             /* The task parameter is not used, so set to NULL. */
                tskIDLE_PRIORITY, /* The task runs at the idle priority. */
                &DefaultTaskHandle);

    xTaskCreate(LED_Task,      /* The function that implements the task. */
                "LED_Task",    /* Text name for the task, provided to assist debugging only. */
                64,             /* The stack allocated to the task. */
                NULL,             /* The task parameter is not used, so set to NULL. */
                tskIDLE_PRIORITY, /* The task runs at the idle priority. */
                &LED_TaskHandle);

    vTaskStartScheduler();
    while (1);
    cleanup_platform();
    return 0;
}

void vApplicationTickHook(void) {
}

void vApplicationIdleHook(void) {

}

void vApplicationDaemonTaskStartupHook() {
}

static void DefaultTask(void *pvParameters) {
    vPortEnterCritical();
    cJSON_Hooks hooks = {
            .free_fn = os_free,
            .malloc_fn = os_malloc,
    };
    cJSON_InitHooks(&hooks);

    CHECK_STATUS(XADC_Init(&xAdcPs, XPAR_XADCPS_0_DEVICE_ID));

    CHECK_STATUS(Fatfs_Init());
    network_init();
    udp_comm_controller_init();

    CHECK_STATUS(Init_qspi(&QspiInstance, XPAR_PS7_QSPI_0_DEVICE_ID));

    CHECK_STATUS(I2C_Init(&iic0, XPAR_XIICPS_0_DEVICE_ID, 100e3));
    CHECK_STATUS(I2C_Init(&iic1, XPAR_XIICPS_1_DEVICE_ID, 400e3));
    DS1337_SetDefaultInstance(&iic1);

    CHECK_STATUS(DMA_Init(&dma0, XPAR_ADDA_AXI_DMA_AD_DA_DEVICE_ID));
    CHECK_STATUS(DMA_SetTxRing(&dma0, DMA0_TxBd, sizeof(DMA0_TxBd)));
    CHECK_STATUS(DMA_SetRxRing(&dma0, DMA0_RxBd, sizeof(DMA0_RxBd)));
    CHECK_STATUS(DAC_init_dma_channel(&dma0));
    CHECK_STATUS(ADC_init_dma_channel(&dma0));

    CHECK_STATUS(DMA_Init(&dma1, XPAR_ADDA_AXI_DMA_FFT_FIR_DEVICE_ID));
    CHECK_STATUS(FFT_init_dma_channel(&dma1));
    CHECK_STATUS(FIR_init_dma_channel(&dma1));

    CHECK_STATUS(ScuGic_Init());
    //	CHECK_STATUS(ScuGic_SetInterrupt(ScuGic_GetPLIntrId(3), kkk, NULL, INT_PRIORITY_160,
    // INT_TYPE_RISING_EDGE));

    CHECK_STATUS(VDMA_Init());
    CHECK_STATUS(VDMA_InitIntterupt(&xInterruptController, INT_PRIORITY_80, ScuGic_GetPLIntrId(0),
                                    zynq_disp_flush_ready));
    CHECK_STATUS(VDMA_Start());

    // ScuGic_SetInterrupt(ScuGic_GetPLIntrId(1), &dma0, XAxiDma_S2MMIrqHandler, INT_PRIORITY_80,
    // INT_TYPE_RISING_EDGE);
//    ScuGic_SetInterrupt(ScuGic_GetPLIntrId(2), XAxiDma_MM2SIntrHandler, &dma0, INT_PRIORITY_80,
//                        INT_TYPE_RISING_EDGE);

    int ret = zynq_lvgl_init(&iic0, &gpio);
    vPortExitCritical();
    if (ret == XST_SUCCESS) mainWindowInit();
    else errWindowInit();
    vTaskDelete(NULL);
}

static void LED_Task(void *pvParameters) {
    XGpioPs_SetDirectionPin(&gpio, LED_GPIO_PIN, GPIO_DIR_OUTPUT);
    XGpioPs_SetOutputEnablePin(&gpio, LED_GPIO_PIN, 1);
    for (;;) {
        XGpioPs_WritePin(&gpio, LED_GPIO_PIN, 1);
        vTaskDelay(10);
        XGpioPs_WritePin(&gpio, LED_GPIO_PIN, 0);
        vTaskDelay(20);
        XGpioPs_WritePin(&gpio, LED_GPIO_PIN, 1);
        vTaskDelay(10);
        XGpioPs_WritePin(&gpio, LED_GPIO_PIN, 0);
        vTaskDelay(100);
    }
}

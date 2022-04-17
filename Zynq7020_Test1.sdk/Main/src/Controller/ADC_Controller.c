//
// Created by yao on 2022/1/22.
//

#include <stdbool.h>
#include "ADC_Controller.h"
#include "xaxidma.h"
#include "SPU_Controller.h"
#include "check.h"
#include "FreeRTOS.h"
#include "task.h"
#include "math.h"
#include "FreeRTOS_Mem/FreeRTOS_Mem.h"

#define ADC_RawToVoltage_mV(AdcData) ((AdcData) * 10000 / 256)
#define TRIGGER_NUM_MAX 128

xSemaphoreHandle ADC_Mutex;

static XAxiDma_BdRing *RingPtr;
static XAxiDma_Bd *BdPtr;

static int8_t ADC_OriginalData[8192] __attribute__((aligned(8)));

static int16_t trigger_level = 0;                             //!<@brief 触发电平，单位mV
static int16_t trigger_hysteresis = 200;                      //!<@brief 触发滞回，单位mV
static trigger_condition_e trigger_condition = RISING_EDGE_TRIGGER;   //!<@brief 触发方向
static int16_t trigger_position = 2048;                       //!<@brief 触发时间，单位采样

static int16_t trigger_num;                      //!<@brief 触发点数量
static int16_t trigger_locate[TRIGGER_NUM_MAX];  //!<@brief 触发点位置

int16_t ADC_Data[4096];

static void ADC_calibration();

int ADC_init_dma_channel(XAxiDma *interface) {
    ADC_Mutex = xSemaphoreCreateMutex();
    XAxiDma_SelectCyclicMode(interface, XAXIDMA_DEVICE_TO_DMA, TRUE);
    RingPtr = XAxiDma_GetRxRing(interface);
    XAxiDma_BdRingEnableCyclicDMA(RingPtr);

    CHECK_STATUS_RET(XAxiDma_BdRingAlloc(RingPtr, 1, &BdPtr));
    CHECK_STATUS_RET(XAxiDma_BdSetBufAddr(BdPtr, (UINTPTR) ADC_OriginalData));
    CHECK_STATUS_RET(XAxiDma_BdSetLength(BdPtr, sizeof(ADC_OriginalData), RingPtr->MaxTransferLen));
    XAxiDma_BdSetCtrl(BdPtr, XAXIDMA_BD_CTRL_ALL_MASK);
    XAxiDma_BdWrite(BdPtr, XAXIDMA_BD_NDESC_OFFSET, BdPtr);
    XAxiDma_BdSetId(BdPtr, (UINTPTR) ADC_OriginalData);

    /* 将描述符链表起始地址下载至DMA寄存器中 */
    CHECK_STATUS_RET(XAxiDma_BdRingToHw(RingPtr, 1, BdPtr));

    /* 启动DMA接收 */
    CHECK_STATUS_RET(XAxiDma_BdRingStart(RingPtr));

    /* ADC校准偏移 */
    ADC_calibration();
    return XST_SUCCESS;
}

static bool ADC_data_copy(int16_t trigger_pos) {
    if (trigger_pos >= trigger_position && trigger_pos < 4096 + trigger_position) {
        for (int i = 0; i < 4096; i++) {
            ADC_Data[i] = ADC_RawToVoltage_mV(ADC_OriginalData[i - trigger_position + trigger_pos]);
        }
        return true;
    } else return false;
}

static void ADC_process_data(bool *triggered) {
    bool t = false;
    int trigger_status = 0;
    int trigger_pos1 = 0;
    int16_t trigger_upper = trigger_level + trigger_hysteresis / 2;
    int16_t trigger_lower = trigger_level - trigger_hysteresis / 2;
    for (int i = 0; i < 8192; i++) {
        int16_t voltage = ADC_RawToVoltage_mV(ADC_OriginalData[i]);
        if (trigger_condition == RISING_EDGE_TRIGGER || trigger_condition == FALLING_EDGE_TRIGGER) {
            if (trigger_condition == FALLING_EDGE_TRIGGER)
                voltage = -voltage;
            switch (trigger_status) {
                case 0:
                    if (voltage < trigger_lower)
                        trigger_status = 1;
                    break;
                case 1:
                    if (voltage > trigger_upper) {
                        if (!t) t = ADC_data_copy(i);
                        if (trigger_num < TRIGGER_NUM_MAX)
                            trigger_locate[trigger_num++] = i;
                        trigger_status = 0;
                    }
                    if (voltage > trigger_lower && voltage < trigger_upper) {
                        trigger_status = 2;
                        trigger_pos1 = i;
                    }
                    break;
                case 2:
                    if (voltage < trigger_lower)
                        trigger_status = 1;
                    else if (voltage > trigger_upper) {
                        if (!t) t = ADC_data_copy((trigger_pos1 + i) / 2);
                        if (trigger_num < TRIGGER_NUM_MAX)
                            trigger_locate[trigger_num++] = (trigger_pos1 + i) / 2;
                        trigger_status = 0;
                    }
                    break;
            }
        }
    }
    if (!t) ADC_data_copy(trigger_position);
    if (triggered) *triggered = t;
}

int ADC_get_data_now(bool *triggered, TickType_t timeout) {
    int status = XST_SUCCESS;
    TickType_t tick = xTaskGetTickCount();
    trigger_num = 0;

    /* 向ADC Packager发送启动信号 */
    SPU_SendPackPulse(ADC_PackPulse);
    do {
        vTaskDelay(1);
        vPortEnterCritical();
        XAXIDMA_CACHE_INVALIDATE(BdPtr);
        vPortExitCritical();
        uint32_t receive_len = XAxiDma_BdGetActualLength(BdPtr, 0xffffff);
        if (receive_len == sizeof(ADC_OriginalData)) {
            os_DCacheInvalidateRange(ADC_OriginalData, sizeof(ADC_OriginalData));
            ADC_process_data(triggered);
            return XST_SUCCESS;
        }
    } while (xTaskGetTickCount() < tick + timeout);
    return XST_FAILURE;
}

int ADC_get_data(bool *triggered) {
    int status = XST_SUCCESS;
    trigger_num = 0;
    vPortEnterCritical();
    XAXIDMA_CACHE_INVALIDATE(BdPtr);
    vPortExitCritical();
    uint32_t receive_len = XAxiDma_BdGetActualLength(BdPtr, 0xffffff);
    if (receive_len == sizeof(ADC_OriginalData)) {
        os_DCacheInvalidateRange(ADC_OriginalData, sizeof(ADC_OriginalData));
        ADC_process_data(triggered);
    } else {
        xil_printf("warning: ADC data length is incorrect\r\n");
        status = XST_DATA_LOST;
    }
    /* 向ADC Packager发送启动信号 */
    SPU_SendPackPulse(ADC_PackPulse);

    return status;
}

float ADC_get_period() {
    if (trigger_num >= 2) {
        float diff_time_sum = 0;
        for (int i = 0; i < trigger_num - 1; i++) {
            diff_time_sum += (trigger_locate[i + 1] - trigger_locate[i]) / 30e6;
        }
        return diff_time_sum / (trigger_num - 1);
    } else return NAN;
}

int ADC_get_max_min(float *max_p, float *min_p) {
    if (max_p == NULL || min_p == NULL)
        return XST_INVALID_PARAM;
    float max = -INFINITY, min = INFINITY;
    for (int i = 0; i < 4096; i++) {
        if (ADC_Data[i] > max) max = ADC_Data[i];
        if (ADC_Data[i] < min) min = ADC_Data[i];
    }
    *max_p = max;
    *min_p = min;
    return XST_SUCCESS;
}

float ADC_get_mean() {
    double sum = 0;
    for (int i = 0; i < 4096; i++)
        sum += ADC_Data[i];
    return sum / 4096;
}

float ADC_get_mean_cycle() {
    int64_t sum = 0;
    if (trigger_num >= 2) {
        for (int i = trigger_locate[0]; i < trigger_locate[trigger_num - 1]; i++)
            sum += ADC_OriginalData[i];
        return ADC_RawToVoltage_mV((float) sum) / (trigger_locate[trigger_num - 1] - trigger_locate[0]);
    } else return NAN;
}

float ADC_get_rms() {
    double sum = 0;
    for (int i = 0; i < 4096; i++)
        sum += pow(ADC_Data[i], 2) / 4096;
    return sqrtl(sum);
}

float ADC_get_rms_cycle() {
    double sum = 0;
    if (trigger_num >= 2) {
        for (int i = trigger_locate[0]; i < trigger_locate[trigger_num - 1]; i++)
            sum += pow(ADC_OriginalData[i], 2) / (trigger_locate[trigger_num - 1] - trigger_locate[0]);
        return ADC_RawToVoltage_mV(sqrtl(sum));
    } else return NAN;
}

static void ADC_calibration() {
    SPU_SetAdcOffset(0);
    SPU_SendPackPulse(ADC_PackPulse);
    vTaskDelay(1);
    ADC_get_data(NULL);
    uint64_t sum = 0;
    for (int i = 0; i < 8192; i++) {
        sum += ADC_OriginalData[i];
    }
    int8_t offset = sum / 8192;
    if (abs(offset - 128) < 10)
        SPU_SetAdcOffset(sum / 8192);
    else
        SPU_SetAdcOffset(127);
}

void ADC_set_trigger_level(int16_t level) {
    trigger_level = level;
}

void ADC_set_trigger_hysteresis(int16_t hysteresis) {
    trigger_hysteresis = hysteresis;
}

void ADC_set_trigger_condition(trigger_condition_e condition) {
    trigger_condition = condition;
}

void ADC_set_trigger_position(int16_t position) {
    trigger_position = position;
}

int16_t ADC_get_trigger_level() {
    return trigger_level;
}

int16_t ADC_get_trigger_hysteresis() {
    return trigger_hysteresis;
}

trigger_condition_e ADC_get_trigger_condition() {
    return trigger_condition;
}

int16_t ADC_get_trigger_position() {
    return trigger_position;
}

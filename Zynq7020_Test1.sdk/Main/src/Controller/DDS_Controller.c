//
// Created by yaoji on 2022/1/29.
//

#include "DDS_Controller.h"
#include "FreeRTOS_Mem/FreeRTOS_Mem.h"
#include "math.h"
#include "utils.h"

#define USE_DDS_RAM 1
#define DDS_RAM_LEN 0x400000
#define DDS_RAM_ATTRIBUTE __attribute__((section(".DDS_RAM")))

static inline double inRange(double min, double v, double max) {
    return v >= max ? max :
           v <= min ? min : v;
}


#if USE_DDS_RAM
static int8_t DDS_BUFFER[DDS_RAM_LEN] DDS_RAM_ATTRIBUTE;
#else
static int8_t *DDS_BUFFER;
static int8_t *NEW_BUFFER;
#endif

static int64_t lcm(int64_t a, int64_t b) {
    while (b != 0) {
        int64_t c = a % b;
        a = b;
        b = c;
    }
    return a;
}

/**
 * 根据信号频率自动分配合适长度的缓冲区
 * @param freq 生成波形的频率
 * @param len_ptr 返回缓冲区长度
 * @return 分配的内存地址
 */
static int8_t *DDS_buff_malloc(uint32_t freq, int *len_ptr) {
    int len = DAC_CLK_FREQ / lcm(freq, DAC_CLK_FREQ);
#if USE_DDS_RAM
    if (len > DDS_RAM_LEN)
        len = DAC_CLK_FREQ / freq;
#else
    if (len > 65536)
        len = DAC_CLK_FREQ / freq;
#endif
    if (len < 512)
        len = len * (512 / len + 1);

    if (len_ptr) *len_ptr = len;
#if USE_DDS_RAM
    return DDS_BUFFER;
#else
    int8_t *align_addr = NULL;
    NEW_BUFFER = os_malloc(len + 16);
    CHECK_FATAL_ERROR(NEW_BUFFER == NULL)
    if (((uint32_t) NEW_BUFFER) % 8 == 0) {
        align_addr = NEW_BUFFER;
    } else {
        align_addr = (int8_t *) (((uint32_t) NEW_BUFFER & ~0x07) + 0x08);
    }

    return align_addr;
#endif
}

/**
 * 替换缓冲区并启动启动DMA
 * @param buf_addr
 * @param len
 * @return
 */
static int DDS_buff_replace(int8_t *buf_addr, int len) {
    int res = XST_SUCCESS;
    Xil_DCacheFlushRange(buf_addr, len);
    res = DAC_start(buf_addr, len);
#if USE_DDS_RAM
#else
    if (res != XST_SUCCESS) {
        os_free(NEW_BUFFER);
        xil_printf("ERROR: File:'%s' Line:%d return %d\r\n", __FILE__, __LINE__, res);
    } else {
        os_free(DDS_BUFFER);
        DDS_BUFFER = NEW_BUFFER;
    }
#endif
    return res;
}

static int DDS_general_generator(DDS_sine_t *param, double (*core)(double, void *)) {
    int len = 0;
    int8_t *align_addr = DDS_buff_malloc(param->base.freq, &len);

    double p = param->base.phase * M_PI * 2 / 3600;
    double w = M_PI * 2 * param->base.freq;
    for (int i = 0; i < len; i++) {
        double v = (param->base.amplitude / 2 * core(fmod(w * i / DAC_CLK_FREQ + p, M_PI * 2), param) +
                    param->base.offset);
        align_addr[i] = inRange(-127, v * 256 / 1000 / 10, 127);
    }

    CHECK_STATUS_RET(DDS_buff_replace(align_addr, len));
    return XST_SUCCESS;
}

static double DDS_sin_core(double x, void *param) {
    (void) param;
    return sin(x);
}

static double DDS_triangle_core(double x, void *param) {
    (void) param;
    return x < M_PI ? (x / M_PI * 2 - 1) : (x / (-M_PI) * 2 + 3);
}

static double DDS_rising_ramp_core(double x, void *param) {
    (void) param;
    return x / M_PI - 1;
}

static double DDS_falling_ramp_core(double x, void *param) {
    (void) param;
    return -(x / M_PI - 1);
}

static double DDS_square_core(double x, void *param) {
    return x < (((DDS_square_t *) param)->duty_cycle / 1000.0 * 2 * M_PI) ? 1 : -1;
}

static double DDS_stair_step_core(double x, void *param) {
    DDS_stair_step_t *p = param;
    double n = floor(x / (M_PI * 2) * (p->falling + p->rising));
    return ((n <= p->rising) ? (n * 2 / p->rising) : (2 - (n - p->rising) * 2 / p->falling)) - 1;
}

int DDS_wav_generator(void *param) {
    if (!param) return XST_FAILURE;
    switch (DDS_get_type(param)) {
        case TYPE_SINE:
            return DDS_general_generator(param, DDS_sin_core);
        case TYPE_SQUARE:
            return DDS_general_generator(param, DDS_square_core);
        case TYPE_TRIANGLE:
            return DDS_general_generator(param, DDS_triangle_core);
        case TYPE_RISING_RAMP:
            return DDS_general_generator(param, DDS_rising_ramp_core);
        case TYPE_FALLING_RAMP:
            return DDS_general_generator(param, DDS_falling_ramp_core);
        case TYPE_STAIR_STEP:
            return DDS_general_generator(param, DDS_stair_step_core);
        default:
            return XST_INVALID_PARAM;
    }
}

uint32_t DDS_get_type(void *param) {
    if (param) return *(uint32_t *) param;
    else return 0xffffffff;
}

int DDS_wav_from_data(int8_t *data, int len) {
    int copy = 1;
    if (len < 512) copy = 512 / len + 1;
#if USE_DDS_RAM
    if (len * copy >= DDS_RAM_LEN) return XST_FAILURE;
    for(int i = 0; i < copy; i++)
        memcpy(DDS_BUFFER + i * len, data, len);
    return DDS_buff_replace(DDS_BUFFER, len * copy);
#else
    int8_t *align_addr = NULL;
    NEW_BUFFER = os_malloc(len * copy + 16);
    CHECK_FATAL_ERROR(NEW_BUFFER == NULL)
    if (((uint32_t) NEW_BUFFER) % 8 == 0) {
        align_addr = NEW_BUFFER;
    } else {
        align_addr = (int8_t *) (((uint32_t) NEW_BUFFER & ~0x07) + 0x08);
    }
    for(int i = 0; i < copy; i++)
        memcpy(align_addr + i * len, data, len);
    return DDS_buff_replace(align_addr, len * copy);
#endif
}

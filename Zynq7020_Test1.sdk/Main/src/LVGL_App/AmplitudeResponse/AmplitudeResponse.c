#include "AmplitudeResponse.h"
#include <xstatus.h>
#include <arm_math.h>

int AmplitudeResponse(const int16_t *coe_input, float *amp_output, int len) {
    static float fft_input[128], fft_output[128];

    if (coe_input == NULL || amp_output == NULL || len <= 0 || len > 128)
        return XST_INVALID_PARAM;

    arm_rfft_fast_instance_f32 rfftFastInstanceF32;
    arm_status status = arm_rfft_fast_init_f32(&rfftFastInstanceF32, 128);
    if (status != ARM_MATH_SUCCESS)
        return XST_FAILURE;

    arm_fill_f32(0, fft_input, 128);
    // 系数归一化
    for (int i = 0; i < len; i++)
        fft_input[i] = (float) coe_input[i] / (float) INT16_MAX;
    arm_rfft_fast_f32(&rfftFastInstanceF32, fft_input, fft_output, 0);
    arm_cmplx_mag_f32(fft_output, amp_output, 64);
    for (int i = 0; i < 64; i++)
        amp_output[i] = 20 * log10f(amp_output[i]);
    return XST_SUCCESS;
}

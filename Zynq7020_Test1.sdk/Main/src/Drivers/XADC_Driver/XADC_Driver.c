#include "XADC_Driver.h"
#include "utils.h"

#define GET_DATA(XADC, SENSOR, DATA) {                                  \
    uint16_t RawData;                                                   \
    RawData = XAdcPs_GetAdcData((XADC), XADCPS_CH_##SENSOR);            \
    (DATA)->SENSOR##_Current = XAdcPs_RawToVoltage(RawData);            \
    RawData = XAdcPs_GetMinMaxMeasurement((XADC), XADCPS_MAX_##SENSOR); \
    (DATA)->SENSOR##_Max = XAdcPs_RawToVoltage(RawData);                \
    RawData = XAdcPs_GetMinMaxMeasurement((XADC), XADCPS_MIN_##SENSOR); \
    (DATA)->SENSOR##_Min = XAdcPs_RawToVoltage(RawData);                \
}

int XADC_Init(XAdcPs *XAdcInstPtr, uint16_t DeviceId) {
    XAdcPs_Config *config = XAdcPs_LookupConfig(DeviceId);
    CHECK_STATUS_RET(XAdcPs_CfgInitialize(XAdcInstPtr, config, config->BaseAddress));
    CHECK_STATUS_RET(XAdcPs_SelfTest(XAdcInstPtr));

    /*
	 * Disable the Channel Sequencer before configuring the Sequence
	 * registers.
	 */
    XAdcPs_SetSequencerMode(XAdcInstPtr, XADCPS_SEQ_MODE_SAFE);
    return XST_SUCCESS;
}

void XADC_GetAll(XAdcPs *XAdcInstPtr, XADC_SensorsData *data) {

    /**
	 * Read the on-chip Temperature Data (Current/Maximum/Minimum)
	 * from the ADC data registers.
	 */
    uint16_t RawData;
    RawData = XAdcPs_GetAdcData(XAdcInstPtr, XADCPS_CH_TEMP);
    data->Temperature_Current = XAdcPs_RawToTemperature(RawData);

    RawData = XAdcPs_GetMinMaxMeasurement(XAdcInstPtr, XADCPS_MAX_TEMP);
    data->Temperature_Max = XAdcPs_RawToTemperature(RawData);

    RawData = XAdcPs_GetMinMaxMeasurement(XAdcInstPtr, XADCPS_MIN_TEMP);
    data->Temperature_Min = XAdcPs_RawToTemperature(RawData);

    GET_DATA(XAdcInstPtr, VCCINT, data)
    GET_DATA(XAdcInstPtr, VCCAUX, data)
    GET_DATA(XAdcInstPtr, VBRAM, data)
    GET_DATA(XAdcInstPtr, VCCPINT, data)
    GET_DATA(XAdcInstPtr, VCCPAUX, data)
    GET_DATA(XAdcInstPtr, VCCPDRO, data)
}

/**
 *
 * This function converts the fraction part of the given floating point number
 * (after the decimal point)to an integer.
 *
 * @param	FloatNum is the floating point number.
 *
 * @return	Integer number to a precision of 3 digits.
 *
 * @note
 * This function is used in the printing of floating point data to a STDIO device
 * using the xil_printf function. The xil_printf is a very small foot-print
 * printf function and does not support the printing of floating point numbers.
 *
 */
int XADC_FractionToInt(float FloatNum) {
    float Temp;

    Temp = FloatNum;
    if (FloatNum < 0) {
        Temp = -(FloatNum);
    }

    return (((int) ((Temp - (float) ((int) Temp)) * (1000.0f))));
}
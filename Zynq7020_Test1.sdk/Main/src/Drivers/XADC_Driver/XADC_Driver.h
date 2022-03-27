#ifndef SRC_DRIVERS_XADC_DRIVER_XADC_DRIVER_H_
#define SRC_DRIVERS_XADC_DRIVER_XADC_DRIVER_H_
#include "xadcps.h"

#define STRUCT_FIELD(NAME) \
    float NAME##_Current;  \
    float NAME##_Max;      \
    float NAME##_Min;

typedef struct {
    STRUCT_FIELD(Temperature)
    STRUCT_FIELD(VCCINT)
    STRUCT_FIELD(VCCAUX)
    STRUCT_FIELD(VBRAM)
    STRUCT_FIELD(VCCPINT)
    STRUCT_FIELD(VCCPAUX)
    STRUCT_FIELD(VCCPDRO)
} XADC_SensorsData;

int XADC_Init(XAdcPs *XAdcInstPtr, uint16_t DeviceId);
void XADC_GetAll(XAdcPs *XAdcInstPtr, XADC_SensorsData *data);
int XADC_FractionToInt(float FloatNum);

#define PRINT_FLOAT(VAL) ((int)(VAL)), XADC_FractionToInt(VAL)

#endif /* SRC_DRIVERS_XADC_DRIVER_XADC_DRIVER_H_ */

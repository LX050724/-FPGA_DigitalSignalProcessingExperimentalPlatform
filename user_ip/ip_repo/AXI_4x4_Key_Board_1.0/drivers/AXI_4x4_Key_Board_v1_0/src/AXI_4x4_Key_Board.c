

/***************************** Include Files *******************************/
#include "AXI_4x4_Key_Board.h"

void AXI_4X4_KEY_BOARD_Reg_SetDiv(void * baseaddr_p, uint32_t div) {
    if (baseaddr_p == NULL) return;
    AXI_4X4_KEY_BOARD_mWriteReg(baseaddr_p, AXI_4X4_KEY_BOARD_S00_AXI_SLV_REG0_OFFSET, div);
}

AXI_4X4_KEY_BOARD_KEY AXI_4X4_KEY_BOARD_Reg_GetKey(void * baseaddr_p) {
	AXI_4X4_KEY_BOARD_KEY key = { 0 };
    if (baseaddr_p == NULL) return key;
    uint16_t val = AXI_4X4_KEY_BOARD_mReadReg(baseaddr_p, AXI_4X4_KEY_BOARD_S00_AXI_SLV_REG1_OFFSET) & 0xffff;
    key = *(AXI_4X4_KEY_BOARD_KEY *)&val;
    return key;
}


/************************** Function Definitions ***************************/

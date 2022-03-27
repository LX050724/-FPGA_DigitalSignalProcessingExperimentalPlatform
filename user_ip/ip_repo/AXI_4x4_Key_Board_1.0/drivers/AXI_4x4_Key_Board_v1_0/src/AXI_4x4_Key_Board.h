
#ifndef AXI_4X4_KEY_BOARD_H
#define AXI_4X4_KEY_BOARD_H


/****************** Include Files ********************/
#include "xil_types.h"
#include "xstatus.h"
#include "xil_io.h"

#define AXI_4X4_KEY_BOARD_S00_AXI_SLV_REG0_OFFSET 0
#define AXI_4X4_KEY_BOARD_S00_AXI_SLV_REG1_OFFSET 4
#define AXI_4X4_KEY_BOARD_S00_AXI_SLV_REG2_OFFSET 8
#define AXI_4X4_KEY_BOARD_S00_AXI_SLV_REG3_OFFSET 12

typedef struct {
    uint16_t Key_1 : 1;
    uint16_t Key_2 : 1;
    uint16_t Key_3 : 1;
    uint16_t Key_A : 1;
    uint16_t Key_4 : 1;
    uint16_t Key_5 : 1;
    uint16_t Key_6 : 1;
    uint16_t Key_B : 1;
    uint16_t Key_7 : 1;
    uint16_t Key_8 : 1;
    uint16_t Key_9 : 1;
    uint16_t Key_C : 1;
    uint16_t Key_AST : 1;   //*
    uint16_t Key_0 : 1;
    uint16_t Key_POU : 1;   //#
    uint16_t Key_D : 1;
} AXI_4X4_KEY_BOARD_KEY;


/**************************** Type Definitions *****************************/
/**
 *
 * Write a value to a AXI_4X4_KEY_BOARD register. A 32 bit write is performed.
 * If the component is implemented in a smaller width, only the least
 * significant data is written.
 *
 * @param   BaseAddress is the base address of the AXI_4X4_KEY_BOARDdevice.
 * @param   RegOffset is the register offset from the base to write to.
 * @param   Data is the data written to the register.
 *
 * @return  None.
 *
 * @note
 * C-style signature:
 * 	void AXI_4X4_KEY_BOARD_mWriteReg(u32 BaseAddress, unsigned RegOffset, u32 Data)
 *
 */
#define AXI_4X4_KEY_BOARD_mWriteReg(BaseAddress, RegOffset, Data) \
  	Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))

/**
 *
 * Read a value from a AXI_4X4_KEY_BOARD register. A 32 bit read is performed.
 * If the component is implemented in a smaller width, only the least
 * significant data is read from the register. The most significant data
 * will be read as 0.
 *
 * @param   BaseAddress is the base address of the AXI_4X4_KEY_BOARD device.
 * @param   RegOffset is the register offset from the base to write to.
 *
 * @return  Data is the data from the register.
 *
 * @note
 * C-style signature:
 * 	u32 AXI_4X4_KEY_BOARD_mReadReg(u32 BaseAddress, unsigned RegOffset)
 *
 */
#define AXI_4X4_KEY_BOARD_mReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))

/************************** Function Prototypes ****************************/
/**
 *
 * Run a self-test on the driver/device. Note this may be a destructive test if
 * resets of the device are performed.
 *
 * If the hardware system is not built correctly, this function may never
 * return to the caller.
 *
 * @param   baseaddr_p is the base address of the AXI_4X4_KEY_BOARD instance to be worked on.
 *
 * @return
 *
 *    - XST_SUCCESS   if all self-test code passed
 *    - XST_FAILURE   if any self-test code failed
 *
 * @note    Caching must be turned off for this function to work.
 * @note    Self test may fail if data memory and device are not on the same bus.
 *
 */
XStatus AXI_4X4_KEY_BOARD_Reg_SelfTest(void * baseaddr_p);

void AXI_4X4_KEY_BOARD_Reg_SetDiv(void * baseaddr_p, uint32_t div);

AXI_4X4_KEY_BOARD_KEY AXI_4X4_KEY_BOARD_Reg_GetKey(void * baseaddr_p);

#endif // AXI_4X4_KEY_BOARD_H

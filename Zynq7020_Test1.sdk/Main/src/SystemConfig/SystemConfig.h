#ifndef SYSTEM_CONFIG_H_
#define SYSTEM_CONFIG_H_

#include "xparameters_ps.h"

/*
 * SLCR Registers
 */
#define PS_RST_CTRL_REG			(XPS_SYS_CTRL_BASEADDR + 0x200)
#define FPGA_RESET_REG			(XPS_SYS_CTRL_BASEADDR + 0x240)
#define RESET_REASON_REG		(XPS_SYS_CTRL_BASEADDR + 0x250)
#define RESET_REASON_CLR		(XPS_SYS_CTRL_BASEADDR + 0x254)
#define REBOOT_STATUS_REG		(XPS_SYS_CTRL_BASEADDR + 0x258)
#define BOOT_MODE_REG			(XPS_SYS_CTRL_BASEADDR + 0x25C)
#define PS_LVL_SHFTR_EN			(XPS_SYS_CTRL_BASEADDR + 0x900)

/*
 * PS reset control register define
 */
#define PS_RST_MASK			0x1	/**< PS software reset */

/*
 * SLCR BOOT Mode Register defines
 */
#define BOOT_MODES_MASK			0x00000007 /**< FLASH types */

/*
 * Boot Modes
 */

typedef enum {
    JTAG_MODE		=	0x00000000,
    QSPI_MODE		=	0x00000001,
    NOR_FLASH_MODE	=	0x00000002,
    NAND_FLASH_MODE	=	0x00000004,
    SD_MODE			=	0x00000005,
    MMC_MODE		=	0x00000006,
} BootMod_t;

BootMod_t GetBootMod();

int ProgramFLASH(const char *bootFile);
int ProgramFLASH_isfinished();
const char *getFirmwareVersion();

#endif

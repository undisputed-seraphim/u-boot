/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Askey AP5100W  board configuration
 */

#ifndef __CONFIG_AP5100W_H
#define __CONFIG_AP5100W_H

/* DRAM base */
#define CFG_SYS_SDRAM_BASE		0x80000000
#define CFG_SYS_INIT_SP_OFFSET		0x800000

/* NS16550 serial */
#define CFG_SYS_NS16550_CLK		50000000
#define CFG_SYS_NS16550_COM1		0xb8002000

#define CFG_SYS_BAUDRATE_TABLE		{ 9600, 19200, 38400, 57600, 115200 }

/* Dummy — will be set by SPL NAND loading later */
#define CFG_SYS_UBOOT_BASE		0

/* NAND boot — U-Boot proper at offset 0x20000 */
#define CFG_SYS_NAND_U_BOOT_OFFS	0x20000
#define CFG_SYS_NAND_BASE		0xB801A000

#endif /* __CONFIG_AP5100W_H */

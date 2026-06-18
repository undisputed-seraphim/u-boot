/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Realtek RTL8198C SoC register definitions
 */

#ifndef _RTL8198C_H_
#define _RTL8198C_H_

/* System controller */
#define SYSCTL_BASE		0x18000000
#define SYSCTL_SIZE		0x1000

#define SYSCTL_CHIP_INFO	0x0000
#define SYSCTL_CLK_MAG		0x0010
#define SYSCTL_CHIP_REVID	0x000c

/* Reset */
#define SYSCTL_RSTCTL		0x0004

/* Memory controller */
#define MEMC_BASE		0x18001000
#define MEMC_SIZE		0x100

/* UART0 */
#define UART0_BASE		0x18002000
#define UART0_SIZE		0x100

/* SPI NOR controller */
#define NOR_BASE		0x18001200

/* NAND flash controller */
#define NAND_BASE		0x1801A000
#define NAND_SIZE		0x1000

/* GPIO */
#define GPIO_BASE		0x18003500

/* Timer block (watchdog within) */
#define TC_BASE			0x18003100
#define WDTCNR_OFFSET		0x1C

/* GIC (MIPS Global Interrupt Controller) */
#define GIC_BASE		0x1BDC0000
#define CM_GCR_BASE		0x1FBF8000

/* CPC (Cluster Power Controller) */
#define CPC_BASE		0x1BDE0000

#endif /* _RTL8198C_H_ */

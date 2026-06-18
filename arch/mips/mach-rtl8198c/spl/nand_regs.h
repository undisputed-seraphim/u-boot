/* SPDX-License-Identifier: GPL-2.0 */
/*
 * RTL8198C NAND flash controller registers
 *
 * Ported from the proven OpenWrt driver
 * (target/linux/realtek/files-6.18/drivers/mtd/nand/raw/rtl8198c_nand.h)
 */

#ifndef _RTL8198C_NAND_REGS_H_
#define _RTL8198C_NAND_REGS_H_

/* Controller base (KSEG1, uncached) */
#define NAND_BASE		0xB801A000

/* Register offsets */
#define NACFR			0x00
#define NACR			0x04
#define NACMR			0x08
#define NAADR			0x0C
#define NADCRR			0x10
#define NADR			0x14
#define NADFSAR			0x18
#define NADRSAR			0x20	/* RTL8198C-specific offset */
#define NADTSAR			0x24	/* RTL8198C-specific offset */
#define NASR			0x28	/* RTL8198C-specific offset */

/* NACR bits */
#define NACR_READY		(1 << 31)
#define NACR_ECC_EN		(1 << 30)
#define NACR_TIMING		0x000FFFFF

/* NACMR bits */
#define NACMR_CECS0		(1 << 30)

/* NAADR bits */
#define NAADR_AD2EN		(1 << 26)
#define NAADR_AD1EN		(1 << 25)
#define NAADR_AD0EN		(1 << 24)

/* NADCRR bits */
#define NADCRR_DESC0		(1 << 4)
#define NADCRR_DMARE		(1 << 3)
#define NADCRR_LBC64		2
#define NADCRR_DMA_READ		(NADCRR_DESC0 | NADCRR_DMARE | NADCRR_LBC64)

/* NASR bits */
#define NASR_NDRS		(1 << 1)
#define NASR_NRER		(1 << 3)
#define NASR_NECN		(1 << 4)
#define NASR_ECC_MASK		0xF0
#define NASR_ECC_SHIFT		4
#define NASR_CLEAR		0x0F

/* Geometry for W29N01HV (128 MiB, 2K pages, 64B OOB, 128K blocks) */
#define NAND_PAGE_SIZE		2048
#define NAND_OOB_SIZE		64
#define NAND_BLOCK_SIZE		(128 * 1024)
#define NAND_PAGES_PER_BLOCK	(NAND_BLOCK_SIZE / NAND_PAGE_SIZE)
#define NAND_SECTOR_SIZE	512
#define NAND_SECTORS_PER_PAGE	(NAND_PAGE_SIZE / NAND_SECTOR_SIZE)
#define NAND_SECTOR_TOTAL	(NAND_SECTOR_SIZE + 16)	/* data + OOB per ECC sector */
#define NAND_CHIP_SIZE		(128 * 1024 * 1024)

/* NAND commands */
#define NAND_CMD_READ0		0x00
#define NAND_CMD_READSTART	0x30
#define NAND_CMD_RESET		0xFF
#define NAND_CMD_STATUS		0x70
#define NAND_CMD_READID		0x90

/* Bad block marker: byte 0 of first page OOB per block */
#define BBM_BYTE		0

#endif /* _RTL8198C_NAND_REGS_H_ */

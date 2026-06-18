// SPDX-License-Identifier: GPL-2.0
/*
 * RTL8198C SPL NAND support — minimal DMA page read
 *
 * Ported directly from the proven OpenWrt driver
 * (target/linux/realtek/files-6.18/drivers/mtd/nand/raw/rtl8198c_nand.c)
 *
 * DMA read sequence is register-identical to the kernel driver.
 * Only reads are supported — no write, erase, or ONFI detection.
 * Chip geometry is hardcoded for W29N01HV (128 MiB, 2K pages).
 */

#include <init.h>
#include <stdio.h>
#include <debug_uart.h>
#include <asm/io.h>
#include "nand_regs.h"

/* DMA bounce buffers — allocated in DRAM after DDR3 init */
static u8 dma_buf[NAND_SECTOR_TOTAL] __aligned(64);
static u8 oob_buf[16] __aligned(4);

/* Convert KSEG1 address to physical (mask top 3 bits) */
static u32 kseg1_to_phys(void *addr)
{
	return (u32)addr & 0x1FFFFFFF;
}

/* Wait for NACR_READY */
static void nfc_wait_ready(void)
{
	while (!(readl((void *)(NAND_BASE + NACR)) & NACR_READY))
		;
}

/* Send a NAND command via NACMR */
static void nfc_cmd(u8 cmd)
{
	writel(NACMR_CECS0 | cmd, (void *)(NAND_BASE + NACMR));
	nfc_wait_ready();
}

/* Read chip ID via PIO */
static void nfc_read_id(u8 *id, int len)
{
	u32 word;
	int i;

	nfc_cmd(NAND_CMD_READID);
	writel(NAADR_AD0EN | 0x00, (void *)(NAND_BASE + NAADR));
	nfc_wait_ready();

	for (i = 0; i < len; i++) {
		if ((i & 3) == 0)
			word = readl((void *)(NAND_BASE + NADR));
		id[i] = (word >> ((i & 3) * 8)) & 0xFF;
	}
}

/* Reset NAND chip */
static void nfc_reset(void)
{
	nfc_cmd(NAND_CMD_RESET);
	writel(0, (void *)(NAND_BASE + NACMR));
	nfc_wait_ready();
}

/* DMA read one ECC sector (512 data + 16 OOB) */
static int nand_dma_read_sector(u32 flash_addr)
{
	u32 data_phys = kseg1_to_phys(dma_buf);
	u32 tag_phys  = data_phys + NAND_SECTOR_SIZE;

	writel(NACR_READY | NACR_ECC_EN | NACR_TIMING,
	       (void *)(NAND_BASE + NACR));
	writel(NASR_CLEAR, (void *)(NAND_BASE + NASR));

	writel(data_phys, (void *)(NAND_BASE + NADRSAR));
	writel(tag_phys,  (void *)(NAND_BASE + NADTSAR));
	writel(flash_addr, (void *)(NAND_BASE + NADFSAR));

	writel(NADCRR_DMA_READ, (void *)(NAND_BASE + NADCRR));
	nfc_wait_ready();

	return 0;
}

/* Check ECC status after DMA read */
static int nand_check_sector(void)
{
	u32 status = readl((void *)(NAND_BASE + NASR));
	int ecnt;

	if (!(status & NASR_NDRS))
		return 0;

	if (status & NASR_NRER) {
		ecnt = (status & NASR_ECC_MASK) >> NASR_ECC_SHIFT;
		if (ecnt > 0 && ecnt <= 4) {
			writel(status & ~NASR_NECN,
			       (void *)(NAND_BASE + NASR));
			return 1;
		}
		writel(status & ~NASR_NECN,
		       (void *)(NAND_BASE + NASR));
		return -1;
	}

	writel(status & ~NASR_ECC_MASK, (void *)(NAND_BASE + NASR));
	return 0;
}

/* Read one full NAND page into dst buffer */
static int nand_read_page(int page, void *dst)
{
	u32 flash_addr = page << 12;  /* 2K-page: page << (page_shift + 1) */
	int sector;

	for (sector = 0; sector < NAND_SECTORS_PER_PAGE; sector++) {
		nand_dma_read_sector(flash_addr);
		nand_check_sector();

		memcpy(dst + sector * NAND_SECTOR_SIZE,
		       dma_buf, NAND_SECTOR_SIZE);

		flash_addr += NAND_SECTOR_TOTAL;
	}

	return 0;
}

/* Check if a block is bad (BBM byte in first page OOB) */
static int nand_block_isbad(int block)
{
	int page = block * NAND_PAGES_PER_BLOCK;
	u32 flash_addr = page << 12;

	nand_dma_read_sector(flash_addr);
	nand_check_sector();

	return dma_buf[BBM_BYTE] != 0xFF;
}

/* ================================================================
 * SPL NAND interface — called by the U-Boot SPL framework
 * ================================================================ */

static int page_size;
static int chip_size;

void nand_init(void)
{
	u8 id[5];

	printascii("SPL: initializing NAND...\n");

	nfc_reset();
	printascii("SPL: NAND reset OK\n");

	nfc_read_id(id, 5);
	printascii("SPL: NAND ID: ");
	/* Print hex bytes inline */
	{
		int i;
		char hex[] = "00 ";
		for (i = 0; i < 5; i++) {
			hex[0] = "0123456789abcdef"[id[i] >> 4];
			hex[1] = "0123456789abcdef"[id[i] & 0xf];
			printascii(hex);
		}
		printascii("\n");
	}

	/* Verify it's a W29N01HV: manufacturer=0xEF (Winbond), dev=0xF1 */
	if (id[0] == 0xEF && id[1] == 0xF1)
		printascii("SPL: NAND: W29N01HV 128 MiB\n");
	else
		printascii("SPL: NAND: unknown chip (proceeding anyway)\n");

	page_size = NAND_PAGE_SIZE;
	chip_size = NAND_CHIP_SIZE;
}

int nand_spl_load_image(uint32_t offs, unsigned int size, void *dst)
{
	int page, start_page, end_page;
	int total_pages;
	int block, bad_blocks = 0;

	start_page = offs / page_size;
	end_page   = (offs + size + page_size - 1) / page_size;
	total_pages = end_page - start_page;

	debug("SPL: loading %u bytes from NAND offset 0x%x (%d pages)\n",
	      size, offs, total_pages);

	for (page = start_page; page < end_page; page++) {
		block = page / NAND_PAGES_PER_BLOCK;

		/* Skip bad blocks */
		if (nand_block_isbad(block)) {
			bad_blocks++;
			page = (block + 1) * NAND_PAGES_PER_BLOCK;
			/* Adjust end_page for the skipped block */
			end_page += NAND_PAGES_PER_BLOCK;
			if (page >= end_page)
				break;
		}

		nand_read_page(page, dst);
		dst += page_size;
	}

	debug("SPL: loaded %d pages (%d bad blocks skipped)\n",
	      total_pages, bad_blocks);

	return 0;
}

void nand_deselect(void)
{
}

int nand_page_size(void)
{
	return page_size;
}

int nand_size(void)
{
	return chip_size;
}

/* Weak default — needed for SPL link */
int nand_default_bbt(struct mtd_info *mtd)
{
	return 0;
}

unsigned int spl_nand_get_uboot_raw_page(void)
{
	return 0x20000; /* U-Boot proper at NAND offset 0x20000 */
}

// SPDX-License-Identifier: GPL-2.0
/*
 * RTL8198C NAND driver for U-Boot proper — minimal DMA read
 *
 * DMA read sequence ported from the proven OpenWrt driver.
 * Only read operations are implemented for now.
 */

#include <init.h>
#include <nand.h>
#include <linux/delay.h>
#include <linux/mtd/rawnand.h>
#include <asm/io.h>

/* Register definitions — shared with SPL nand_regs.h */
#define NAND_BASE		0xB801A000
#define NACR			0x04
#define NACMR			0x08
#define NAADR			0x0C
#define NADCRR			0x10
#define NADR			0x14
#define NADFSAR			0x18
#define NADRSAR			0x20
#define NADTSAR			0x24
#define NASR			0x28

#define NACR_READY		(1 << 31)
#define NACR_ECC_EN		(1 << 30)
#define NACR_TIMING		0x000FFFFF

#define NACMR_CECS0		(1 << 30)

#define NAADR_AD2EN		(1 << 26)
#define NAADR_AD1EN		(1 << 25)
#define NAADR_AD0EN		(1 << 24)

#define NADCRR_DESC0		(1 << 4)
#define NADCRR_DMARE		(1 << 3)
#define NADCRR_LBC64		2
#define NADCRR_DMA_READ		(NADCRR_DESC0 | NADCRR_DMARE | NADCRR_LBC64)

#define NASR_NDRS		(1 << 1)
#define NASR_NRER		(1 << 3)
#define NASR_NECN		(1 << 4)
#define NASR_ECC_MASK		0xF0
#define NASR_ECC_SHIFT		4
#define NASR_CLEAR		0x0F

#define NAND_SECTOR_SIZE	512
#define NAND_SECTORS_PER_PAGE	4
#define NAND_SECTOR_TOTAL	(NAND_SECTOR_SIZE + 16)

static void __iomem *nand_base = (void __iomem *)NAND_BASE;
static u8 dma_buf[NAND_SECTOR_TOTAL] __aligned(64);
static u8 oob_buf[16] __aligned(4);

/* ---- low-level helpers ---- */

static u32 kseg1_to_phys(void *addr)
{
	return (u32)addr & 0x1FFFFFFF;
}

static void nfc_wait_ready(void)
{
	while (!(readl(nand_base + NACR) & NACR_READY))
		udelay(1);
}

static void nfc_cmd(u8 cmd)
{
	writel(NACMR_CECS0 | cmd, nand_base + NACMR);
	nfc_wait_ready();
	writel(0, nand_base + NACMR);
}

static int nand_dma_read_sector(u32 flash_addr)
{
	u32 data_phys = kseg1_to_phys(dma_buf);
	u32 tag_phys  = data_phys + NAND_SECTOR_SIZE;

	writel(NACR_READY | NACR_ECC_EN | NACR_TIMING,
	       nand_base + NACR);
	writel(NASR_CLEAR, nand_base + NASR);

	writel(data_phys, nand_base + NADRSAR);
	writel(tag_phys,  nand_base + NADTSAR);
	writel(flash_addr, nand_base + NADFSAR);

	writel(NADCRR_DMA_READ, nand_base + NADCRR);
	nfc_wait_ready();

	return 0;
}

/* ---- nand_chip ops — U-Boot API uses struct mtd_info * first param ---- */

static void rtl8198c_select_chip(struct mtd_info *mtd, int cs)
{
}

static int rtl8198c_dev_ready(struct mtd_info *mtd)
{
	return 1;
}

static void rtl8198c_cmdfunc(struct mtd_info *mtd, unsigned int command,
			     int column, int page_addr)
{
	u32 addr;

	switch (command) {
	case NAND_CMD_RESET:
		writel(NACMR_CECS0 | NAND_CMD_RESET, nand_base + NACMR);
		nfc_wait_ready();
		writel(0, nand_base + NACMR);
		break;

	case NAND_CMD_READID:
		writel(NACMR_CECS0 | NAND_CMD_READID, nand_base + NACMR);
		nfc_wait_ready();
		writel(NAADR_AD0EN | 0x00, nand_base + NAADR);
		nfc_wait_ready();
		break;

	case NAND_CMD_READ0:
		break; /* handled by DMA read_page */

	case NAND_CMD_READSTART:
		break; /* handled by DMA read_page */

	case NAND_CMD_STATUS:
		writel(NACMR_CECS0 | NAND_CMD_STATUS, nand_base + NACMR);
		nfc_wait_ready();
		break;

	case NAND_CMD_PARAM:
		writel(NACMR_CECS0 | NAND_CMD_PARAM, nand_base + NACMR);
		nfc_wait_ready();
		writel(NAADR_AD0EN | (column & 0xFF), nand_base + NAADR);
		nfc_wait_ready();
		break;

	case NAND_CMD_RNDOUT:
		writel(NACMR_CECS0 | NAND_CMD_RNDOUT, nand_base + NACMR);
		nfc_wait_ready();
		addr = NAADR_AD1EN | NAADR_AD0EN |
		       ((column >> 8) << 8) | (column & 0xFF);
		writel(addr, nand_base + NAADR);
		nfc_wait_ready();
		break;

	case NAND_CMD_ERASE1:
		writel(NACMR_CECS0 | NAND_CMD_ERASE1, nand_base + NACMR);
		nfc_wait_ready();
		addr = NAADR_AD2EN | NAADR_AD1EN | NAADR_AD0EN |
		       (page_addr & 0xFF) |
		       ((page_addr >> 8) & 0xFF) << 8 |
		       ((page_addr >> 16) & 0xFF) << 16;
		writel(addr, nand_base + NAADR);
		nfc_wait_ready();
		break;

	case NAND_CMD_ERASE2:
		writel(NACMR_CECS0 | NAND_CMD_ERASE2, nand_base + NACMR);
		nfc_wait_ready();
		break;

	default:
		break;
	}
}

static u8 rtl8198c_read_byte(struct mtd_info *mtd)
{
	u32 word = readl(nand_base + NADR);
	return word & 0xFF;
}

static void rtl8198c_read_buf(struct mtd_info *mtd, u8 *buf, int len)
{
	int i;
	u32 word = 0;

	for (i = 0; i < len; i++) {
		if ((i & 3) == 0)
			word = readl(nand_base + NADR);
		buf[i] = (word >> ((i & 3) * 8)) & 0xFF;
	}
}

static int rtl8198c_read_page(struct mtd_info *mtd, struct nand_chip *chip,
			      uint8_t *buf, int oob_required, int page)
{
	u32 flash_addr = page << 12;
	int sector, ret = 0;

	for (sector = 0; sector < NAND_SECTORS_PER_PAGE; sector++) {
		nand_dma_read_sector(flash_addr);

		/* Check ECC */
		u32 nasr = readl(nand_base + NASR);
		if (!(nasr & NASR_NDRS))
			return -EIO;
		if (nasr & NASR_NRER) {
			int ecnt = (nasr & NASR_ECC_MASK) >> NASR_ECC_SHIFT;
			if (ecnt > 0 && ecnt <= 4) {
				writel(nasr & ~NASR_NECN, nand_base + NASR);
			} else {
				writel(nasr & ~NASR_NECN, nand_base + NASR);
				ret = -EBADMSG;
			}
		} else {
			writel(nasr & ~NASR_ECC_MASK, nand_base + NASR);
		}

		memcpy(buf + sector * NAND_SECTOR_SIZE,
		       dma_buf, NAND_SECTOR_SIZE);
		if (oob_required)
			memcpy(chip->oob_poi + sector * 16,
			       dma_buf + NAND_SECTOR_SIZE, 16);

		flash_addr += NAND_SECTOR_TOTAL;
	}

	return ret;
}

static int rtl8198c_read_oob(struct mtd_info *mtd, struct nand_chip *chip,
			     int page)
{
	u32 flash_addr = page << 12;
	int sector;

	for (sector = 0; sector < NAND_SECTORS_PER_PAGE; sector++) {
		nand_dma_read_sector(flash_addr);
		memcpy(chip->oob_poi + sector * 16,
		       dma_buf + NAND_SECTOR_SIZE, 16);
		flash_addr += NAND_SECTOR_TOTAL;
	}

	return 0;
}

/* ---- board_nand_init — registered with U-Boot NAND framework ---- */

static struct nand_chip rtl8198c_chip;

void board_nand_init(void)
{
	struct nand_chip *chip = &rtl8198c_chip;
	struct mtd_info *mtd;

	memset(chip, 0, sizeof(*chip));

	chip->select_chip = rtl8198c_select_chip;
	chip->dev_ready   = rtl8198c_dev_ready;
	chip->cmdfunc     = rtl8198c_cmdfunc;
	chip->read_byte   = rtl8198c_read_byte;
	chip->read_buf    = rtl8198c_read_buf;
	chip->ecc.mode    = NAND_ECC_HW;
	chip->ecc.strength = 4;
	chip->ecc.size    = NAND_SECTOR_SIZE;
	chip->ecc.read_page    = rtl8198c_read_page;
	chip->ecc.read_oob     = rtl8198c_read_oob;
	chip->options     = NAND_NO_SUBPAGE_WRITE;

	mtd = nand_to_mtd(chip);
	mtd->writesize = 2048;
	mtd->oobsize   = 64;
	mtd->erasesize = 128 * 1024;

	/* Hardcode W29N01HV geometry for now.
	 * ONFI detection could be added via nand_scan_ident() later.
	 */
	chip->chipsize = 128ULL * 1024 * 1024;
	chip->page_shift = 11;  /* 2K pages */
	chip->phys_erase_shift = 17;  /* 128K blocks */
	chip->bbt_erase_shift = 17;
	chip->chip_shift = 27;  /* 128 MiB */
	chip->pagemask = (chip->chipsize >> chip->page_shift) - 1;

	chip->numchips = 1;
	chip->bbt_options = NAND_BBT_USE_FLASH;

	nand_register(0, mtd);
}

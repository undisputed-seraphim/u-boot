/*
 * RTL8198C DDR3 Initialization — 580 MHz, 256 MiB, 16-bit
 *
 * Extracted from the original Realtek bootloader (btcode/start.S).
 * Values match CONFIG_DDR3_256MB_16bit + CONFIG_DDR3_580MHZ.
 *
 * Callable from C via: void dram_init_asm(void);
 */

#include <asm/asm.h>
#include <asm/regdef.h>

	.set noreorder
	.text

LEAF(dram_init_asm)

/* =================================================================
 * DDR ZQ calibration
 * ================================================================= */

	/* DBFM = 0x1e0 */
	lui	t0, 0xb800
	ori	t0, t0, 0x1000
	lui	t1, 0x0000
	ori	t1, t1, 0x01e0
	sw	t1, 0(t0)

	/* REG32(0xb8001090) &= ~(1<<25) */
	lui	t0, 0xb800
	ori	t0, t0, 0x1090
	lw	t1, 0(t0)
	lui	t2, 0xfdff
	ori	t2, t2, 0xffff
	and	t1, t1, t2
	sw	t1, 0(t0)

	/* REG32(0xb80000a0) |= (1<<24) — enable ZQ for RTL8198C */
	lui	t0, 0xb800
	ori	t0, t0, 0x00a0
	lw	t1, 0(t0)
	lui	t2, 0x0100
	or	t1, t1, t2
	sw	t1, 0(t0)

	/* REG32(0xb8001090) |= (1<<31) — Force ZQ */
	lui	t0, 0xb800
	ori	t0, t0, 0x1090
	lw	t1, 0(t0)
	lui	t2, 0x8000
	or	t1, t1, t2
	sw	t1, 0(t0)

	/* REG32(0xb8001094) = 0x80024B16 — Trigger calibration */
	lui	t0, 0xb800
	ori	t0, t0, 0x1094
	lui	t1, 0x8002
	ori	t1, t1, 0x4b16
	sw	t1, 0(t0)

	/* ODT low */
	lui	t0, 0xb800
	ori	t0, t0, 0x107c
	sw	zero, 0(t0)

/* =================================================================
 * DDR3 bank config — 256 MiB, 16-bit (DCR = 0x21320000)
 * ================================================================= */

	lui	t0, 0xb800
	ori	t0, t0, 0x1004
	lui	t1, 0x2132
	sw	t1, 0(t0)

/* =================================================================
 * DDR3 580MHz timing registers
 * ================================================================= */

	/* PLL and pad config — first group */
	lui	t0, 0xb800
	ori	t0, t0, 0x00a0
	lui	t1, 0x2900
	ori	t1, t1, 0x2107
	sw	t1, 0(t0)

	lui	t0, 0xb800
	ori	t0, t0, 0x1590
	lui	t1, 0x0808
	sw	t1, 0(t0)

	lui	t0, 0xb800
	ori	t0, t0, 0x0010
	lui	t1, 0x8000
	ori	t1, t1, 0x0908
	sw	t1, 0(t0)

	lui	t0, 0xb800
	ori	t0, t0, 0x1800
	lui	t1, 0xc108
	sw	t1, 0(t0)

	lui	t0, 0xb800
	ori	t0, t0, 0x1064
	sw	zero, 0(t0)			/* = 0x000000ff */

	lui	t0, 0xb800
	ori	t0, t0, 0x00dc
	lui	t1, 0x0000
	ori	t1, t1, 0x0400
	sw	t1, 0(t0)

	lui	t0, 0xb800
	ori	t0, t0, 0x1804
	lui	t1, 0x150f
	sw	t1, 0(t0)

	/* Memory controller registers */
	lui	t0, 0xb800
	ori	t0, t0, 0x1000
	lui	t1, 0x2204
	ori	t1, t1, 0x1de0
	sw	t1, 0(t0)

	lui	t0, 0xb800
	ori	t0, t0, 0x1008		/* DTR */
	lui	t1, 0x8864
	ori	t1, t1, 0x4740
	sw	t1, 0(t0)

	lui	t0, 0xb800
	ori	t0, t0, 0x100c		/* DTR1 */
	lui	t1, 0x0808
	ori	t1, t1, 0x051d
	sw	t1, 0(t0)

	lui	t0, 0xb800
	ori	t0, t0, 0x1010		/* DTR2 */
	lui	t1, 0x0c01
	ori	t1, t1, 0xa000
	sw	t1, 0(t0)

	lui	t0, 0xb800
	ori	t0, t0, 0x1500		/* DACCR */
	lui	t1, 0x8001
	ori	t1, t1, 0x0210
	sw	t1, 0(t0)

	lui	t0, 0xb800
	ori	t0, t0, 0x1050		/* DPHY */
	lui	t1, 0x1f1f
	sw	t1, 0(t0)

	/* Per-byte-lane calibration */
	lui	t1, 0x0804
	ori	t1, t1, 0x0200

	lui	t0, 0xb800
	ori	t0, t0, 0x1510
	sw	t1, 0(t0)
	lui	t0, 0xb800
	ori	t0, t0, 0x1514
	sw	t1, 0(t0)
	lui	t0, 0xb800
	ori	t0, t0, 0x1518
	sw	t1, 0(t0)
	lui	t0, 0xb800
	ori	t0, t0, 0x151c
	sw	t1, 0(t0)
	lui	t0, 0xb800
	ori	t0, t0, 0x1520
	sw	t1, 0(t0)
	lui	t0, 0xb800
	ori	t0, t0, 0x1524
	sw	t1, 0(t0)
	lui	t0, 0xb800
	ori	t0, t0, 0x1528
	sw	t1, 0(t0)
	lui	t0, 0xb800
	ori	t0, t0, 0x152c
	sw	t1, 0(t0)
	lui	t0, 0xb800
	ori	t0, t0, 0x1530
	sw	t1, 0(t0)
	lui	t0, 0xb800
	ori	t0, t0, 0x1534
	sw	t1, 0(t0)
	lui	t0, 0xb800
	ori	t0, t0, 0x1538
	sw	t1, 0(t0)
	lui	t0, 0xb800
	ori	t0, t0, 0x153c
	sw	t1, 0(t0)
	lui	t0, 0xb800
	ori	t0, t0, 0x1540
	sw	t1, 0(t0)
	lui	t0, 0xb800
	ori	t0, t0, 0x1544
	sw	t1, 0(t0)
	lui	t0, 0xb800
	ori	t0, t0, 0x1548
	sw	t1, 0(t0)
	lui	t0, 0xb800
	ori	t0, t0, 0x154c
	sw	t1, 0(t0)

	lui	t1, 0x0004
	ori	t1, t1, 0x0200

	lui	t0, 0xb800
	ori	t0, t0, 0x1550
	sw	t1, 0(t0)
	lui	t0, 0xb800
	ori	t0, t0, 0x1554
	sw	t1, 0(t0)
	lui	t0, 0xb800
	ori	t0, t0, 0x1558
	sw	t1, 0(t0)
	lui	t0, 0xb800
	ori	t0, t0, 0x155c
	sw	t1, 0(t0)
	lui	t0, 0xb800
	ori	t0, t0, 0x1560
	sw	t1, 0(t0)
	lui	t0, 0xb800
	ori	t0, t0, 0x1564
	sw	t1, 0(t0)
	lui	t0, 0xb800
	ori	t0, t0, 0x1568
	sw	t1, 0(t0)
	lui	t0, 0xb800
	ori	t0, t0, 0x156c
	sw	t1, 0(t0)
	lui	t0, 0xb800
	ori	t0, t0, 0x1570
	sw	t1, 0(t0)
	lui	t0, 0xb800
	ori	t0, t0, 0x1574
	sw	t1, 0(t0)
	lui	t0, 0xb800
	ori	t0, t0, 0x1578
	sw	t1, 0(t0)
	lui	t0, 0xb800
	ori	t0, t0, 0x157c
	sw	t1, 0(t0)
	lui	t0, 0xb800
	ori	t0, t0, 0x1580
	sw	t1, 0(t0)
	lui	t0, 0xb800
	ori	t0, t0, 0x1584
	sw	t1, 0(t0)
	lui	t0, 0xb800
	ori	t0, t0, 0x1588
	sw	t1, 0(t0)
	lui	t0, 0xb800
	ori	t0, t0, 0x158c
	sw	t1, 0(t0)

	/* MRS commands: MR2 → MR3 → MR1 → MR0 */
	lui	t0, 0xb800
	ori	t0, t0, 0x101c

	lui	t1, 0x0012
	ori	t1, t1, 0x0210
	sw	t1, 0(t0)

	lui	t1, 0x0013
	sw	t1, 0(t0)			/* MR3 */

	lui	t1, 0x0011
	ori	t1, t1, 0x0042
	sw	t1, 0(t0)			/* MR1 */

	lui	t1, 0x0010
	ori	t1, t1, 0x1b40
	sw	t1, 0(t0)			/* MR0 */

/* =================================================================
 * DDR3 PLL — set memory clock to 580 MHz (17 << 10)
 * ================================================================= */

	lui	t0, 0xb800
	ori	t0, t0, 0x0008
	lw	t1, 0(t0)
	lui	t2, 0xffff
	ori	t2, t2, 0x83ff
	and	t1, t1, t2
	ori	t1, t1, 0x4400		/* 17 << 10 = 0x4400 */
	sw	t1, 0(t0)

/* =================================================================
 * CPU frequency — 800 MHz for NAND boot (7 << 15)
 * ================================================================= */

	lui	t0, 0xb800
	ori	t0, t0, 0x0008
	lw	t1, 0(t0)
	lui	t2, 0x0003
	ori	t2, t2, 0x8000		/* 7 << 15 = 0x38000 */
	or	t1, t1, t2
	sw	t1, 0(t0)

/* =================================================================
 * CKE toggle (DRAM reset sequence)
 * ================================================================= */

	/* Wait for DMCR ready */
	lui	t0, 0xb800
	ori	t0, t0, 0x101c
1:
	lw	t1, 0(t0)
	bltz	t1, 1b
	nop

	/* CKE low */
	lui	t0, 0xb800
	ori	t0, t0, 0x1040
	lw	t1, 0(t0)
	lui	t2, 0x1000
	or	t1, t1, t2
	sw	t1, 0(t0)

	/* CKE high */
	lui	t0, 0xb800
	ori	t0, t0, 0x1040
	lw	t1, 0(t0)
	lui	t2, 0x0fff
	ori	t2, t2, 0xffff
	and	t1, t1, t2
	sw	t1, 0(t0)

/* =================================================================
 * DRAM ZQ long calibration
 * ================================================================= */

	lui	t0, 0xb800
	ori	t0, t0, 0x1080
	lw	t1, 0(t0)
	lui	t2, 0x8000
	or	t1, t1, t2
	sw	t1, 0(t0)

/* =================================================================
 * Enable DRAM refresh
 * ================================================================= */

	lui	t0, 0xb800
	ori	t0, t0, 0x101c
	lw	t1, 0(t0)
	lui	t2, 0xfeff
	ori	t2, t2, 0xffff
	and	t1, t1, t2
	sw	t1, 0(t0)

/* =================================================================
 * ODT termination enable
 * ================================================================= */

	lui	t0, 0xb800
	ori	t0, t0, 0x107c
	lui	t1, 0xc000
	sw	t1, 0(t0)

/* =================================================================
 * Return
 * ================================================================= */

	jr	ra
	 nop

	END(dram_init_asm)

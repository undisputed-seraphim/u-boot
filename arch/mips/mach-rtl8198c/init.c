// SPDX-License-Identifier: GPL-2.0
/*
 * Realtek RTL8198C — SoC init and CPU info
 */

#include <init.h>
#include <stdio.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include "rtl8198c.h"

DECLARE_GLOBAL_DATA_PTR;

int print_cpuinfo(void)
{
	void __iomem *sysc = ioremap(SYSCTL_BASE, SYSCTL_SIZE);
	u32 info, id, model, ver;

	info = readl(sysc + SYSCTL_CHIP_INFO);
	id   = (info >> 16) & 0xffff;
	ver  = (info >> 8) & 0xf;
	model = id;
	(void)model;

	info = readl(sysc + SYSCTL_CHIP_REVID);
	/* info provides additional revision data */

	printf("CPU:   Realtek RTL%04X rev %c\n", id, 'A' + ver);

	/* Use CP0 count at half CPU rate for timer */
	gd->arch.timer_freq = 400000000;

	return 0;
}

void _machine_restart(void)
{
	void __iomem *tc = ioremap(TC_BASE, 0x20);

	/* Trigger immediate watchdog reset */
	writel(0, tc + WDTCNR_OFFSET);

	while (1)
		;
}

void lowlevel_init(void)
{
}

// SPDX-License-Identifier: GPL-2.0
/*
 * RTL8198C SPL — DDR init → NAND load → U-Boot proper
 */

#include <debug_uart.h>
#include <spl.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

void dram_init_asm(void);
void board_init_r(struct spl_image_info *spl_image, ulong jump_to);

void board_init_f(ulong dummy)
{
	debug_uart_init();
	printascii("\nSPL: RTL8198C AP5100W (U-Boot SPL)\n");

	printascii("SPL: initializing DDR3...\n");
	dram_init_asm();
	printascii("SPL: DDR3 init done\n");

	preloader_console_init();

	spl_init();

	printascii("SPL: loading U-Boot from NAND...\n");
	board_init_r(NULL, 0);
}

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_NAND;
}

void timer_init(void)
{
	/* MIPS CP0 timer runs from CP0 Count register */
}

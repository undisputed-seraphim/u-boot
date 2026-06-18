// SPDX-License-Identifier: GPL-2.0
/*
 * Askey AP5100W  board support
 *
 * RTL8198C SoC, 256 MiB DDR3, 128 MiB NAND, dual-band WiFi
 */

#include <init.h>
#include <fdtdec.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	return 0;
}

int board_init(void)
{
	return 0;
}

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
	fdtdec_setup_memory_banksize();
	return 0;
}

int board_fdt_blob_setup(void **fdtp)
{
	*fdtp = NULL;
	return 0;
}

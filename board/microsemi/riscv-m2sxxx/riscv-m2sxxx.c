/*
 * Copyright (c) 2016 Microsemi Corporation.
 * Padmarao Begari, Microsemi Corporation <padmarao.begari@microsemi.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <netdev.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Miscellaneous platform dependent initializations
 */

int board_init(void)
{

    gd->bd->bi_arch_number = MACH_TYPE_RISCVM2S;
    gd->bd->bi_boot_params = PHYS_SDRAM_0 + 0x400;

    return 0;
}

int dram_init(void)
{
    unsigned long sdram_base = PHYS_SDRAM_0;
    unsigned long expected_size = PHYS_SDRAM_0_SIZE;
    unsigned long actual_size;

    actual_size = get_ram_size((void *)sdram_base, expected_size);

    gd->ram_size = actual_size;

    if (expected_size != actual_size) {
        printf("Warning: Only %lu of %lu MiB SDRAM is working\n",
            actual_size >> 20, expected_size >> 20);
    }

    return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_0;
	gd->bd->bi_dram[0].size =  PHYS_SDRAM_0_SIZE;
}

int board_eth_init(bd_t *bd)
{
	return 0; 
}

ulong board_flash_get_legacy(ulong base, int banknum, flash_info_t *info)
{
    return 0;
}

int board_mmc_init(bd_t *bis)
{
    return 0;
}

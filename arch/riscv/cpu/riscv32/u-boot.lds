/*
 * (c) Copyright 2016 Microsemi Corporation.
 
 * Padmarao Begari, Microsemi Corporation <padmarao.begari@microsemi.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* OUTPUT_FORMAT("elf32-riscv32", "elf32-riscv32", "elf32-riscv32") */
OUTPUT_ARCH("riscv")
ENTRY(_start)

STACK_SIZE = 128k;
HEAP_SIZE = 128k;

SECTIONS
{
    . = 0x80000000;
	. = ALIGN(4);
	.text :
	{
		arch/riscv/cpu/riscv32/start.o	(.text)
		*(.text)
	}

	. = ALIGN(4);
	.rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }

	. = ALIGN(4);
	.data : { *(.data*) }

	. = ALIGN(4);

/*	.got : {
		__got_start = .;
		 *(.got.plt) *(.got)
		__got_end = .;
	}
*/
	. = .;

	. = ALIGN(4);
	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	}

	. = ALIGN(4);

	_end = .;

	.bss : {
		__bss_start = .;
		*(.bss)
		 . = ALIGN(4);
		__bss_end = .;
	}

}

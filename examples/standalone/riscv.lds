/*
 * Copyright (C) 2016 Microsemi Corporation 
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

OUTPUT_FORMAT("elf32-littleriscv", "elf32-littleriscv", "elf32-littleriscv")
OUTPUT_ARCH(riscv)
ENTRY(_start)
SECTIONS
{
    . = ALIGN(4);
    .text :
    {
        *(.text)
    }

    . = ALIGN(4);
    .data : { *(.data) }

    . = ALIGN(4);

    .got : {
        __got_start = .;
        *(.got)
        __got_end = .;
    }
 
     . = ALIGN(4);
    __bss_start = .;
    .bss : { *(.bss) }
    __bss_end = .;

    . = ALIGN(4);
    .rela.text : { *(.rela.text .rela.text.* .rela.gnu.linkonce.t.*) } 

    _end = .;
}
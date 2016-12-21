/*
 * Copyright (C) 2016 Microsemi Corporation.
 * Padmarao Begari, Microsemi Corporation <padmarao.begari@microsemi.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H


/*
 * CPU and Board Configuration Options
 */
#define CONFIG_RISCV_M2SXXX

#define CONFIG_USE_INTERRUPT

#define CONFIG_SKIP_LOWLEVEL_INIT

/*
 * Definitions related to passing arguments to kernel.
 */
#define CONFIG_ARCH_CPU_INIT
#define CONFIG_CMDLINE_TAG          /* send commandline to Kernel */
#define CONFIG_SETUP_MEMORY_TAGS    /* send memory definition to kernel */
#define CONFIG_INITRD_TAG           /* send initrd params */

/* #define CONFIG_NEEDS_MANUAL_RELOC */

#define CONFIG_SYS_TEXT_BASE    0x80000000 /* SDRAM  */

#define CONFIG_SYS_CLK_FREQ     166000000  

/* #define CONFIG_SYS_UBOOT_START 0x80000000 */

#define TIMER_LOAD_VAL  		0xffffffff

/*
 * Serial console configuration
 */
#define CONFIG_MSCC_COREUART
#define CONFIG_USART_BASE       	0x70001000
#define CONFIG_USART_ID         	1
#define CONFIG_BAUDRATE         	115200
#define CONFIG_SYS_COREUART_CLK     83000000
#define CONSOLE_ARG					"console=console=ttyS0,115200\0"

#define CONFIG_CORETIMER_BASE   	0x70003000

#define CONFIG_BOOTDELAY    		3

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP         /* undef to save memory */
#define CONFIG_SYS_CBSIZE   1024     /* Console I/O Buffer Size */

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE  (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

/* max number of command args */
#define CONFIG_SYS_MAXARGS  16

/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE

/*
 * Size of malloc() pool
 */
/* 512kB is suggested, (CONFIG_ENV_SIZE + 128 * 1024) was not enough */
 #define CONFIG_SYS_MALLOC_LEN   (512 << 10)


/*
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS    1   /* we have 1 bank of DRAM */
#define PHYS_SDRAM_0            0x80000000  /* SDRAM Bank #1 */
#define PHYS_SDRAM_1            (PHYS_SDRAM_0 + PHYS_SDRAM_0_SIZE) /* SDRAM Bank #2 */
#define PHYS_SDRAM_0_SIZE       0x10000000    /* 256 MB */

#define CONFIG_SYS_SDRAM_BASE   PHYS_SDRAM_0
#define CONFIG_SYS_SDRAM_SIZE	PHYS_SDRAM_0_SIZE

#define CONFIG_SYS_INIT_SP_ADDR (CONFIG_SYS_SDRAM_BASE + 0x01f1000 - \
   GENERATED_GBL_DATA_SIZE)


/* NOR flash - no real flash on this board */
#define CONFIG_SYS_NO_FLASH
#define CONFIG_SYS_MAX_FLASH_SECT	0
#define CONFIG_SYS_MAX_FLASH_BANKS 0
/*
 * Load address and memory test area should agree with
 * arch/riscv/config.mk. Be careful not to overwrite U-Boot itself.
 */
#define CONFIG_SYS_LOAD_ADDR 0x80100000 /* SDRAM */

#define CONFIG_LOADADDR			0x80100000

/* memtest works on 512 MB in DRAM */
#define CONFIG_SYS_MEMTEST_START PHYS_SDRAM_0
#define CONFIG_SYS_MEMTEST_END (PHYS_SDRAM_0 + PHYS_SDRAM_0_SIZE)

/*
 * Memory organisation:
 *
 * RAM start ---------------------------
 *           | ...                     |
 *           ---------------------------
 *           | Stack                   |
 *           ---------------------------
 *           | Global data             |
 *           ---------------------------
 *           | Environment             |
 *           ---------------------------
 *           | Monitor                 |
 * RAM end   ---------------------------
 */

/*#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE		0x20000*/  /* Total Size of Environment, 128KB */

#define CONFIG_SPI
#define CONFIG_CORESPI_MICROSEMI
#define CORESPI_BASE_ADDRESS		0x70006000
#define CORESPI_SLAVE_SELECT		0
#define CONFIG_SYS_SPI_BASE			CORESPI_BASE_ADDRESS
#define CONFIG_SYS_SPI_CLK			83000000
#define CONFIG_SF_DEFAULT_SPEED		83000000
#define CONFIG_SF_DEFAULT_BUS		0
#define CONFIG_ENV_SPI_MAX_HZ		CONFIG_SF_DEFAULT_SPEED

#define CONFIG_SPI_FLASH          1
#define CONFIG_SPI_FLASH_STMICRO
#define CONFIG_SPI_FLASH_BAR
#define CONFIG_CMD_SF
#define CONFIG_SF_DEFAULT_MODE            SPI_MODE_0


/*

* Env Storage Settings

*/

#define CONFIG_ENV_IS_IN_SPI_FLASH
#if defined(CONFIG_ENV_IS_IN_SPI_FLASH)
#define CONFIG_ENV_OFFSET          0x10000
#define CONFIG_ENV_SIZE            0x2000
#define CONFIG_ENV_SECT_SIZE       0x10000
#define CONFIG_ENV_SPI_BUS		0
#define CONFIG_ENV_SPI_CS		0
#define CONFIG_ENV_SPI_MODE		SPI_MODE_0
#define CONFIG_ENV_SPI_MAX_HZ	CONFIG_SF_DEFAULT_SPEED
#endif

#endif /* __CONFIG_H */

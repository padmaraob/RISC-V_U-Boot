/*
 * Copyright (c) 2016 Microsemi Corporation.
 * Padmarao Begari, Microsemi Corporation <padmarao.begari@microsemi.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef _ASM_RISCV_CACHE_H
#define _ASM_RISCV_CACHE_H
/*
 * The current upper bound for RISCV L1 data cache line sizes is 64 bytes.
 * We use that value for aligning DMA buffers unless the board config has
 * specified an alternate cache line size.
 */
#ifdef CONFIG_SYS_CACHELINE_SIZE
#define ARCH_DMA_MINALIGN   CONFIG_SYS_CACHELINE_SIZE
#else
#define ARCH_DMA_MINALIGN   64
#endif

#define L1_CACHE_SHIFT		6
#define L1_CACHE_BYTES		(1 << L1_CACHE_SHIFT)

#endif /* _ASM_RISCV_CACHE_H */

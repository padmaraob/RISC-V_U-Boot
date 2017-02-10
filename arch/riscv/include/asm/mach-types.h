/*
 * Copyright (c) 2017 Microsemi Corporation.
 * Padmarao Begari, Microsemi Corporation <padmarao.begari@microsemi.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_RISCV_MACH_TYPE_H
#define __ASM_RISCV_MACH_TYPE_H

#ifndef __ASSEMBLY__
/* The type of machine we're running on */
extern unsigned int __machine_arch_type;
#endif

/* see arch/arm/kernel/arch.c for a description of these */
#define MACH_TYPE_RISCVM2S            1

#ifdef CONFIG_ARCH_RISCVM2S 
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type __machine_arch_type
# else
#  define machine_arch_type MACH_TYPE_RISCVM2S
# endif
# define machine_is_riscvm2s() (machine_arch_type == MACH_TYPE_RISCVM2S)
#else
# define machine_is_riscvm2s() (1)
#endif

#endif /* __ASM_RISCV_MACH_TYPE_H */

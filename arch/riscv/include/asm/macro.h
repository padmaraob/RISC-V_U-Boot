/*
 * include/asm-riscv/macro.h
 *
 * Copyright (c) 2016 Microsemi Corporation.
 * Padmarao Begari, Microsemi Corporation <padmarao.begari@microsemi.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef __ASM_RISCV_MACRO_H
#define __ASM_RISCV_MACRO_H
#ifdef __ASSEMBLY__

/*
 * These macros provide a convenient way to write 8, 16 and 32 bit data
 * to an "immediate address (address used by periphal)" only.
 * Registers r4 and r5 are used, any data in these registers are
 * overwritten by the macros.
 * The macros are valid for any riscv architecture, they do not implement
 * any memory barriers so caution is recommended when using these when the
 * caches are enabled or on a multi-core system.
 */

.macro	write32, addr, data
	li  t4, \addr
	li  t5, \data
	sw  t5, (t4)
.endm

.macro	write16, addr, data
	li t4, \addr
	li t5, \data
	sh t5, (t4)
.endm

.macro	write8, addr, data
	li  t4, \addr
	li  t5, \data
	sb  t5, (t4)
.endm

/*
 * This macro read a value from a register, then do OR operation
 * (set bit fields) to the value, and then store it back to the register.
 * Note: Instruction 'ori' supports immediate value up to 15 bits.
 */
.macro	setbf32, addr, data
	li	t4, \addr
	lw	t5, (t4)
	li	t6, \data
	or	t5, t5, t6
	sw	t5, (t4)
.endm

.macro	setbf15, addr, data
	li	t4, \addr
	lw  t5, (t4)
	ori	t5, t5, \data
	sw  t5, (t4)
.endm

/*
 * This macro generates a loop that can be used for delays in the code.
 * Register t4 is used, any data in this register is overwritten by the
 * macro.
 * The macro is valid for any riscv architeture. The actual time spent in the
 * loop will vary from CPU to CPU though.
 */

.macro	wait_timer, time
	li	t4, \time
1:
	nop
	addi	t4, t4, -1
	bnez    t4, 1b
.endm

#endif /* __ASSEMBLY__ */
#endif /* __ASM_RISCV_MACRO_H */

/*
 * Startup Code for RISC-V Core
 *
 * Copyright (c) 2017 Microsemi Corporation.
 * Copyright (c) 2017 Padmarao Begari <Padmarao.Begari@microsemi.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <asm-offsets.h>
#include <config.h>
#include <common.h>
#include <asm/macro.h>
#include <asm/encoding.h>

#ifdef __riscv64
# define LREG ld
# define SREG sd
# define REGBYTES 8
#else
# define LREG lw
# define SREG sw
# define REGBYTES 4
# define RISCV_RELOC 3

# define DYNSYM 16
#endif

    .section      .text
    .globl _start

_start:
    j handle_reset

nmi_vector:
    j nmi_vector

trap_vector:
    j trap_entry

handle_reset:
    la t0, trap_entry
    csrw mtvec, t0
    csrwi mstatus, 0
    csrwi mideleg, 0
    csrwi medeleg, 0
    csrwi mie, 0

#ifndef CONFIG_SKIP_LOWLEVEL_INIT
  /* Initialize any external memory */
    la t1, lowlevel_init
    jalr t1
    
  /* Initialize caches... */
  /* ... and enable them */
#endif

/*
 * Do CPU critical regs init only at reboot,
 * not when booting from ram
 */
#ifdef CONFIG_INIT_CRITICAL
    jal cpu_init_crit  /* Do CPU critical regs init */
#endif

/*
 * Set stackpointer in internal/ex RAM to call board_init_f
 */
call_board_init_f:
    li  t0, -16
    li  t1, CONFIG_SYS_INIT_SP_ADDR
    and sp, t1, t0  /* force 16 byte alignment */

#ifdef CONFIG_SYS_MALLOC_F_LEN
    li t2, CONFIG_SYS_MALLOC_F_LEN
    sub sp, sp, t2  /* reserve space for early malloc */
#endif

    li t3, GD_SIZE
    sub sp, sp, t3 /* reserve space for gd */
    and sp, sp, t0  /* force 16 byte alignment */
    mv gp, sp   /* save gd pointer */
  
    /* Clear gd */
    mv t0, gp
1:
    SREG zero, 0(t0)
    addi t0, t0, 4    
    blt t0, t1, 1b
    
#ifdef CONFIG_SYS_MALLOC_F_LEN
    SREG sp, GD_MALLOC_BASE(gp)  /* gd->malloc_base offset */
#endif

#if (CONFIG_SYS_TEXT_BASE == CONFIG_SYS_SDRAM_BASE)

    /* Update dynamic relocations before board_init_f */
    la  t1, __rel_dyn_start
    la  t2, __rel_dyn_end

    beq t1, t2, call_board_init_f_0

 /*   addi t1, t1, 12 */ /* skip first reserved entry: address, type, addend */

    bne t1, t2, 2f

1:
    lw  t5, -8(t1)      /* t5 <-- relocation info:type */

    li  t3, RISCV_RELOC /* reloc type R_RISCV_RELATIVE */
    bne t5, t3, 3f     /* skip non-RISCV_RELOC entries */

    LREG t3, -12(t1) /* t3 <-- location to fix up in FLASH:address */
    LREG t5, -4(t1)   /* t5 <-- addend */

    SREG t5, 0(t3)

2:
    addi t1, t1, 12 /* each rela.dyn entry is 12 bytes */
    blt t1, t2, 1b

3:
    la  t4, __dyn_sym_start

4:
    lw  t5, -8(t1)      /* t5 <-- relocation info:type */
    srli t0, t5, 0x8    /* t0 <--- sym table index */
    andi t5, t5, 0xFF   /* t5 <--- relocation type */

    li  t3, 0x1         /* reloc type R_RISCV_32 */
    bne t5, t3, 5f     /* skip non-addned entries */

    LREG t3, -12(t1)    /* t3 <-- location to fix up in FLASH:address */
    li t5, 16
    mul t0, t0, t5
    add s1, t4, t0
    LREG t5, 4(s1)      /* s1 <-- dyn sym table index - addr value */

    SREG t5, 0(t3)

5:
    addi t1, t1, 12     /* each rela.dyn entry is 12 bytes */
    blt t1, t2, 4b

#endif /* CONFIG_SYS_TEXT_BASE */

call_board_init_f_0:
    mv  a0, zero   /* a0 <-- boot_flags = 0 */
    la t5, board_init_f
    jr t5     /* jump to board_init_f() */

/*
 * void relocate_code (addr_sp, gd, addr_moni)
 *
 * This "function" does not return, instead it continues in RAM
 * after relocating the monitor code.
 *
 */
.globl relocate_code

relocate_code:
    mv  s2, a0  /* save addr_sp */    
    mv  s3, a1  /* save addr of gd */
    mv  s4, a2  /* save addr of destination */

/* Set up the stack */
stack_setup:
    mv sp, s2
    
    la t0, _start
    sub t6, s4, t0  /* t6 <- relocation offset */

    beq t0, s4, clear_bss  /* skip relocation */

    mv t1, s4       /* t1 <- scratch for copy_loop */
    la t3, __bss_start
    sub t3, t3, t0  /* t3 <- __bss_start_ofs */
    add t2, t0, t3  /* t2 <- source end address */

copy_loop:
    LREG t5, 0(t0)
    addi t0, t0, 4
    SREG t5, 0(t1)
    addi t1, t1, 4
    blt t0, t2, copy_loop

fix_rela_dyn:

    /* Update dynamic relocations after board_init_f */

    la  t1, __rel_dyn_start
    la  t2, __rel_dyn_end

    beq t1, t2, clear_bss

    add t1, t1, t6  /* t1 <- rela_dyn_start in RAM */
    add t2, t2, t6  /* t2 <- rela_dyn_end in RAM */

    /* skip first reserved entry: address, type, addend */
    bne t1, t2, 7f

6:
    lw  t5, -8(t1)      /* t5 <-- relocation info:type */

    li  t3, RISCV_RELOC /* reloc type R_RISCV_RELATIVE */
    bne t5, t3, 8f      /* skip non-RISCV_RELOC entries */

    LREG t3, -12(t1)    /* t3 <-- location to fix up in FLASH:address */
    LREG t5, -4(t1)     /* t5 <-- addend */

    add t5, t5, t6      /* t5 <-- location to fix up in RAM */
    add t3, t3, t6      /* t3 <-- location to fix up in RAM */
    SREG t5, 0(t3)

7:
    addi t1, t1, 12 /* each rela.dyn entry is 12 bytes */
    blt t1, t2, 6b
8:

    la  t4, __dyn_sym_start
    add t4, t4, t6
9:
    lw  t5, -8(t1)      /* t5 <-- relocation info:type */
    srli t0, t5, 0x8    /* t0 <--- sym table index */
    andi t5, t5, 0xFF   /* t5 <--- relocation type */

    li  t3, 0x1         /* reloc type R_RISCV_32 */
    bne t5, t3, 10f     /* skip non-addned entries */

    LREG t3, -12(t1)    /* t3 <-- location to fix up in FLASH:address */
    li t5, 16
    mul t0, t0, t5
    add s1, t4, t0
    LREG t5, 4(s1)      /* s1 <-- dyn sym table index - addr value */

    add t5, t5, t6      /* t5 <-- location to fix up in RAM */
    add t3, t3, t6      /* t3 <-- location to fix up in RAM */

    SREG t5, 0(t3)

10:
    addi t1, t1, 12 /* each rel.dyn entry is 12 bytes */
    blt t1, t2, 9b

    /* trap update */
    la t0, trap_entry
    add t0, t0, t6
    csrw mtvec, t0

clear_bss:
    la t0, __bss_start /* t0 <- rel __bss_start in FLASH */
    add t0, t0, t6  /* t0 <- rel __bss_start in RAM */
    la t1, __bss_end  /* t1 <- rel __bss_end in FLASH */
    add t1, t1, t6  /* t1 <- rel __bss_end in RAM */
    li t2, 0x00000000  /* clear */

    beq t0, t1, call_board_init_r
clbss_l:

    SREG t2, 0(t0)  /* clear loop... */
    addi t0, t0, 4
    bne t0, t1, clbss_l

/*
 * We are done. Do not return, instead branch to second part of board
 * initialization, now running from RAM.
 */
call_board_init_r:
    la t0, board_init_r
    mv t4, t0  /* offset of board_init_r() */
    add t4, t4, t6  /* real address of board_init_r() */
    /* setup parameters for board_init_r */
    mv a0, s3  /* gd_t */
    mv a1, s4  /* dest_addr */

    /* jump to it ... */
    jr t4   /* jump to board_init_r() */

/*  trap entry */
trap_entry:
    addi sp, sp, -32*REGBYTES

    SREG x1, 1*REGBYTES(sp)
    SREG x2, 2*REGBYTES(sp)
    SREG x3, 3*REGBYTES(sp)
    SREG x4, 4*REGBYTES(sp)
    SREG x5, 5*REGBYTES(sp)
    SREG x6, 6*REGBYTES(sp)
    SREG x7, 7*REGBYTES(sp)
    SREG x8, 8*REGBYTES(sp)
    SREG x9, 9*REGBYTES(sp)
    SREG x10, 10*REGBYTES(sp)
    SREG x11, 11*REGBYTES(sp)
    SREG x12, 12*REGBYTES(sp)
    SREG x13, 13*REGBYTES(sp)
    SREG x14, 14*REGBYTES(sp)
    SREG x15, 15*REGBYTES(sp)
    SREG x16, 16*REGBYTES(sp)
    SREG x17, 17*REGBYTES(sp)
    SREG x18, 18*REGBYTES(sp)
    SREG x19, 19*REGBYTES(sp)
    SREG x20, 20*REGBYTES(sp)
    SREG x21, 21*REGBYTES(sp)
    SREG x22, 22*REGBYTES(sp)
    SREG x23, 23*REGBYTES(sp)
    SREG x24, 24*REGBYTES(sp)
    SREG x25, 25*REGBYTES(sp)
    SREG x26, 26*REGBYTES(sp)
    SREG x27, 27*REGBYTES(sp)
    SREG x28, 28*REGBYTES(sp)
    SREG x29, 29*REGBYTES(sp)
    SREG x30, 30*REGBYTES(sp)
    SREG x31, 31*REGBYTES(sp)

    csrr a0, mcause
    csrr a1, mepc
    mv a2, sp
    jal handle_trap
    csrw mepc, a0

    /* Remain in M-mode after mret */
    li t0, MSTATUS_MPP
    csrs mstatus, t0

    LREG x1, 1*REGBYTES(sp)
    LREG x2, 2*REGBYTES(sp)
    LREG x3, 3*REGBYTES(sp)
    LREG x4, 4*REGBYTES(sp)
    LREG x5, 5*REGBYTES(sp)
    LREG x6, 6*REGBYTES(sp)
    LREG x7, 7*REGBYTES(sp)
    LREG x8, 8*REGBYTES(sp)
    LREG x9, 9*REGBYTES(sp)
    LREG x10, 10*REGBYTES(sp)
    LREG x11, 11*REGBYTES(sp)
    LREG x12, 12*REGBYTES(sp)
    LREG x13, 13*REGBYTES(sp)
    LREG x14, 14*REGBYTES(sp)
    LREG x15, 15*REGBYTES(sp)
    LREG x16, 16*REGBYTES(sp)
    LREG x17, 17*REGBYTES(sp)
    LREG x18, 18*REGBYTES(sp)
    LREG x19, 19*REGBYTES(sp)
    LREG x20, 20*REGBYTES(sp)
    LREG x21, 21*REGBYTES(sp)
    LREG x22, 22*REGBYTES(sp)
    LREG x23, 23*REGBYTES(sp)
    LREG x24, 24*REGBYTES(sp)
    LREG x25, 25*REGBYTES(sp)
    LREG x26, 26*REGBYTES(sp)
    LREG x27, 27*REGBYTES(sp)
    LREG x28, 28*REGBYTES(sp)
    LREG x29, 29*REGBYTES(sp)
    LREG x30, 30*REGBYTES(sp)
    LREG x31, 31*REGBYTES(sp)

    addi sp, sp, 32*REGBYTES
    mret

#ifdef CONFIG_INIT_CRITICAL
cpu_init_crit:
    ret
#endif

#ifndef CONFIG_SKIP_LOWLEVEL_INIT
lowlevel_init:
    ret
#endif

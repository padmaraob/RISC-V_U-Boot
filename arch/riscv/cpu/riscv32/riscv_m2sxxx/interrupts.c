/*
 * Copyright (c) 2016 Microsemi Corporation.
 * Padmarao Begari, Microsemi Corporation <padmarao.begari@microsemi.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/ptrace.h>
#include <asm/system.h>
#include <asm/encoding.h>
#include <asm/riscv_plic.h>

static void _exit_trap(int code);

#ifdef CONFIG_USE_IRQ
int interrupt_init(void)
{
	plic_init();
	return 0;
}
/* enable interrupts */
void enable_interrupts(void)
{
    set_csr(mie, MIP_MEIP);
    set_csr(mstatus, MSTATUS_MIE);
}

/*
 * disable interrupts
 */
int disable_interrupts(void)
{
    clear_csr(mie, MIP_MEIP);
    clear_csr(mstatus, MSTATUS_MIE);
	return 0;
}
#else
int interrupt_init(void)
{
	return 0;
}
/* enable interrupts */
void enable_interrupts(void)
{
}

/*
 * disable interrupts
 */
int disable_interrupts(void)
{
	return 0;
}
#endif


uint handle_trap(uint mcause, uint epc)
{

  if (0){
  /* External Machine-Level Interrupt from PLIC */
  }else if ((mcause & MCAUSE_INT) && ((mcause & MCAUSE_CAUSE)  == IRQ_M_EXT))
  {
    /* handle_m_ext_interrupt */
    external_interrupt(0);
  }
  else if ((mcause & MCAUSE_INT) && ((mcause & MCAUSE_CAUSE)  == IRQ_M_TIMER))
  {
   /* handle_m_timer_interrupt */
	  timer_interrupt(0);
  }
  else
  {
	  _exit_trap(1 + mcause);
  }
  return epc;
}

/*Entry Point for PLIC Interrupt Handler*/
__attribute__((weak)) void external_interrupt(struct pt_regs * regs)
{
	_exit_trap(10);
}

__attribute__((weak)) void timer_interrupt(struct pt_regs * regs)
{
	_exit_trap(11);
}
static void _exit_trap(int code)
{
	fprintf(stderr, "*** Progam has exited with code: %d***\n", code);
	while (1);
}

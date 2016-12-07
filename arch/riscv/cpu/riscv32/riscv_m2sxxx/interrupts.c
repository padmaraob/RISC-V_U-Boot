/*
 *
 * Copyright (C) 2016 Microsemi Corporation.
 * Padmarao Begari, Microsemi Corporation <padmarao.begari@microsemi.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/ptrace.h>
#include <asm/system.h>
#include <asm/encoding.h>

#ifdef CONFIG_USE_INTERRUPT

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
 * Return true if GIE is enabled before we disable it.
 */
int disable_interrupts(void)
{
	return 0;
}
#endif

uint handle_trap(uint mcause, uint epc)
{
  if (0){
  // External Machine-Level Interrupt from PLIC
  }else if ((mcause & MCAUSE_INT) && ((mcause & MCAUSE_CAUSE)  == IRQ_M_EXT)) {
   // handle_m_ext_interrupt();
  }    
  else{
   // write(1, "trap\n", 5);
  //  _exit(1 + mcause);
  }
  return epc;
}
/*Entry Point for PLIC Interrupt Handler*/
//void handle_m_ext_interrupt(){

//}


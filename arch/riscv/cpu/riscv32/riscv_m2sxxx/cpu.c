/*
 * Copyright (C) 2016 Microsemi Corporation.
 * Padmarao Begari, Microsemi Corporation <padmarao.begari@microsemi.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* CPU specific code */
#include <common.h>
#include <command.h>
#include <watchdog.h>
#include <asm/cache.h>


/*
 * cleanup_before_linux() is called just before we call linux
 * it prepares the processor for linux
 *
 * we disable interrupt and caches.
 */
int cleanup_before_linux(void)
{
	disable_interrupts();

	return 0;
}

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	disable_interrupts();
	fprintf(stderr, "*** reset failed---(not implemented ***\n");
	/* Code the jump to reset(_start) here  */
	/*__asm__("la t0, _start;  \
		 jr t0");*/
	/* not reached, __reset does not return */
	return 0;

	/*NOTREACHED*/
}

/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Copyright (c) 2017 Microsemi Corporation.
 * Padmarao Begari, Microsemi Corporation <padmarao.begari@microsemi.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _U_BOOT_RISCV_H_
#define _U_BOOT_RISCV_H_	1


/* cpu/.../cpu.c */
int	cleanup_before_linux(void);

/* board/.../... */
int	board_init(void);

/* cpu/.../timer.c */
void reset_timer_masked(void);

#endif	/* _U_BOOT_RISCV_H_ */

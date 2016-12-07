/*
 * Copyright (C) 2016 Microsemi Corporation.
 * Padmarao Begari, Microsemi Corporation <padmarao.begari@microsemi.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>

/* CoreUART register footprint */
typedef struct msc_coretimer {
	uint timer_load;
	uint timer_value;
	uint timer_ctrl;
	uint timer_prescale;
	uint timer_intclr;
	uint timer_ris;
	uint timer_mis;
} msc_coretimer_t;

#define  CORETIMER_ENABLE	0x01

static ulong timestamp;
static ulong lastdec;

int timer_init(void)
{
	msc_coretimer_t *tmr = (msc_coretimer_t *)CONFIG_CORETIMER_BASE;
	unsigned int cr;

	debug("%s()\n", __func__);

	/* disable timer and interrupts */
	writel(0, &tmr->timer_ctrl);


	/* setup timer */
	/* prescale = 83mhz/2 => 41.5Mhz */
	writel(0, &tmr->timer_prescale); 
	writel(TIMER_LOAD_VAL, &tmr->timer_load);
	/* clear interrupts */
	writel(0, &tmr->timer_intclr);


	cr = readl(&tmr->timer_ctrl);
	cr |= CORETIMER_ENABLE;
	writel(cr, &tmr->timer_ctrl);

	/* init the timestamp and lastdec value */
	reset_timer_masked();

	return 0;
}

/*
 * timer without interrupts
 */

/*
 * reset time
 */
void reset_timer_masked(void)
{
	msc_coretimer_t *tmr = (msc_coretimer_t *)CONFIG_CORETIMER_BASE;

	/* capure current decrementer value time */

	lastdec = readl(&tmr->timer_value) /
			(CONFIG_SYS_CLK_FREQ / 4 / CONFIG_SYS_HZ);
	timestamp = 0;		/* start "advancing" time stamp from 0 */

	debug("%s(): lastdec = %lx\n", __func__, lastdec);
}

void reset_timer(void)
{
	debug("%s()\n", __func__);
	reset_timer_masked();
}

/*
 * return timer ticks
 */
ulong get_timer_masked(void)
{
	msc_coretimer_t *tmr = (msc_coretimer_t *)CONFIG_CORETIMER_BASE;

	/* current tick value */

	ulong now = readl(&tmr->timer_value) /
			(CONFIG_SYS_CLK_FREQ / 4 / CONFIG_SYS_HZ);

	debug("%s(): now = %lx, lastdec = %lx\n", __func__, now, lastdec);

	if (lastdec >= now) {
		/*
		 * normal mode (non roll)
		 * move stamp fordward with absoulte diff ticks
		 */
		timestamp += lastdec - now;
	} else {
		/*
		 * we have overflow of the count down timer
		 *
		 * nts = ts + ld + (TLV - now)
		 * ts=old stamp, ld=time that passed before passing through -1
		 * (TLV-now) amount of time after passing though -1
		 * nts = new "advancing time stamp"...it could also roll and
		 * cause problems.
		 */
		timestamp += lastdec + TIMER_LOAD_VAL - now;
	}

	lastdec = now;

	debug("%s() returns %lx\n", __func__, timestamp);

	return timestamp;
}

/*
 * return difference between timer ticks and base
 */
ulong get_timer(ulong base)
{
	debug("%s(%lx)\n", __func__, base);
	return get_timer_masked() - base;
}

void set_timer(ulong t)
{
	debug("%s(%lx)\n", __func__, t);
	timestamp = t;
}

/* delay x useconds AND preserve advance timestamp value */
void __udelay(unsigned long usec)
{
	msc_coretimer_t *tmr = (msc_coretimer_t *)CONFIG_CORETIMER_BASE;


	long tmo = usec * ((CONFIG_SYS_CLK_FREQ / 4) / 1000) / 1000;

	unsigned long now, last = readl(&tmr->timer_value);

	debug("%s(%lu)\n", __func__, usec);
	while (tmo > 0) {
		now = readl(&tmr->timer_value);
		if (now > last) /* count down timer overflow */
			tmo -= TIMER_LOAD_VAL + last - now;
		else
			tmo -= last - now;
		last = now;
	}
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return get_timer(0);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{

	return CONFIG_SYS_CLK_FREQ;
}

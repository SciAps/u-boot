/*
 * (C) Copyright 2008
 * Texas Instruments
 *
 * Richard Woodruff <r-woodruff2@ti.com>
 * Syed Moahmmed Khasim <khasim@ti.com>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 * Alex Zuepke <azu@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <errno.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

static struct gptimer *timer_base = (struct gptimer *)CONFIG_SYS_TIMERBASE;


/*
 * Nothing really to do with interrupts, just starts up a counter.
 */

#define TIMER_CLOCK	(V_SCLK / (2 << CONFIG_SYS_PTV))
#define TIMER_OVERFLOW_VAL	0xffffffff
#define TIMER_LOAD_VAL		0

int timer_init(void)
{
	/* start the counter ticking up, reload value on overflow */
	writel(TIMER_LOAD_VAL, &timer_base->tldr);
	/* enable timer */
	writel((CONFIG_SYS_PTV << 2) | TCLR_PRE | TCLR_AR | TCLR_ST,
		&timer_base->tclr);

	reset_timer_masked();	/* init the timestamp and lastinc value */

	return 0;
}

/*
 * timer without interrupts
 */
void reset_timer(void)
{
	reset_timer_masked();
}

ulong get_timer(ulong base)
{
	return get_timer_masked() - base;
}

void set_timer(ulong t)
{
	gd->tbl = t;
}

/* delay x useconds */
void __udelay(unsigned long usec)
{
	long tmo = usec * (TIMER_CLOCK / 1000) / 1000;
	unsigned long now, last = readl(&timer_base->tcrr);

	while (tmo > 0) {
		now = readl(&timer_base->tcrr);
		if (last > now) /* count up timer overflow */
			tmo -= TIMER_OVERFLOW_VAL - last + now;
		else
			tmo -= now - last;
		last = now;
	}
}

void reset_timer_masked(void)
{
	/* reset time, capture current incrementer value time */
	gd->lastinc = readl(&timer_base->tcrr) / (TIMER_CLOCK / CONFIG_SYS_HZ);
	gd->tbl = 0;		/* start "advancing" time stamp from 0 */
}

ulong get_timer_masked(void)
{
	/* current tick value */
	ulong now = readl(&timer_base->tcrr) / (TIMER_CLOCK / CONFIG_SYS_HZ);

	if (now >= gd->lastinc)	/* normal mode (non roll) */
		/* move stamp fordward with absoulte diff ticks */
		gd->tbl += (now - gd->lastinc);
	else	/* we have rollover of incrementer */
		gd->tbl += ((TIMER_LOAD_VAL / (TIMER_CLOCK / CONFIG_SYS_HZ))
			     - gd->lastinc) + now;
	gd->lastinc = now;
	return gd->tbl;
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
	return CONFIG_SYS_HZ;
}

int init_gpt_timer(u32 timer, u32 value, u32 range)
{
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	struct gptimer *timer_base;
	u32 reg;

	printf("%s: timer %u value %u range %u\n", __FUNCTION__, timer, value, range);
	switch(timer) {
	case 10:
		writel(readl(&prcm_base->clksel_core) | (1 << 6), &prcm_base->clksel_core);
		writel(readl(&prcm_base->fclken1_core) | (1 << 11), &prcm_base->fclken1_core);
		writel(readl(&prcm_base->iclken1_core) | (1 << 11), &prcm_base->iclken1_core);
		timer_base = (struct gptimer *)OMAP34XX_GPT10;
		break;
	default:
		return -ENODEV;
	}

	/* dm_timer_set_load */
	reg = readl(&timer_base->tclr);
	reg |= TCLR_AR;
	writel(reg, &timer_base->tclr);
	writel(0xffff0000, &timer_base->tldr);
	writel(0, &timer_base->ttgr);

	/* dm_set_pwm */
	reg = readl(&timer_base->tclr);
	reg &= ~(TCLR_GPO_CFG | TCLR_SCPWM | TCLR_PT | (0x03 << 10));
	reg |= TCLR_PT;
	reg |= (0x2 << 10); /* OVERFLOW_AND_COMPARE */
	writel(reg, &timer_base->tclr);

	/* dm_set_match */
	reg = readl(&timer_base->tclr);
	reg |= TCLR_CE;
	writel(reg, &timer_base->tclr);
	writel(0xffff0000 | (((value * 0xff) / range) << 8), &timer_base->tmar);

	/* dm_timer_start */
	reg = readl(&timer_base->tclr);
	if (!(reg & TCLR_ST)) {
		reg |= TCLR_ST;
		writel(reg | TCLR_ST, &timer_base->tclr);
	}

	return 0;
}

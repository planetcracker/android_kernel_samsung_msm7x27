/*
 * Author: Alin Panaitiu <alin.p32@gmail.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _LINUX_PERF_CONTROL_H
#define _LINUX_PERF_CONTROL_H

struct clkctl_acpu_speed {
	unsigned int	use_for_scaling;
	unsigned int	a11clk_khz;
	int		pll;
	unsigned int	a11clk_src_sel;
	unsigned int	a11clk_src_div;
	unsigned int	ahbclk_khz;
	unsigned int	ahbclk_div;
	int		vdd;
	unsigned int 	axiclk_khz;
	unsigned long	lpj; /* loops_per_jiffy */
/* Pointers in acpu_freq_tbl[] for max up/down steppings. */
	struct clkctl_acpu_speed *down[3];
	struct clkctl_acpu_speed *up[3];
};

extern int pll2_overclock;
extern struct clkctl_acpu_speed pll0_960_pll1_245_pll2_1200[];

extern int aggressivity;
extern void lowmem_compute(int aggressivity);

#endif


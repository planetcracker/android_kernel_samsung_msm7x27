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

extern int volt_66;
extern int volt_122;
extern int volt_245;
extern int volt_360;
extern int volt_480;
extern int volt_604;
extern int volt_658;
extern int volt_770;
extern int pll2_overclock;

extern void voltage_levels(int freq);

extern int aggressivity;
extern void lowmem_compute(int aggressivity);

#endif


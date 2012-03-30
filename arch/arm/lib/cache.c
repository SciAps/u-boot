/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

/* for now: just dummy functions to satisfy the linker */

#include <common.h>

#ifdef CONFIG_OMAP34XX
/* Cache routine for OMAP34XX (Cortex-A8) */
void invalidate_dcache_range(unsigned long start, unsigned long stop)
{
	unsigned int cache_level_id;		/* cache level ID register */
	unsigned int cache_size_id;	/* cache size ID register */
	unsigned long addr, end_addr;
	unsigned int level;
	unsigned int line_size;
	unsigned int assoc, num_sets;

	/* First get the cache ID register */
	asm("mrc p15, 1, %0, c0, c0, 1 @ get cache level ID" : "=r" (cache_level_id) : : "cc");
	/* Strip off LoU/LoC */
	cache_level_id &= ~0xFF000000;

	/* Loop over the levels until there's no higher order cache */
	for (level = 0; cache_level_id; level+=2) {
		/* Select the level */
		asm volatile("mcr p15, 2, %0, c0, c0, 0 @ Select cache level"
			: : "r" (level) : "cc");

		asm("mrc p15, 1, %0, c0, c0, 0 @ get cache size ID" : "=r" (cache_size_id) : : "cc");

		/* Number of bytes in line */
		line_size = (1 << ((cache_size_id & 0x3) + 2)) * 4;

		/* Calculate number of sets * associativity to
		 * figure if its easier to use MVA vs set/way */
		assoc = ((cache_size_id >> 2) + 1) & 0x3ff;
		num_sets = ((cache_size_id >> 13) + 1) & 0x7fff;

		if (0 && (assoc * num_sets * line_size) > (stop - start)) {
			/* Cheaper to flush/invalidate using set/way */
			;
		} else {
			/* Cheaper to flush/invalidate using MVA */
			addr = start & ~line_size;
			end_addr = (stop + line_size - 1) & ~line_size;

			/* Clean and invalidate each line */
			for (; addr < end_addr; addr += line_size) {
				asm volatile("mcr p15, 0, %0, c7, c14, 1 @ clean/invalidate Dcache" : : "r" (addr) : "cc");
			}
		}

		/* Peel off this cache layer and continue until no more */
		cache_level_id >>= 3;
	}

	/* Switch back to level 0 */
	asm volatile("mcr p15, 2, %0, c0, c0, 0 @ Cache Size SelectID"
		: : "r" (0) : "cc");

	asm volatile("mcr p15, 0, %0, c7, c5, 4	@ flush prefetch buffer" : : "r" (0) : "cc");
#if 0
	/* invalidate the I-cache */
	asm volatile("mcr p15 0, %0, c7, c5, 0 @ I+BTB cache invalidate", : : "r" (0) : "cc");
#endif
}
#endif

void  flush_cache (unsigned long dummy1, unsigned long dummy2)
{
#if defined(CONFIG_OMAP2420) || defined(CONFIG_ARM1136)
	void arm1136_cache_flush(void);

	arm1136_cache_flush();
#endif
#ifdef CONFIG_ARM926EJS
	/* test and clean, page 2-23 of arm926ejs manual */
	asm("0: mrc p15, 0, r15, c7, c10, 3\n\t" "bne 0b\n" : : : "memory");
	/* disable write buffer as well (page 2-22) */
	asm("mcr p15, 0, %0, c7, c10, 4" : : "r" (0));
#endif
#ifdef CONFIG_OMAP34XX
	void v7_flush_cache_all(void);

	v7_flush_cache_all();
#endif
	return;
}

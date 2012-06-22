/*
 * (C) Copyright 2011
 * Logic Product Devleopemnt <www.logicpd.com>
 * Peter Barada <peter.barada@logicpd.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __MTD_PARTS_H__
#define __MTD_PARTS_H__
extern int mtd_get_part_priv(const char *partname, int *idx,
			struct mtd_device **dev, loff_t *off,
			loff_t *size, void **cookie, void **priv, int quiet);

extern void mtd_set_part_priv(void *cookie, void *priv);

extern int mtd_id_parse(const char *id, const char **ret_id,
			u8 *dev_type, u8 *dev_num, int quiet);
extern int mtdparts_init(void);
extern int mtd_part_getmask(struct part_info *part);
#endif

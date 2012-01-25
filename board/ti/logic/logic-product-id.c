/*
 * (C) Copyright 2011
 * Logic Product Development <www.logicpd.com>
 * Peter Barada <peter.barada@logicpd.com>
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

/* Code to extract x-loader perntinent data from product ID chip */

#include <common.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mem.h>

// #include "dm3730logic-ddr.h"
// #include "dm3730logic-product-id.h"

#include "prod-id/interface.h"
#include "prod-id/id-errno.h"
#include "logic-i2c.h"
#include "logic-id-data.h"
#include "logic-at24.h"
#include "logic-gpio.h"

struct id_data id_data;

/* Fetch a byte of data from the ID data on the i2c bus */
unsigned char id_fetch_byte(unsigned char *mem_ptr, int offset, int *oor)
{
	unsigned char val;

	/* If data is off end of known size then complain */
	if (id_data.root_size && (offset >= (id_data.root_offset + id_data.root_size))) {
		id_printf("Attempt to read past end of buffer (offset %u >= size %u)\n", offset, sizeof(id_data_buf));
		*oor = -ID_ERANGE;
		return 0;  /* Force upper layer to recover */
	}

	*oor = ID_EOK;

	if (mem_ptr) {
		val = mem_ptr[offset];
		return val;
	} else {
		if (at24_read(offset, &val, 1) == 0) {
			return val;
		}
	}

	*oor = ID_ENODEV;
	return 0;
}

int id_printf(const char *fmt, ...)
{
	va_list args;
	char printbuffer[256];

	va_start (args, fmt);
	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	vsprintf (printbuffer, fmt, args);
	va_end (args);
	/* Print the string */
	serial_puts (printbuffer);

	return 0;
}

void id_error(const char *fmt, ...)
{
	va_list args;
	char printbuffer[256];

	va_start (args, fmt);
	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	vsprintf (printbuffer, fmt, args);
	va_end (args);
	/* Print the string */
	serial_puts (printbuffer);

}

static int found_id_data;
/* Initialize the product ID data and return 0 if found */
static int product_id_init(void)
{
	int ret;

	memset(&id_data, 0, sizeof(id_data));

	/* id_data.mem_ptr is an address top copy the ID data from the AT24
	 * chip into during startup(since startup codehas to CRC whole data
	 * area).  Once its read there we can access the copy instead of
	 * going back to the AT24 to read the data. */
	id_data.mem_ptr = (void *)SRAM_BASE;

	ret=at24_wakeup();
	if(ret) {
		printf("wakeup_err=%d\n", ret);
	}

	ret = id_startup(&id_data);


	at24_shutdown();

	if (ret != ID_EOK) {
		return -1;
	}

	return 0;
}

int logic_has_new_product_id(void)
{
	if (!found_id_data) {
		if (!product_id_init()) {
			found_id_data = 1;
		}
	}
	return found_id_data;
}


int logic_dump_serialization_info(void)
{
	int ret;
	struct id_cookie cookie;
	int part_number;
	u8 model_name[32];
	u32 model_name_size;
	u8 serial_number[10];
	u32 serial_number_size;

	if (!found_id_data) {
		return -1;
	}

	ret = id_init_cookie(&id_data, &cookie);
	if (ret != ID_EOK) {
		printf("%s:%d ret %d\n", __FUNCTION__, __LINE__, ret);
		return ret;
	}

	/* find /serialization_group from root */
	ret = id_find_dict(&cookie, ID_KEY_serialization_group, IDENUM_DICT);
	if (ret != ID_EOK) {
		printf("%s:%d ret %d\n", __FUNCTION__, __LINE__, ret);
		return ret;
	}

	/* Find serial_number */
	serial_number_size = sizeof(serial_number);
	ret = id_find_string(&cookie, ID_KEY_serial_number, serial_number, &serial_number_size);
	if (ret != ID_EOK) {
		printf("%s:%d ret %d\n", __FUNCTION__, __LINE__, ret);
		return ret;
	}

	/* Reinitialise cookie back to the root */
	ret = id_init_cookie(&id_data, &cookie);
	if (ret != ID_EOK) {
		printf("%s:%d ret %d\n", __FUNCTION__, __LINE__, ret);
		return ret;
	}

	/* find /model_group from root */
	ret = id_find_dict(&cookie, ID_KEY_model_group, IDENUM_DICT);
	if (ret != ID_EOK) {
		printf("%s:%d ret %d\n", __FUNCTION__, __LINE__, ret);
		return ret;
	}

	/* Find part number */
	ret = id_find_number(&cookie, ID_KEY_part_number, &part_number);
	if (ret != ID_EOK) {
		printf("%s:%d ret %d\n", __FUNCTION__, __LINE__, ret);
		return ret;
	}

	/* Find model name */
	model_name_size = sizeof(model_name);
	ret = id_find_string(&cookie, ID_KEY_model_name, model_name, &model_name_size);
	if (ret != ID_EOK) {
		printf("%s:%d ret %d\n", __FUNCTION__, __LINE__, ret);
		return ret;
	}

	printf("Part Number  : %u\n", part_number);
	printf("Model Name   : %.*s\n", model_name_size, model_name);
	printf("Serial Number: %.*s\n", serial_number_size, serial_number);
	return 0;
}

/* Extract GPMC timings for particular CS register */
id_keys_t gpmc_ncs_keys[] = {
	ID_KEY_cs0_group,
	ID_KEY_cs1_group,
	ID_KEY_cs2_group,
	ID_KEY_cs3_group,
	ID_KEY_cs4_group,
	ID_KEY_cs5_group,
	ID_KEY_cs6_group,
};

id_keys_t gpmc_config_reg_keys[] = {
	ID_KEY_config1_reg,
	ID_KEY_config2_reg,
	ID_KEY_config3_reg,
	ID_KEY_config4_reg,
	ID_KEY_config5_reg,
	ID_KEY_config6_reg,
	ID_KEY_config7_reg,
};

int logic_extract_gpmc_timing(int cs, int *config_regs)
{
	int ret;
	struct id_cookie cookie;
	// int gpmc_config_values[ARRAY_SIZE(gpmc_config_reg_keys)];

	if (!found_id_data)
		return -1;

	ret = id_init_cookie(&id_data, &cookie);
	if (ret != ID_EOK) {
		return ret;
	}

	/* find /cpu0_bus_group from root */
	ret = id_find_dict(&cookie, ID_KEY_cpu0_bus_group, IDENUM_DICT);
	if (ret != ID_EOK) {
		return ret;
	}

	/* find /local_bus_group from /cpu0_bus_group */
	ret = id_find_dict(&cookie, ID_KEY_local_bus_group, IDENUM_DICT);
	if (ret != ID_EOK) {
		return ret;
	}

	/* Now look for the particular chip select group */
	ret = id_find_dict(&cookie, gpmc_ncs_keys[cs], IDENUM_DICT);
	if (ret != ID_EOK) {
		return ret;
	}

	/* We have the group, now extract all the config registers */
	ret = id_find_numbers(&cookie, gpmc_config_reg_keys, ARRAY_SIZE(gpmc_config_reg_keys), config_regs);

	return ret;
}

int do_dump_id_data(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
	printf("id_data: mem_ptr %p root_offset %u root_size %u\n",
		id_data.mem_ptr, id_data.root_offset, id_data.root_size);
	return 1;
}

U_BOOT_CMD(
	dump_id_data, 1, 1, do_dump_id_data,
	"dump_id_data - dump product ID data",
	"dump product ID data in human-readable form"
);

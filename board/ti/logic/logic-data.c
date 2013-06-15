/*
 * (C) Copyright 2008-2011
 * Logic Product Development, <www.logicpd.com>
 * Peter Barada <peter.barada@logicpd.com>
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
#include <command.h>
#include <asm/arch/cpu.h>
#include <asm/io.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/clocks.h>
#include <asm/arch/mem.h>
#include <i2c.h>
#include <asm/mach-types.h>
#include "product_id.h"
#include "logic-gpio.h"
#include "logic-i2c.h"
#include "logic-at88.h"
#include "logic-product-id.h"
#include "logic-data.h"

static int header_version = -1;


/*
 * Identify the device
 */
struct device_param {
	char *name;
	unsigned char reset[8];	/* ATR for part */
	unsigned int zones;	/* number of zones */
	unsigned int zonesize;	/* bytes per zone */
};

static const struct device_param answers[] = {
	{
		.name = "AT88SC0104C",
		.reset = {0x3B, 0xB2, 0x11, 0x00, 0x10, 0x80, 0x00, 0x01},
		.zones = 4,
		.zonesize = 32
	},
	{
		.name = "AT88SC0204C",
		.reset = {0x3B, 0xB2, 0x11, 0x00, 0x10, 0x80, 0x00, 0x02},
		.zones = 4,
		.zonesize = 64
	},
	{
		.name = "AT88SC0404C",
		.reset = {0x3B, 0xB2, 0x11, 0x00, 0x10, 0x80, 0x00, 0x04},
		.zones = 4,
		.zonesize = 128
	},
	{
		.name = "AT88SC0808C",
		.reset = {0x3B, 0xB2, 0x11, 0x00, 0x10, 0x80, 0x00, 0x08},
		.zones = 8,
		.zonesize = 128
	},
	{
		.name = "AT88SC1616C",
		.reset = {0x3B, 0xB2, 0x11, 0x00, 0x10, 0x80, 0x00, 0x16},
		.zones = 16,
		.zonesize = 128
	},
	{
		.name = "AT88SC3216C",
		.reset = {0x3B, 0xB3, 0x11, 0x00, 0x00, 0x00, 0x00, 0x32},
		.zones = 16,
		.zonesize = 256
	},
	{
		.name = "AT88SC6416C",
		.reset = {0x3B, 0xB3, 0x11, 0x00, 0x00, 0x00, 0x00, 0x64},
		.zones = 16,
		.zonesize = 512
	},
	{
		.name = "AT88SC12816C",
		.reset = {0x3B, 0xB3, 0x11, 0x00, 0x00, 0x00, 0x01, 0x28},
		.zones = 16,
		.zonesize = 1024
	},
	{
		.name = "AT88SC25616C",
		.reset = {0x3B, 0xB3, 0x11, 0x00, 0x00, 0x00, 0x02, 0x56},
		.zones = 16,
		.zonesize = 2048
	},
};

static const struct device_param *devptr; /* pointer to ID'd device */

#define CMD_SYSTEM_READ	0xB6

static int
identify_device(void)
{
	const struct device_param *p;
	unsigned char cmd[] = { CMD_SYSTEM_READ, 0x00, 0x00, 0x00 };
	unsigned char buf[8];
	int err;
	int i,j;

	err = at88_send_packet(cmd, sizeof(cmd), buf, sizeof(buf));
	if (err)
		return err;

	if (DEBUG_PRODUCTION_DATA)
		printf("%s: ident %02x %02x %02x %02x %02x %02x %02x %02x\n", __FUNCTION__,
			buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);

	for (p=answers,i=0; i<sizeof(answers)/sizeof(answers[0]); ++i,++p) {
		for (j=0; j<8 && (p->reset[j] == buf[j]); ++j)
			;
		if (j==8) {
			devptr = p;

			if (DEBUG_PRODUCTION_DATA)
				printf("%s: device %s zones %u zonesize %u\n", __FUNCTION__,
					devptr->name, devptr->zones, devptr->zonesize);

			return 0;
		}
	}

	if (DEBUG_PRODUCTION_DATA)
		printf("%s: Huh? couldn't ID productID device\n", __FUNCTION__);

	return -1;
}

#define CMD_SYSTEM_WRITE    0xB4

static int
set_user_zone(int zone)
{
	unsigned char cmd[] = { CMD_SYSTEM_WRITE, 0x03, 0x00, 0x00 };

	if (DEBUG_PRODUCTION_DATA)
		printf("%s: zone %d\n", __FUNCTION__, zone);
	cmd[2] = zone;
	return at88_send_packet(cmd, sizeof(cmd), NULL, 0);
}

#define CMD_READ_USER_ZONE  0xB2

static int
read_user_zone(unsigned int startzone, unsigned int offset,unsigned char *buf, int len)
{
	unsigned char cmd[] = { CMD_READ_USER_ZONE, 0x00, 0x00, 0x00 };
	int ret;
	unsigned int endzone;
	unsigned int nbytes, zone_offset;

	if (DEBUG_PRODUCTION_DATA)
		printf("%s: offset %u len %d\n", __FUNCTION__, offset, len);

	/* If zone is non-zero, then we use zone/offset addressing, not
	   offset from start of chip */

	/* abort if we'll go past the end of the device */
	if (startzone) {
		if (offset > devptr->zonesize) {
			printf("%s: offset %u > zonesize %u\n", __FUNCTION__, offset, devptr->zonesize);
			return -1;
		}
		if (startzone > devptr->zones) {
			printf("%s: startzone %u > numzones %u\n", __FUNCTION__, startzone, devptr->zones);
			return -1;
		}
	} else {
		startzone = offset / devptr->zonesize;
	}
	endzone = (offset + (len - 1)) / devptr->zonesize;
	if (endzone > devptr->zones) {
		printf("%s: endzone %u > numzones %u\n", __FUNCTION__, endzone, devptr->zones);
		return -1;
	}

	do {
		/* Set the zone */
		if (set_user_zone(startzone))
			return -1;

		zone_offset = offset % devptr->zonesize;
		nbytes = devptr->zonesize - zone_offset;
		if (nbytes > len)
			nbytes = len;

		cmd[2] = zone_offset;
		cmd[3] = nbytes;
		ret = at88_send_packet(cmd, sizeof(cmd), buf, nbytes);
		if (DEBUG_PRODUCTION_DATA_BUF) {
			char obuf[128];
			int i,j,offset;
			for (i = 0, offset=0; i<len; i+=16) {
				for (j = 0; j<16 && i+j<len; ++j)
					if (!j)
						offset = sprintf(obuf, "%02x", buf[i+j]);
					else
						offset += sprintf(&obuf[offset], " %02x", buf[i+j]);
				printf("%s\n", obuf);
			}
		}

		buf += nbytes;
		len -= nbytes;
		offset += nbytes;
		startzone++;
	} while (len);
	return ret;
}

#define XMK_STR(x)	#x
#define MK_STR(x)	XMK_STR(x)
int production_data_valid;

static struct product_id_data product_id_data;

static int valid_mac_address(unsigned char mac[3])
{
	if (mac[0] == 0xff && mac[1] == 0xff && mac[2] == 0xff)
		return 0;
	if (mac[0] == 0x00 && mac[1] == 0x00 && mac[2] == 0x00)
		return 0;
	return !0;
}

static int valid_full_mac_address(unsigned char mac[6])
{
	return (valid_mac_address(&mac[0]) && valid_mac_address(&mac[3]));
}

int extract_mac_address(struct product_id_data *p, int position, unsigned char mac[6])
{
	unsigned char *m = NULL;
	if (!production_data_valid)
		return -1;

	if (DEBUG_PRODUCTION_DATA)
		printf("%s: position %d\n", __FUNCTION__, position);
	switch(position) {
	case 0:
		if (header_version >= 2) {
			if (valid_full_mac_address(p->d.u_zone0.pz_0r2.full_mac)) {
				memcpy(mac, p->d.u_zone0.pz_0r2.full_mac, 6);
				goto out;
			}
		}
		m = p->d.zone2.pz_2r0.mac0;
		break;
	case 1:
		m = p->d.zone2.pz_2r0.mac1;
		break;
	case 2:
		m = p->d.zone2.pz_2r0.mac2;
		break;
	case 3:
		m = p->d.zone2.pz_2r0.mac3;
		break;
	default:
		return -1;
	}
	if (valid_mac_address(m)) {
		mac[0] = 0x00;
		mac[1] = 0x08;
		mac[2] = 0xee;
		mac[3] = m[0];
		mac[4] = m[1];
		mac[5] = m[2];
	} else {
		return -1;
	}

out:
	if (DEBUG_PRODUCTION_DATA)
		printf("%s:%d valid %d position %d %02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__, __LINE__,
			production_data_valid, position,
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return 0;
}

/*
 * Extract/set an ethernet address.
 * Which is the address in the environment, position is which MAC address
 * in the product ID data
 */
void board_get_nth_enetaddr (unsigned char *enetaddr, int which, int position)
{
	unsigned char mac[6];
	char buf[32];
	char *s = NULL, *e;
	int i;
	char ethbuf[18];
	int ret;

	/* We only handle the first two interfaces (LAN/WiFi)... */
	if (which >= 2)
		return;

	memset(mac, 0, sizeof(mac));
	ret = extract_mac_address(&product_id_data, position, mac);
	if (DEBUG_PRODUCTION_DATA)
		printf("%s: ret %d valid %d which %d position %d %02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__,
			ret, production_data_valid, which, position,
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	memset(enetaddr, '\0', 6);
	if (!production_data_valid ||
		!valid_full_mac_address(mac)) {
		if (DEBUG_PRODUCTION_DATA)
			printf("%s: no valid address\n", __FUNCTION__);
		s = getenv("ethaddr");

#ifdef CONFIG_ETHADDR
		if (!s)
			s = MK_STR(CONFIG_ETHADDR);
#endif

		/* If no ethaddr found in productID or environment, then punt*/
		if (!s)
			return;

		for (i = 0; i < 6; ++i) {
			enetaddr[i] = s ? simple_strtoul (s, &e, 16) : 0;
			if (s)
				s = (*e) ? e + 1 : e;
		}
		goto set_it;
	}

	memcpy(enetaddr, mac, 6);

set_it:
	if (which == 0) {
		sprintf(ethbuf, "%02x:%02x:%02x:%02x:%02x:%02x", enetaddr[0], enetaddr[1], enetaddr[2], enetaddr[3], enetaddr[4], enetaddr[5]);
		sprintf(buf, "ethaddr");
	} else {
		sprintf(ethbuf, "%02x:%02x:%02x:%02x:%02x:%02x", enetaddr[0], enetaddr[1], enetaddr[2], enetaddr[3], enetaddr[4], enetaddr[5]);
		sprintf(buf, "eth%daddr", which);
	}
	if (DEBUG_PRODUCTION_DATA)
		printf("setenv '%s' '%s'\n", __FUNCTION__, ethbuf);

	/* If already set, then don't change it */
	s = getenv(buf);
	if (s) {
		if (strcmp(s, ethbuf) != 0)
			printf("Overriding %s productID value %s with environment value\n", buf, ethbuf);
		return;
	}
	setenv(buf, ethbuf);
}

static int extract_product_id_part_number(struct product_id_data *p, char *buf, int buflen)
{
	int size;

	if (!production_data_valid)
		return -1;

	buf[0] = '\0';
	if (header_version == LOGIC_HEADER_VERSION_0) {
		size = sizeof(p->d.u_zone0.pz_0r0.part_number);
		if (buflen < sizeof(p->d.u_zone0.pz_0r0.part_number))
			size = buflen;
		strncpy(buf, p->d.u_zone0.pz_0r0.part_number, sizeof(p->d.u_zone0.pz_0r0.part_number));
		buf[sizeof(p->d.u_zone0.pz_0r0.part_number)] = '\0';
		return 0;
	}

	if (header_version == LOGIC_HEADER_VERSION_1) {
		size = sizeof(p->d.u_zone0.pz_0r1.part_number);
		if (buflen < sizeof(p->d.u_zone0.pz_0r1.part_number))
			size = buflen;
		strncpy(buf, p->d.u_zone0.pz_0r1.part_number, sizeof(p->d.u_zone0.pz_0r1.part_number));
		buf[sizeof(p->d.u_zone0.pz_0r1.part_number)] = '\0';
		return 0;
	}

	if (p->d.u_zone0.pz_0r0.header_version == LOGIC_HEADER_VERSION_2
		|| p->d.u_zone0.pz_0r0.header_version == LOGIC_HEADER_VERSION_3) {
		size = sizeof(p->d.u_zone0.pz_0r2.part_number);
		if (buflen < sizeof(p->d.u_zone0.pz_0r2.part_number))
			size = buflen;
		strncpy(buf, p->d.u_zone0.pz_0r2.part_number, sizeof(p->d.u_zone0.pz_0r2.part_number));
		buf[sizeof(p->d.u_zone0.pz_0r2.part_number)] = '\0';
		return 0;
	}

	return -1;
}


static int extract_header_version(struct product_id_data *p, int *header_version)
{
	if (p->d.u_zone0.pz_0r0.header_version == LOGIC_HEADER_VERSION_0) {
		*header_version = p->d.u_zone0.pz_0r0.header_version;
		return 0;
	}

	if (p->d.u_zone0.pz_0r1.header_version == LOGIC_HEADER_VERSION_1) {
		*header_version = p->d.u_zone0.pz_0r1.header_version;
		return 0;
	}

	if (p->d.u_zone0.pz_0r2.header_version == LOGIC_HEADER_VERSION_2
		|| p->d.u_zone0.pz_0r2.header_version == LOGIC_HEADER_VERSION_3) {
		*header_version = p->d.u_zone0.pz_0r2.header_version;
		return 0;
	}

	*header_version = -1;
	return -1;

}

static int extract_serial_number(struct product_id_data *p, char *buf, int buflen)
{
	buf[0] = '\0';

	if (!production_data_valid)
		return -1;

	if (header_version == LOGIC_HEADER_VERSION_0) {
		sprintf(buf, "%02d%02d%c%05d", p->d.u_zone0.pz_0r0.sn_week,
			 p->d.u_zone0.pz_0r0.sn_year, p->d.u_zone0.pz_0r0.sn_site,
			 p->d.u_zone0.pz_0r0.sn_cnt);
		return 0;
	}
	if (header_version == LOGIC_HEADER_VERSION_1) {
		sprintf(buf, "%02d%02d%c%05d", p->d.u_zone0.pz_0r1.sn_week,
			 p->d.u_zone0.pz_0r1.sn_year, p->d.u_zone0.pz_0r1.sn_site,
			 p->d.u_zone0.pz_0r1.sn_cnt);
		return 0;
	}
	if (header_version == LOGIC_HEADER_VERSION_2
		|| header_version == LOGIC_HEADER_VERSION_3) {
		sprintf(buf, "%02d%02d%c%05d", p->d.u_zone0.pz_0r2.sn_week,
			 p->d.u_zone0.pz_0r2.sn_year, p->d.u_zone0.pz_0r2.sn_site,
			 p->d.u_zone0.pz_0r2.sn_cnt);
		return 0;
	}
	return -1;
}

static void extract_model_number_revision(struct product_id_data *p, char *buf, int buflen)
{
	int i;

	strncpy(buf, product_id_data.d.zone1.model_number, buflen);
	buf[buflen-1] = '\0';
	if (header_version < LOGIC_HEADER_VERSION_2) {
		i = strlen(buf);
		if (i + 3 < buflen) {
			buf[i] = '-';
			buf[i+1] = product_id_data.d.zone1.model_revision;
			buf[i+2] = '\0';
		}
	}
}

int _fetch_production_data(void)
{
	int err = 0;
	int checksum;
	int i;

	production_data_valid = 0;

	/* Make sure voltage is to productID chip! */
	gpio_i2c_init(50000);

	/* The productID chip wants at least 5 clocks to wake it up... */
	gpio_i2c_config_pin(GPIO_I2C_SCLK, GPIO_I2C_OUTPUT);
	gpio_i2c_set_pin_level(GPIO_I2C_SCLK, 1);
	for (i=0; i<10; ++i) {
		udelay(100);
		gpio_i2c_set_pin_level(GPIO_I2C_SCLK, 0);
		udelay(100);
		gpio_i2c_set_pin_level(GPIO_I2C_SCLK, 1);
	}

	if (identify_device()) {
		printf("failed to identify ID device!\n");
		err = -1;
		goto out;
	}

	if (read_user_zone(0, 0, (unsigned char *)&product_id_data.d.u_zone0, sizeof(product_id_data.d.u_zone0))) {
		printf("failed to read ID device!\n");
		err = -1;
		goto out;
	}

	/* If the header doesn't match, we can't map any of the data */
	if (extract_header_version(&product_id_data, &header_version)) {
		printf("ID device read failed - invalid header version %d!\n", header_version);
		err = -2;
		goto out;
	}

	if (read_user_zone(0, 32, (unsigned char *)&product_id_data.d.zone1, sizeof(product_id_data.d.zone1))) {
		printf("failed reading ID device zone1 data!\n");
		err = -3;
		goto out;
	}

	if (read_user_zone(0, 64, (unsigned char *)&product_id_data.d.zone2, sizeof(product_id_data.d.zone2))) {
		printf("failed reading ID device zone2 data!\n");
		err = -4;
		goto out;
	}

	production_data_valid = 1;
	/* Correct endianess issues */
	product_id_data.d.zone2.pz_2r0.processor_type = le16_to_cpu(product_id_data.d.zone2.pz_2r0.processor_type);

	if (header_version < 2)
		product_id_data.d.zone2.pz_2r0.feature_bits = le32_to_cpu(product_id_data.d.zone2.pz_2r0.feature_bits);

	product_id_data.d.zone2.pz_2r0.platform_bits = le32_to_cpu(product_id_data.d.zone2.pz_2r0.platform_bits);

	/* WiFi config data starts at begining of zone 2.  Don't bother
	   reading it if we know it can't fit in the productID chip */
	if (2 + sizeof(product_id_data.d.wifi_config_data.data) / devptr->zonesize < devptr->zones) {
		if (read_user_zone(2, 0, (unsigned char *)&product_id_data.d.wifi_config_data.data, sizeof(product_id_data.d.wifi_config_data.data))) {
			printf("failed reading wifi_config data from ID device!\n");
		} else
			product_id_data.d.wifi_config_data.valid = 1;
	}

out:

 	/* Restore GPIO_OE registers back to reset state (All input) */
	gpio_i2c_config_pin(GPIO_I2C_SDATA, GPIO_I2C_INPUT);
	gpio_i2c_config_pin(GPIO_I2C_SCLK, GPIO_I2C_INPUT);

	/* Restore pins back to their intended use */
	gpio_i2c_restore_pins();

	/* Calculate a checksum for the data and copy the
	 * production data to the start of SRAM */
	checksum = calculate_checksum(&product_id_data.d,
				sizeof(product_id_data.d));
	product_id_data.checksum = checksum;

	*(struct product_id_data *)(SRAM_BASE) = product_id_data;

	return err;
}

void _dump_production_data(void)
{
	/* DECLARE_GLOBAL_DATA_PTR; */
	char buf[36];
	unsigned char mac[6];
	int ret;
	int i;

	if (!production_data_valid)
		return;

	printf("\nID data ROM  : Gen 1\n");
	
	/* Print out the name, model number, and set MAC addresses */
	extract_product_id_part_number(&product_id_data, buf, sizeof(buf));
	printf("Part Number  : %s\n", buf);

	extract_model_number_revision(&product_id_data, buf, sizeof(buf));
	if (strlen(buf))
	{
		printf("Model Name   : %s\n", buf);
		setenv("logic_model", buf);
	}

	extract_serial_number(&product_id_data, buf, sizeof(buf));
	printf("Serial Number: %s\n", buf);
	setenv("logic_serial", buf);

	ret = extract_mac_address(&product_id_data, 0, mac);
	if (!ret) {
		printf("LAN ethaddr  : %02x:%02x:%02x:%02x:%02x:%02x\n",
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}

	for (i=1; i<4; ++i) {
		if (!extract_mac_address(&product_id_data, i, mac))
			printf("LAN[%d] = %02x:%02x:%02x:%02x:%02x:%02x\n",
				i, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}
	printf("\n");
}

#ifdef CONFIG_OMAP3_LOGIC_USE_NEW_PRODUCT_ID
static int has_new_product_id_data;
#endif

/* Stub functions that call into current AT88 product ID code.
 * Need to add tests for *new* product ID data before looking for older
 * data device/format. */
int fetch_production_data(void)
{
#ifdef CONFIG_OMAP3_LOGIC_USE_NEW_PRODUCT_ID

	/* This makes first pass through the EEPROM so we have to initialize
	 * software i2c before reading and clean up after we are done */

	if (logic_has_new_product_id()) {
		has_new_product_id_data = 1;
		return 0;
	}
#endif
	return _fetch_production_data();
}

void dump_production_data(void)
{
#ifdef CONFIG_OMAP3_LOGIC_USE_NEW_PRODUCT_ID

	if (has_new_product_id_data) {
		logic_dump_serialization_info();
		return;
	}
#endif
	_dump_production_data();

}

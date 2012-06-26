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
#include "logic-i2c.h"

#define AT24_CHIP_ID		0xA0
#define AT24_ACCESS_WRITE	0
#define AT24_ACCESS_READ	1

#define AT24_DEVICE_ADDRESS(rw) (AT24_CHIP_ID | (rw & 1))
#define AT24_SIZE_MASK		0x7FF	/* 2048 byte AT24C16 */
int at24_wakeup(void)
{
	int i;
	gpio_i2c_init(100000);

	/* Need 9 clocks to wake up AT24 chips */
	gpio_i2c_config_pin(GPIO_I2C_SCLK, GPIO_I2C_OUTPUT);
	gpio_i2c_set_pin_level(GPIO_I2C_SCLK, 1);
	for (i=0; i<10; ++i) {
		udelay(100);
		gpio_i2c_set_pin_level(GPIO_I2C_SCLK, 0);
		udelay(100);
		gpio_i2c_set_pin_level(GPIO_I2C_SCLK, 1);
	}

	/* We can't identify AT24 chips... */

	return 0;

}

int at24_shutdown(void)
{
	/* Restore GPIO_OE registers back to reset state (All input) */
	gpio_i2c_config_pin(GPIO_I2C_SDATA, GPIO_I2C_INPUT);
	gpio_i2c_config_pin(GPIO_I2C_SCLK, GPIO_I2C_INPUT);
	/* Restore pins back to their intended use */
	gpio_i2c_restore_pins();

	return 0;
}

/* Send the offset to the AT24 chip */
static int at24_send_offset(u32 offset)
{
	int sleeps = 100;
	int timeout = 1000;
	int err, i;
	u32 byteoffset, sizemask, sentoffset;

	do {
		while (gpio_i2c_busy() && timeout--)
			udelay(100);
		sleeps--;

		gpio_i2c_tx_start();

		err = gpio_i2c_tx_byte(AT24_DEVICE_ADDRESS(AT24_ACCESS_WRITE));

		/* If the eeprom is busy, reset the bus an start over */
		if (err)
			gpio_i2c_tx_stop();

		if (err && !sleeps) {
			/* NACK, return error */
			return err;
		}
	} while (err);

	/* Write the destination offset to the AT24 */
	sizemask = AT24_SIZE_MASK;
	sentoffset = offset & sizemask;

	for (i=0; i<4; ++i) {
		byteoffset = (sentoffset & 0xFF000000) >> 24;
		if (sizemask & 0xFF000000) {
			err = gpio_i2c_tx_byte(byteoffset);
			if (err)
				return err;
		}
		sentoffset <<= 8;
		sizemask <<= 8;
	}
	return 0;
}

/* Read 'size' bytes from the AT88 at offset 'offset'; return 0 if good */
int at24_read(u32 offset, u8 *buf, u32 size)
{
	int rx_mode;
	int err, idx;
	
	err = at24_send_offset(offset);
	if (err)
		return err;
	gpio_i2c_tx_stop();

	gpio_i2c_tx_start();
	err = gpio_i2c_tx_byte(AT24_DEVICE_ADDRESS(AT24_ACCESS_READ));
	if (err)
		return err;

	for (idx = 0; idx < size; ++idx) {
		if (size == 1)
			rx_mode = RX_MODE_ONE_BYTE;
		else if (idx == (size - 1))
			rx_mode = RX_MODE_LAST_BYTE;
		else if (idx == (size - 2))
			rx_mode = RX_MODE_NEXT_TO_LAST_BYTE;
		else if (idx == 0)
			rx_mode = RX_MODE_FIRST_BYTE;
		else
			rx_mode = RX_MODE_MIDDLE_BYTE;

		err = gpio_i2c_rx_byte(&buf[idx], rx_mode);
		if (err)
			printf("%s:%d err idx %d\n", __FUNCTION__, __LINE__, idx);
	}

	gpio_i2c_tx_stop();
	return err;
}

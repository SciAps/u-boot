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
#include "logic-data.h"

int at88_send_packet(uint8_t *data, int len, uint8_t *rxbuf, int rxlen)
{
	int timeout = 1000;
	int retry;
	int rx_mode;
	int tick, err, idx;

	if (DEBUG_PRODUCTION_DATA) {
		char buf[3 * len + 2];
		int i, offset;
		for (offset = 0, i=0; i<len; ++i) {
			if (!i)
				offset = sprintf(buf, "%02x", data[i]);
			else
				offset += sprintf(&buf[offset], " %02x", data[i]);
		}
		printf("%s: %s\n", __FUNCTION__, buf);
	}

	/* Wait for bus */
	while (gpio_i2c_busy() && timeout--)
		udelay(100);

	if (!timeout)
		printf("%s:%d i2c_busy never return zero!\n", __FUNCTION__, __LINE__);

	retry = 0;
	do {
		tick = 0;
		do {
			gpio_i2c_tx_stop();
			gpio_i2c_tx_start();

			/* send cmd */
			err = gpio_i2c_tx_byte(data[0]);
			tick++;
		} while (err && tick < 100);

		if (tick > 3)
			printf("I2C ACK polling tick %d!\n", tick);

		for (idx = 1; idx<len; ++idx) {
			err = gpio_i2c_tx_byte(data[idx]);
			if (err) {
				printf("%s:%d NACK idx %d\n", __FUNCTION__, __LINE__, idx);
			}
		}
	} while (err && (retry++ < 5));

	if (err)
		return err;

	/* Are we expecting a response? */
	if (rxbuf) {
		for (idx = 0; idx < rxlen; ++idx) {
			if (rxlen == 1)
				rx_mode = RX_MODE_ONE_BYTE;
			else if (idx == (rxlen - 1))
				rx_mode = RX_MODE_LAST_BYTE;
			else if (idx == (rxlen - 2))
				rx_mode = RX_MODE_NEXT_TO_LAST_BYTE;
			else if (idx == 0)
				rx_mode = RX_MODE_FIRST_BYTE;
			else
				rx_mode = RX_MODE_MIDDLE_BYTE;

			err = gpio_i2c_rx_byte(&rxbuf[idx], rx_mode);
			if (DEBUG_PRODUCTION_DATA) {
				if (err)
					printf("%s:%d err idx %d\n", __FUNCTION__, __LINE__, idx);
			}
		}
	}

	gpio_i2c_tx_stop();
	return err;
}




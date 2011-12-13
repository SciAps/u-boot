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
#include "product_id.h"
#include "logic-gpio.h"
#include "logic-data.h"

static enum {
	GPIO_I2C_UNINIT,
	GPIO_I2C_STOPPED,
	GPIO_I2C_STARTED,
} gpio_i2c_bus_state;

static int gpio_i2c_clock_high_width, gpio_i2c_clock_low_width;
static int gpio_i2c_coarse_delay;

/*
 * IEN  - Input Enable
 * IDIS - Input Disable
 * PTD  - Pull type Down
 * PTU  - Pull type Up
 * DIS  - Pull type selection is inactive
 * EN   - Pull type selection is active
 * M0   - Mode 0
 * The commented string gives the final mux configuration for that pin
 */

/* Put SCLK/SDA pins connected to the product ID into GPIO mode */
static void gpio_i2c_config_pins(void)
{
	MUX_VAL(CP(I2C3_SCL),       (IEN  | PTU | EN  | M4)); /*I2C3_SCL*/
	MUX_VAL(CP(I2C3_SDA),       (IEN  | PTU | EN  | M4)); /*I2C3_SDA*/
}

/* Restore SCLK/SDA pins connected to the product ID back to I2C mode */

void gpio_i2c_restore_pins(void)
{
	MUX_VAL(CP(I2C3_SCL),       (IEN  | PTU | EN  | M0)); /*I2C3_SCL*/
	MUX_VAL(CP(I2C3_SDA),       (IEN  | PTU | EN  | M0)); /*I2C3_SDA*/
}

#define GPIO_I2C_GPIO_SCLK  184
#define GPIO_I2C_GPIO_SDATA 185

void gpio_i2c_config_pin(GPIO_I2C_PIN pin, GPIO_I2C_DIRECTION dir)
{
	if (dir == GPIO_I2C_INPUT) {
		if (pin == GPIO_I2C_SCLK)
			pin_init_gpio(GPIO_I2C_GPIO_SCLK, 1);
		else
			pin_init_gpio(GPIO_I2C_GPIO_SDATA, 1);
	} else if (dir == GPIO_I2C_OUTPUT) {
		if (pin == GPIO_I2C_SCLK)
			pin_init_gpio(GPIO_I2C_GPIO_SCLK, 0);
		else
			pin_init_gpio(GPIO_I2C_GPIO_SDATA, 0);
	}
}

static int gpio_i2c_read_pin(GPIO_I2C_PIN pin)
{
	if (pin == GPIO_I2C_SCLK)
		return pin_get_gpio_input(GPIO_I2C_GPIO_SCLK);
	else
		return pin_get_gpio_input(GPIO_I2C_GPIO_SDATA);
	return 0;
}

void gpio_i2c_set_pin_level(GPIO_I2C_PIN pin, int level)
{
	uint8_t pin_level;

	if (pin == GPIO_I2C_SCLK) {
		pin_level = pin_get_gpio_input(GPIO_I2C_GPIO_SCLK);
		if (((level == 1) && (pin_level == 0)) ||
		    ((level == 0) && (pin_level == 1)))
			pin_set_gpio_dataout(GPIO_I2C_GPIO_SCLK, level);
	} else if (pin == GPIO_I2C_SDATA) {
		if (level == 0) {
			gpio_i2c_config_pin(pin, GPIO_I2C_OUTPUT);
			pin_set_gpio_dataout(GPIO_I2C_GPIO_SDATA, 0);
		} else {
			gpio_i2c_config_pin(pin, GPIO_I2C_INPUT);
		}
	}
}


void gpio_i2c_init(int bps)
{
	gpio_i2c_bus_state = GPIO_I2C_UNINIT;

	/* Config SCLK, SDATA pins */
	gpio_i2c_config_pin(GPIO_I2C_SCLK, GPIO_I2C_OUTPUT);
	gpio_i2c_set_pin_level(GPIO_I2C_SCLK, 1);

	gpio_i2c_config_pin(GPIO_I2C_SDATA, GPIO_I2C_INPUT);

	gpio_i2c_config_pins();

	/* Assume 1:1 clock duty cycle */
	gpio_i2c_clock_high_width = gpio_i2c_clock_low_width
	  = 1000000 / bps / 2;

	gpio_i2c_coarse_delay = gpio_i2c_clock_high_width;
}

int gpio_i2c_busy(void)
{
	return (gpio_i2c_bus_state == GPIO_I2C_STARTED);
}

void gpio_i2c_tx_stop(void)
{
	if (gpio_i2c_bus_state == GPIO_I2C_STARTED) {
		udelay(gpio_i2c_coarse_delay);

		/* Pull SDATA low */
		gpio_i2c_set_pin_level(GPIO_I2C_SDATA, 0);
		udelay(gpio_i2c_coarse_delay);

		/* Push SCLK high */
		gpio_i2c_set_pin_level(GPIO_I2C_SCLK, 1);
		udelay(gpio_i2c_coarse_delay);

		/* Now drive SDATA high - thats a STOP. */
		gpio_i2c_set_pin_level(GPIO_I2C_SDATA, 1);
		udelay(gpio_i2c_coarse_delay);
		gpio_i2c_bus_state = GPIO_I2C_STOPPED;
	}
}

void gpio_i2c_tx_start(void)
{
	if (gpio_i2c_bus_state == GPIO_I2C_UNINIT
	    || gpio_i2c_bus_state == GPIO_I2C_STOPPED) {
		udelay(gpio_i2c_coarse_delay);
		gpio_i2c_set_pin_level(GPIO_I2C_SDATA, 1);
		udelay(gpio_i2c_coarse_delay);
		gpio_i2c_set_pin_level(GPIO_I2C_SCLK, 1);
		udelay(gpio_i2c_coarse_delay);
		gpio_i2c_set_pin_level(GPIO_I2C_SDATA, 0);
		udelay(gpio_i2c_coarse_delay);
		gpio_i2c_set_pin_level(GPIO_I2C_SCLK, 0);
		udelay(gpio_i2c_coarse_delay);
		gpio_i2c_bus_state = GPIO_I2C_STARTED;
	}
}

/* Return !0 if NACK */
int gpio_i2c_tx_byte(uint8_t data)
{
	uint8_t clock, tx_bit_mask=0x80, nack;

	if (gpio_i2c_bus_state != GPIO_I2C_STARTED)
		printf("%s: Unexpected I2C bus state!\n", __FUNCTION__);

	udelay(gpio_i2c_coarse_delay);

	for (clock=0; clock <= 7; ++clock) {
		if (data & tx_bit_mask)
			gpio_i2c_set_pin_level(GPIO_I2C_SDATA, 1);
		else
			gpio_i2c_set_pin_level(GPIO_I2C_SDATA, 0);
		udelay(gpio_i2c_clock_low_width);
		gpio_i2c_set_pin_level(GPIO_I2C_SCLK, 1);
		udelay(gpio_i2c_clock_high_width);
		gpio_i2c_set_pin_level(GPIO_I2C_SCLK, 0);
		tx_bit_mask >>= 1;
	}
	gpio_i2c_config_pin(GPIO_I2C_SDATA, GPIO_I2C_INPUT);
	udelay(gpio_i2c_clock_low_width);
	gpio_i2c_set_pin_level(GPIO_I2C_SCLK, 1);
	udelay(gpio_i2c_clock_high_width);
	nack = gpio_i2c_read_pin(GPIO_I2C_SDATA);
	gpio_i2c_set_pin_level(GPIO_I2C_SCLK, 0);
	return (nack != 0);
}

int gpio_i2c_rx_byte(uint8_t *data, int rx_mode)
{
	uint8_t clock, data_bit;

	*data = 0;

	gpio_i2c_config_pin(GPIO_I2C_SDATA, GPIO_I2C_INPUT);

	udelay(gpio_i2c_coarse_delay);

	for (clock=0; clock<=8; ++clock) {
		if (clock < 8) {
			gpio_i2c_set_pin_level(GPIO_I2C_SCLK, 1);
			udelay(gpio_i2c_clock_high_width);
			data_bit = gpio_i2c_read_pin(GPIO_I2C_SDATA);
			gpio_i2c_set_pin_level(GPIO_I2C_SCLK, 0);
			*data = (*data << 1) | data_bit;
		} else {
			if ((rx_mode == RX_MODE_LAST_BYTE) || (rx_mode == RX_MODE_ONE_BYTE))
				gpio_i2c_set_pin_level(GPIO_I2C_SDATA, 1);
			else
				gpio_i2c_set_pin_level(GPIO_I2C_SDATA, 0);

			gpio_i2c_set_pin_level(GPIO_I2C_SCLK, 1);
			udelay(gpio_i2c_clock_high_width);
			gpio_i2c_set_pin_level(GPIO_I2C_SCLK, 0);
		}
		udelay(gpio_i2c_clock_low_width);
	}

	return 0;
}


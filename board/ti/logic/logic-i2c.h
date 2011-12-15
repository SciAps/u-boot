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

typedef enum {
	GPIO_I2C_INPUT,
	GPIO_I2C_OUTPUT,
} GPIO_I2C_DIRECTION;

typedef enum {
	RX_MODE_FIRST_BYTE,
	RX_MODE_MIDDLE_BYTE,
	RX_MODE_NEXT_TO_LAST_BYTE,
	RX_MODE_LAST_BYTE,
	RX_MODE_ONE_BYTE
} I2C_RX_MODE;

typedef enum {
	GPIO_I2C_SDATA,
	GPIO_I2C_SCLK,
} GPIO_I2C_PIN;

extern int gpio_i2c_busy(void);
extern void gpio_i2c_tx_stop(void);
extern void gpio_i2c_tx_start(void);
extern int gpio_i2c_tx_byte(uint8_t data);
extern int gpio_i2c_rx_byte(uint8_t *data, int rx_mode);
extern void gpio_i2c_init(int bps);
extern void gpio_i2c_config_pin(GPIO_I2C_PIN pin, GPIO_I2C_DIRECTION dir);
extern void gpio_i2c_set_pin_level(GPIO_I2C_PIN pin, int level);
extern void gpio_i2c_restore_pins(void);

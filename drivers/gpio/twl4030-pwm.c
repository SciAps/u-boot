#include <common.h>
#include <twl4030.h>

/* INTRB register offsets (TWL4030_CHIP_INTBR) */
#define TWL_INTBR_PMBR1	0x92
#define TWL_INTBR_GPBR1	0x91

/*PWM0 register offsets (TWL4030_CHIP_PWM0) */
#define TWL_LED_PWMON	0xf8
#define TWL_LED_PWMOFF	0xf9

#if 0
#define PWM_PRINT(fmt, args...) printf(fmt, ## args)
#else
#define PWM_PRINT(fmt, args...)
#endif

int twl4030_set_pwm0(int level, int max_brightness)
{
	u8 mux_pwm, enb_pwm;
	unsigned char c;
	int status;

	PWM_PRINT("%s: level %d\n", __FUNCTION__, level);
	if (level > max_brightness)
		return -1;

	twl4030_i2c_read_u8(TWL4030_CHIP_INTBR, &mux_pwm, TWL_INTBR_PMBR1);
	twl4030_i2c_read_u8(TWL4030_CHIP_INTBR, &enb_pwm, TWL_INTBR_GPBR1);

	PWM_PRINT("%s: enb_pwm %02x mux_pwm %02x\n", __FUNCTION__, enb_pwm, mux_pwm);

	if (level == 0) {
		/* disable pwm0 output and clock */
		enb_pwm &= ~0x05;
		/* change pwm0 pin to gpio pin */
		mux_pwm &= ~0x0c;

		PWM_PRINT("%s: disable enb_pwm %02x mux_pwm %02x\n", __FUNCTION__, enb_pwm, mux_pwm);

		status = twl4030_i2c_write_u8(TWL4030_CHIP_INTBR,
					enb_pwm, TWL_INTBR_GPBR1);
		if (status) {
			printf("%s:%d status %d\n", __FUNCTION__, __LINE__, status);
			return -2;
		}
		status = twl4030_i2c_write_u8(TWL4030_CHIP_INTBR,
					mux_pwm, TWL_INTBR_PMBR1);
		if (status) {
			printf("%s:%d status %d\n", __FUNCTION__, __LINE__, status);
			return -3;
		}
#if 0
		PWM_PRINT("%s: turn off GPIO_%d as backlight!\n", __FUNCTION__, omap3logic_dss_lcd_data.lcd_gpio_backlight);
		/* Turn off the backlight! */
		gpio_set_value(omap3logic_dss_lcd_data.lcd_gpio_backlight, 0);
#endif
		return 0;
	}

	if (((enb_pwm & 0x5) != 0x5) || ((mux_pwm & 0x0c) != 0x4)) {
		/* change gpio pin to pwm0 pin */
		mux_pwm = (mux_pwm & ~0xc) | 0x04;
		/* enable pwm0 output and clock*/
		enb_pwm = (enb_pwm & ~0x5) | 0x05;

		PWM_PRINT("%s: enable enb_pwm %02x mux_pwm %02x\n", __FUNCTION__, enb_pwm, mux_pwm);
		status = twl4030_i2c_write_u8(TWL4030_CHIP_INTBR,
					mux_pwm, TWL_INTBR_PMBR1);
		if (status) {
			printf("%s:%d status %d\n", __FUNCTION__, __LINE__, status);
			return -4;
		}
		status = twl4030_i2c_write_u8(TWL4030_CHIP_INTBR,
					enb_pwm, TWL_INTBR_GPBR1);
		if (status) {
			printf("%s:%d status %d\n", __FUNCTION__, __LINE__, status);
			return -5;
		}
#if 0
		PWM_PRINT("%s: turn on GPIO_%d as backlight!\n", __FUNCTION__, omap3logic_dss_lcd_data.lcd_gpio_backlight);
		/* Turn on the backlight! */
		gpio_set_value(omap3logic_dss_lcd_data.lcd_gpio_backlight, 1);
#endif
	}

	/* 255 -> 1, 1 -> 126 */
	c = (max_brightness * 126 + (1 - 126) * level) / (max_brightness - 1);

	PWM_PRINT("%s: c %d (%d%% on)\n", __FUNCTION__, c, (((max_brightness+1)-c) * 100)/(max_brightness+1));
	status = twl4030_i2c_write_u8(TWL4030_CHIP_PWM0, 0x7F, TWL_LED_PWMOFF);
	if (status) {
		PWM_PRINT("%s:%d status %d\n", __FUNCTION__, __LINE__, status);
		return -6;
	}
	status = twl4030_i2c_write_u8(TWL4030_CHIP_PWM0, c, TWL_LED_PWMON);
	if (status) {
		PWM_PRINT("%s:%d status %d\n", __FUNCTION__, __LINE__, status);
		return -7;
	}
	return 0;
}

void twl4030_dump_pwm0(void)
{
	int result;
	u8 mux_pwm, enb_pwm;
	u8 off_period, on_period;

	result = twl4030_i2c_read_u8(TWL4030_CHIP_INTBR, &mux_pwm,
				TWL_INTBR_PMBR1);
	result |= twl4030_i2c_read_u8(TWL4030_CHIP_INTBR, &enb_pwm,
				TWL_INTBR_GPBR1);

	result |= twl4030_i2c_read_u8(TWL4030_CHIP_PWM0, &off_period,
				TWL_LED_PWMOFF);
	result |=twl4030_i2c_read_u8(TWL4030_CHIP_PWM0, &on_period,
				TWL_LED_PWMON);

	if (result) {
		printf("%s: failed to read TWL regitsters; result %d\n", __FUNCTION__, result);
		return;
	}
	printf("%s: mux_pwm %02x enb_pwm %02x off_period %02x on_period %02x\n",
		__FUNCTION__, mux_pwm, enb_pwm, off_period, on_period);
}

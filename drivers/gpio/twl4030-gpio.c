#include <common.h>
#include <twl4030.h>

#define BIT(x) (1 << (x))

/* GPIO_CTRL Fields */
#define MASK_GPIO_CTRL_GPIO_ON		BIT(2)

/* store usage of each GPIO. - each bit represents one GPIO */
static unsigned int gpio_usage_count;

static inline int gpio_twl4030_write(u8 address, u8 data)
{
	return twl4030_i2c_write_u8(TWL4030_CHIP_GPIO, data, address);
}

static inline int gpio_twl4030_read(u8 address)
{
	u8 data;
	int ret = 0;

	ret = twl4030_i2c_read_u8(TWL4030_CHIP_GPIO, &data, address);
	return (ret < 0) ? ret : data;
}

int twl4030_set_gpio_direction(unsigned int gpio, unsigned int is_input)
{
	u8 d_bnk = gpio >> 3;
	u8 d_msk = BIT(gpio & 0x7);
	u8 reg = 0;
	u8 base = REG_GPIODATADIR1 + d_bnk;
	int ret = 0;

	ret = gpio_twl4030_read(base);
	if (ret >= 0) {
		if (is_input)
			reg = ret & ~d_msk;
		else
			reg = ret | d_msk;

		ret = gpio_twl4030_write(base, reg);
	}

	return ret;
}

int twl4030_set_gpio_dataout(unsigned int gpio, unsigned int enable)
{
	u8 d_bnk = gpio >> 3;
	u8 d_msk = BIT(gpio & 0x7);
	u8 base = 0;

	if (enable)
		base = REG_SETGPIODATAOUT1 + d_bnk;
	else
		base = REG_CLEARGPIODATAOUT1 + d_bnk;

	return gpio_twl4030_write(base, d_msk);
}

int twl4030_get_gpio_datain(unsigned int gpio)
{
	u8 d_bnk = gpio >> 3;
	u8 d_off = gpio & 0x7;
	u8 base = 0;
	int ret = 0;

	if (unlikely((gpio >= TWL4030_GPIO_MAX)
		|| !(gpio_usage_count & BIT(gpio))))
		return -1;

	base = REG_GPIODATAIN1 + d_bnk;
	ret = gpio_twl4030_read(base);
	if (ret > 0)
		ret = (ret >> d_off) & 0x1;

	return ret;
}

int twl4030_request_gpio(unsigned int gpio)
{
	int status = 0;

	/* on first use, turn GPIO module "on" */
	if (!gpio_usage_count) {
		// struct twl4030_gpio_platform_data *pdata;
		u8 value = MASK_GPIO_CTRL_GPIO_ON;

		status = gpio_twl4030_write(REG_GPIO_CTRL, value);
	}

	if (!status)
		gpio_usage_count |= (0x1 << gpio);

	return status;
}

void twl4030_free_gpio(unsigned int gpio)
{
	gpio_usage_count &= ~BIT(gpio);

	/* on last use, switch off GPIO module */
	if (!gpio_usage_count)
		gpio_twl4030_write(REG_GPIO_CTRL, 0x0);
}

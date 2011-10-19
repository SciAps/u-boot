#include <common.h>
#include <asm/io.h>
#include <asm/arch/gpio.h>
#include <asm/arch/dss.h>
#include <twl4030.h>
#include <lcd.h>
#include "splash-480x272.h"
#include "logic-proto.h"

/* LCD-required members */
int lcd_line_length;  /* initialized in lcd.c */
int lcd_color_fg = 0xFFFF;
int lcd_color_bg = 0x0000;
void *lcd_base;                  /* initialized in lcd.c */
void *lcd_console_address;       /* where is this initialized? */
short console_col = 0;
short console_row = 0;
#if 1
vidinfo_t panel_info; /* Filled in by find_screen */
#else
vidinfo_t panel_info = {
	.vl_col = 480,
	.vl_row = 272,
	.vl_bpix = LCD_COLOR16,
};
#endif
void lcd_setcolreg(ushort regno, ushort red, ushort green, ushort blue) {}

struct logic_screen {
	char * name;
	u32 width;
	u32 height;
	u32 bpp;
	u32 bpix;
	struct panel_config cfg;
};

static struct logic_screen *default_screen;

ulong calc_fbsize(void) {
	if (!default_screen)
		return 0;

	return (default_screen->width * default_screen->height * (default_screen->bpp/8));
}


struct logic_screen logic_screens[] = {
	{
		/* Timing for the 4.3" display */
		.name	= "15",
		.width	= 480,
		.height	= 272,
		.bpp	= 16,
		.bpix	= LCD_COLOR16,
		.cfg = {
			.timing_h	= 0x00100229, /* DISPC_TIMING_H */
			.timing_v	= 0x0030020a, /* DISPC_TIMING_V */
			.pol_freq	= 0x0003b000, /* DISPC_POL_FREQ */
			.divisor	= 0x0001000b, /* DISPC_DIVISOR */
			.lcd_size	= 0x010f01df, /* DISPC_SIZE_LCD */
			.panel_type	= 0x01, /* Active Matrix TFT */
			.data_lines	= 0x01, /* 1=RGB16, 2=RGB18, 3=RGB24 */
			.load_mode	= 0x02, /* Frame Mode */
			.panel_color	= 0x00000000, /* black */
		}
	},
	{
		/* Timing for the 3.6" display */
		.name	= "3",
		.width	= 320,
		.height	= 240,
		.bpp	= 16,
		.bpix	= LCD_COLOR16,
		.cfg = {
			.timing_h	= 0x01401414, /* DISPC_TIMING_H */
			.timing_v	= 0x00300302, /* DISPC_TIMING_V */
			.pol_freq	= 0x00000000, /* DISPC_POL_FREQ */
			.divisor	= 0x00010004, /* DISPC_DIVISOR */
			.lcd_size	= 0x00ef013f, /* DISPC_SIZE_LCD */
			.panel_type	= 0x01, /* Active Matrix TFT */
			.data_lines	= 0x01, /* 1=RGB16, 2=RGB18, 3=RGB24 */
			.load_mode	= 0x02, /* Frame Mode */
			.panel_color	= 0x00000000, /* black */
		}
	},
};

static struct logic_screen *find_screen(void)
{
	char *screen;
	int i;

	screen = getenv("display");
	if (!screen) {
		printf("No 'display' variable found in environment; suppress splashimage\n");
		return NULL;
	}

	for (i=0; i<ARRAY_SIZE(logic_screens); ++i)
		if (!strcmp(screen, logic_screens[i].name))
			break;

	if (i < ARRAY_SIZE(logic_screens)) {
		printf("Found '%s' display screen\n", screen);
		default_screen = &logic_screens[i];
		panel_info.vl_col = default_screen->width;
		panel_info.vl_row = default_screen->height;
		panel_info.vl_bpix = default_screen->bpix;
		lcd_line_length = (panel_info.vl_col * NBITS (panel_info.vl_bpix)) / 8;
		printf("%s:%d lcd_line_length %d\n", __FUNCTION__, __LINE__, lcd_line_length);
		return default_screen;
	}
	return NULL;
}
#if 0
/*
 * Timings for LCD Display
 */
static const struct panel_config lcd_cfg = {
	/* Timing for the 4.3" display */
	.timing_h	= 0x00100229, /* DISPC_TIMING_H */
	.timing_v	= 0x0030020a, /* DISPC_TIMING_V */
	.pol_freq	= 0x0003b000, /* DISPC_POL_FREQ */
	.divisor	= 0x0001000b, /* DISPC_DIVISOR */
	.lcd_size	= 0x010f01df, /* DISPC_SIZE_LCD */
	.panel_type	= 0x01, /* Active Matrix TFT */
	.data_lines	= 0x01, /* 1=RGB16, 2=RGB18, 3=RGB24 */
	.load_mode	= 0x02, /* Frame Mode */
	.panel_color	= 0x00000000, /* black */
};
#endif

/* Return size of largest framebuffer (so system can reserve memory on start) */
ulong board_lcd_setmem(ulong addr)
{
	return 1280*720*3;	/* 720p at 24bpp */
}

void touchup_display_env(void)
{
	// enable the splash screen
	char splash_bmp_gz_str[12];
	sprintf(splash_bmp_gz_str, "0x%08X", (unsigned int)splash_bmp_gz);
	setenv("splashimage", splash_bmp_gz_str);
}

void lcd_ctrl_init(void *lcdbase)
{
	struct dispc_regs *dispc = (struct dispc_regs *) OMAP3_DISPC_BASE;
	struct logic_screen *screen;
	struct panel_config *panel;

	printf("%s: lcdbase %p\n", __FUNCTION__, lcdbase);

	screen = find_screen();
	if (!screen)
		return;
	
	/* configure DSS for single graphics layer */
	panel = &screen->cfg;
	omap3_dss_panel_config(panel);

	writel((ulong)lcdbase, &dispc->gfx_ba0); /* frame buffer address */
	writel((ulong)lcdbase, &dispc->gfx_ba1); /* frame buffer address */
	writel(panel->lcd_size, &dispc->gfx_size); /* size - same as LCD */
#if 1
	writel((1<<0)|(6<<1), &dispc->gfx_attributes); /* 6=RGB16,8=RGB24 */
#else
	writel((1<<0)|(6<<1)|(1<<5), &dispc->gfx_attributes); /* 6=RGB16,8=RGB24 */
#endif

	touchup_display_env();
}

void lcd_enable(void)
{
#if 0
	int status;
#endif
	int mSec;

	lcd_is_enabled = 0; /* keep console messages on the serial port */

	omap3_dss_enable();

	/* Delay 300mS to allow 4.3" panel to initialize */
	for (mSec=0; mSec<300; ++mSec)
		udelay(1000);

	/*
	 * panel_enable = 155
	 * backlight 154 (torpedo); 8 (som)
	 */

	/* Kill SOM LCD_BACKLIGHT_PWR */
	if (!omap_request_gpio(8)) {
		omap_set_gpio_direction(8, 0);
		omap_set_gpio_dataout(8, 0);
	}
	 
	/* turn on LCD_PANEL_PWR */
	if (!omap_request_gpio(155)) {
		omap_set_gpio_direction(155, 0);
		omap_set_gpio_dataout(155, 1);
	} else
		printf("%s:%d fail!\n", __FUNCTION__, __LINE__);


#if 1
#ifdef CONFIG_TWL4030_PWM
	twl4030_set_pwm0(70, 100); /* 70% backlight brighntess */
#else
	/* SOM PWM0 output is GPIO.6 on TWL4030... */
	if (!twl4030_request_gpio(6)) {
		status = twl4030_set_gpio_direction(6, 0);
		printf("%s:%d status %d\n", __FUNCTION__, __LINE__, status);
		status = twl4030_set_gpio_dataout(6, 1);
		printf("%s:%d status %d\n", __FUNCTION__, __LINE__, status);
	} else
		printf("%s:%d - failed to get twl4030_gpio!\n", __FUNCTION__, __LINE__);
#endif
#else
	/* set LCD_PWM0 to full brightness */
	if (!omap_request_gpio(56)) {
		omap_set_gpio_direction(56, 0);
		omap_set_gpio_dataout(56, 1);
	} else
		printf("%s:%d fail!\n", __FUNCTION__, __LINE__);

#endif

	/* Sleep 300mS to allow panel to stabilize */
	for (mSec=0; mSec < 300; ++mSec)
		udelay(1000);

#if 1
	/* turn on LCD_BACKLIGHT_PWR SOM LV */
	if (!omap_request_gpio(8)) {
		omap_set_gpio_direction(8, 0);
		omap_set_gpio_dataout(8, 1);
	} else
		printf("%s:%d fail!\n", __FUNCTION__, __LINE__);
#else
	/* turn on LCD_BACKLIGHT_PWR Torpedo */
	if (!omap_request_gpio(154)) {
		omap_set_gpio_direction(154, 0);
		omap_set_gpio_dataout(154, 1);
	} else
		printf("%s:%d fail!\n", __FUNCTION__, __LINE__);

#endif

}

int do_backlight(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	ulong level;
#if 1
	level = simple_strtoul(argv[1], NULL, 10);
	twl4030_set_pwm0(level, 100); /* Adjust PWM */
#else
	printf("%s: don't know how to handle Torpedo!\n", __FUNCTION__);
#endif
	return 0;
}

U_BOOT_CMD(backlight, 2, 1, do_backlight,
	"backlight - change backlight level",
	"<level>"
);

int do_dump_pwm0(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	twl4030_dump_pwm0();
	return 0;
}

U_BOOT_CMD(dump_pwm0, 1, 1, do_dump_pwm0,
	"dump_pwm0 - dump TWL PWM registers",
	""
);

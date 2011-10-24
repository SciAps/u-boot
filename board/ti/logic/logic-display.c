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

struct omap_video_timings {
	/* Unit: pixels */
	u16 x_res;
	/* Unit: pixels */
	u16 y_res;
	/* Unit: KHz */
	u32 pixel_clock;
	/* Unit: pixel clocks */
	u16 hsw;	/* Horizontal synchronization pulse width */
	/* Unit: pixel clocks */
	u16 hfp;	/* Horizontal front porch */
	/* Unit: pixel clocks */
	u16 hbp;	/* Horizontal back porch */
	/* Unit: line clocks */
	u16 vsw;	/* Vertical synchronization pulse width */
	/* Unit: line clocks */
	u16 vfp;	/* Vertical front porch */
	/* Unit: line clocks */
	u16 vbp;	/* Vertical back porch */
};

struct logic_panel {
	char				*name;
	int				config;
	int				acb;
	char				data_lines;
	struct omap_video_timings	timing;
};


static struct logic_panel default_panel;

ulong calc_fbsize(void) {
	ulong size;
	if (!default_panel.timing.x_res || !default_panel.timing.y_res)
		return 0;

	size = default_panel.timing.x_res * default_panel.timing.y_res;
	size *= (default_panel.data_lines / 8);

	return size;
}


#if 1
struct logic_panel logic_panels[] = {
	{
		.name	= "15",
		.config = OMAP_DSS_LCD_TFT | OMAP_DSS_LCD_IVS
		| OMAP_DSS_LCD_ONOFF | OMAP_DSS_LCD_RF | OMAP_DSS_LCD_IEO
				| OMAP_DSS_LCD_IHS,
		.acb	= 0x28,
		.data_lines = 16,
		.timing = {
			/* 480 x 272, LQ043T1DG01 */
			.x_res		= 480,
			.y_res		= 272,
			.pixel_clock	= 9000,
			.hfp		= 3,
			.hsw		= 42,
			.hbp		= 2,
			.vfp		= 4,
			.vsw		= 11,
			.vbp		= 3,
		},
	},
	{
		.name	= "3",
		.config	= OMAP_DSS_LCD_TFT,
		.acb	= 0x28,
		.timing = {
			/* 320 x 240, LQ036Q1DA01 */
			.x_res		= 320,
			.y_res		= 240,
			.pixel_clock	= 24500,
			.hfp		= 20,
			.hsw		= 20,
			.hbp		= 20,
			.vfp		= 3,
			.vsw		= 3,
			.vbp		= 4,
		},
	},
};

#else
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
#endif

static struct logic_panel *find_panel(void)
{
	char *panel;
	int i;

	panel = getenv("display");
	if (!panel) {
		printf("No 'display' variable found in environment; suppress splashimage\n");
		return NULL;
	}

	for (i=0; i<ARRAY_SIZE(logic_panels); ++i)
		if (!strcmp(panel, logic_panels[i].name))
			break;

	if (i < ARRAY_SIZE(logic_panels)) {
		printf("Found '%s' display panel\n", panel);
		default_panel = logic_panels[i];
		panel_info.vl_col = default_panel.timing.x_res;
		panel_info.vl_row = default_panel.timing.y_res;
		if (default_panel.data_lines == 16)
			panel_info.vl_bpix = LCD_COLOR16;
		else
			panel_info.vl_bpix = LCD_COLOR24;
		lcd_line_length = (panel_info.vl_col * NBITS (panel_info.vl_bpix)) / 8;
#if 0
		printf("%s:%d lcd_line_length %d\n", __FUNCTION__, __LINE__, lcd_line_length);
#endif
		return &default_panel;
	}
	printf("display='%s' does not describe a valid screen!\n", panel);
	return NULL;
}

/* Return size of largest framebuffer (so system can reserve memory on start) */
ulong board_lcd_setmem(ulong addr)
{
	return 1280*720*3;	/* 720p at 24bpp */
}

void touchup_display_env(void)
{
	// enable the splash screen
	char splash_bmp_gz_str[12];

	printf("%s\n", __FUNCTION__);
	sprintf(splash_bmp_gz_str, "0x%08X", (unsigned int)splash_bmp_gz);
	setenv("splashimage", splash_bmp_gz_str);
}

void lcd_ctrl_init(void *lcdbase)
{
	struct dispc_regs *dispc = (struct dispc_regs *) OMAP3_DISPC_BASE;
	struct logic_panel *panel;
	struct panel_config dss_panel;
	int ret;

	memset(&panel_info, 0, sizeof(panel_info));
	panel = find_panel();
	if (!panel)
		return;

	/* Convert from timings into panel_config structure */
	dss_panel.panel_color = 0x0; /* black */
	dss_panel.load_mode = 0x2; /* Frame Mode */
	dss_panel.panel_type = 1; /* Active TFT */

	dss_panel.timing_h = panel->timing.hsw - 1;
	dss_panel.timing_h |= ((panel->timing.hfp - 1) << 8);
	dss_panel.timing_h |= ((panel->timing.hbp - 1) << 20);
	dss_panel.timing_v = panel->timing.vsw - 1;
	dss_panel.timing_v |= ((panel->timing.vfp - 1) << 8);
	dss_panel.timing_v |= ((panel->timing.vbp - 1)  << 16);
	dss_panel.pol_freq = panel->acb;
	dss_panel.pol_freq |= ((panel->config & 0x3f) << 12);
	dss_panel.lcd_size = panel->timing.x_res - 1;
	dss_panel.lcd_size |= (panel->timing.y_res - 1) << 16;
	if (panel->data_lines == 16)
		dss_panel.data_lines = 1;
	else if (panel->data_lines == 24)
		dss_panel.data_lines = 2;
	else {
		printf("%s: Invalid data_lines!\n", __FUNCTION__);
		memset(&panel_info, 0, sizeof(panel_info));
		return;
	}

	dss_panel.pixel_clock = panel->timing.pixel_clock;

	/* configure DSS for single graphics layer */
	omap3_dss_panel_config(&dss_panel);

	writel((ulong)lcdbase, &dispc->gfx_ba0); /* frame buffer address */
	writel((ulong)lcdbase, &dispc->gfx_ba1); /* frame buffer address */
	writel(dss_panel.lcd_size, &dispc->gfx_size); /* size - same as LCD */
#if 1
	writel((1<<0)|(6<<1), &dispc->gfx_attributes); /* 6=RGB16,8=RGB24 */
#else
	writel((1<<0)|(6<<1)|(1<<5), &dispc->gfx_attributes); /* 6=RGB16,8=RGB24 */
#endif

#if 0
	touchup_display_env();
#endif
}

void lcd_enable(void)
{
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

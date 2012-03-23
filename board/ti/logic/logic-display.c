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
#include <asm/io.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mux.h>
#include <asm/arch/dss.h>
#include <asm/arch/timer.h>
#include <twl4030.h>
#include <lcd.h>
#include "splash-332x57.h"
#include "logic-proto.h"

DECLARE_GLOBAL_DATA_PTR;

/* LCD-required members */
int lcd_line_length;  /* initialized in lcd.c */
int lcd_color_fg;
int lcd_color_bg;
void *lcd_base;                  /* initialized in lcd.c */
void *lcd_console_address;       /* where is this initialized? */
short console_col = 0;
short console_row = 0;

vidinfo_t panel_info; /* Filled in by find_screen */

void lcd_setcolreg(ushort regno, ushort red, ushort green, ushort blue)
{
	/* Empty */
}

void lcd_initcolregs(void)
{
	/* Empty */
}

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

static struct omap_custom_lcd_fields {
	char *field;
	char *format;
	void *ptr;
	int len;
} omap_custom_lcd_fields[] = {
	{ "xres", "%u", &default_panel.timing.x_res , 2},
	{ "yres", "%u", &default_panel.timing.y_res , 2},
	{ "left margin", "%u", &default_panel.timing.hbp, 2 },
	{ "right margin", "%u", &default_panel.timing.hfp, 2 },
	{ "top margin", "%u", &default_panel.timing.vbp, 2 },
	{ "bottom margin", "%u", &default_panel.timing.vfp, 2 },
	{ "hsync length", "%u", &default_panel.timing.hsw, 2 },
	{ "vsync length", "%u", &default_panel.timing.vsw, 2 },
	{ "pixclock", "%u", &default_panel.timing.pixel_clock, 4 },
	{ "config", "%u", &default_panel.config, 4 },
	{ "data_lines", "%u", &default_panel.data_lines, 1 },
};

ulong calc_fbsize(void) {
	ulong size;
	if (!default_panel.timing.x_res || !default_panel.timing.y_res)
		return 0;

	size = default_panel.timing.x_res * default_panel.timing.y_res;
	size *= (default_panel.data_lines / 8);

	return size;
}


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
		.data_lines = 16,
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
	{
		.name	= "7",
		.config	= OMAP_DSS_LCD_TFT,
		.acb	= 0x28,
		.data_lines = 16,
		.timing = {
			/* 640 x 480, LQ10D368 */
			.x_res		= 640,
			.y_res		= 480,
			.pixel_clock	= 27000,
			.hfp		= 24,
			.hsw		= 48,
			.hbp		= 135,
			.vfp		= 34,
			.vsw		= 1,
			.vbp		= 34,
		},
	},
	{
		.name	= "5",
		.config	= OMAP_DSS_LCD_TFT,
		.acb	= 0x28,
		.data_lines = 16,
		.timing = {
			/* 640 x 240, LQ036Q1DA01 */
			.x_res		= 640,
			.y_res		= 480,
			.pixel_clock	= 27000,
			.hfp		= 24,
			.hsw		= 48,
			.hbp		= 135,
			.vfp		= 34,
			.vsw		= 1,
			.vbp		= 34,
		},
	},
	{
		.name	= "2",
		.config	= OMAP_DSS_LCD_TFT,
		.acb	= 0x28,
		.data_lines = 16,
		.timing = {
			/* 800 x 600, LQ121S1DG31 */
			.x_res		= 800,
			.y_res		= 600,
			.pixel_clock	= 42000,
			.hfp		= 120,
			.hsw		= 5,
			.hbp		= 88-4-2,
			.vfp		= 100,
			.vsw		= 4,
			.vbp		= 21-1,
		},
	},
	{
		.name	= "vga",
		.config	= OMAP_DSS_LCD_TFT,
		.acb	= 0x28,
		.data_lines = 16,
		.timing = {
			/* 640 x 480, VGA on DVI */
			.x_res		= 640,
			.y_res		= 480,
			.pixel_clock	= 24685,
			.hfp		= 16,
			.hsw		= 96,
			.hbp		= 48,
			.vfp		= 10,
			.vsw		= 2,
			.vbp		= 33,
		},
	},
	{
		.name	= "svga",
		.config	= OMAP_DSS_LCD_TFT,
		.acb	= 0x28,
		.data_lines = 16,
		.timing = {
			/* 800 x 600, SVGA on DVI */
			.x_res		= 800,
			.y_res		= 600,
			.pixel_clock	= 42000,
			.hfp		= 120,
			.hsw		= 5,
			.hbp		= 88-4-2,
			.vfp		= 100,
			.vsw		= 4,
			.vbp		= 21-1,
		},
	},
	{
		.name	= "xga",
		.config	= OMAP_DSS_LCD_TFT,
		.acb	= 0x28,
		.data_lines = 16,
		.timing = {
			/* 1024 x 769, XGA on DVI */
			.x_res		= 1024,
			.y_res		= 768,
			.pixel_clock	= 61714,
			.hfp		= 24,
			.hsw		= 41,
			.hbp		= 160,
			.vfp		= 3,
			.vsw		= 6,
			.vbp		= 29,
		},
	},
	{
		.name	= "720p",
		.config	= OMAP_DSS_LCD_TFT,
		.acb	= 0x28,
		.data_lines = 16,
		.timing = {
			/* 1280 x 720, 720P on DVI */
			.x_res		= 1280,
			.y_res		= 720,
			.pixel_clock	= 72000,
			.hfp		= 110,
			.hsw		= 40,
			.hbp		= 220,
			.vfp		= 5,
			.vsw		= 5,
			.vbp		= 20,
		},
	},
};



struct logic_panel *logic_find_panel(void)
{
	char *p, *q, *r;
	struct omap_custom_lcd_fields *f;
	char *panel;
	char panel_name[128];
	unsigned int val;
	int err = 0, i;
	int last, data_lines;
	int found = 0;

	panel = getenv("display");
	if (!panel) {
		printf("No 'display' variable found in environment; suppress splashimage\n");
		return NULL;
	}

	if (strchr(panel, ':')) {
		default_panel = logic_panels[0];
		default_panel.name = "custom";
		default_panel.data_lines = 16;
		strcpy(panel_name, panel);
		found = 1;
		last = 0;
		p = panel_name;
		for (i=0, f=omap_custom_lcd_fields; !last && i<ARRAY_SIZE(omap_custom_lcd_fields); ++i, ++f) {
			q = strchr(p, ':');
			if (q)
				*q = '\0';
			else
				last = 1;

			val = simple_strtoul(p, &r, 0);

			if (q && (r != q)) {
				printf("Custom display field '%s' value of '%s' invalid\n", f->field, p);
				err = 1;
				break;
			}
			switch(f->len) {
			case 1: {
					u8 *ptr = f->ptr;
					*ptr = val;
				}
				break;
			case 2: {
					u16 *ptr = f->ptr;
					*ptr = val;
				}
				break;
			default:
			case 4: {
					u32 *ptr = f->ptr;
					*ptr = val;
				}
				break;
			}
			p = q+1;
		}
		strcpy(panel_name, default_panel.name);
		if (calc_fbsize() > board_lcd_setmem(0)) {
			printf("Custom screen definition too large!\n");
			found = 0;
		}
	} else {
		/* Copy panel name and null-terminate it */
		strncpy(panel_name, panel, sizeof(panel_name));
		panel_name[sizeof(panel_name)-1] = '\0';

		/* Search for trailing "-dvi"/"-24" or "-hdmi"/"-24", if found
		 * set data_lines and strip off trailing specifier */
		data_lines = 16;
		if ((p = strrchr(panel_name, '-')) != NULL) {
			if (!strcmp(p+1, "dvi") || !strcmp(p+1, "16")) {
				data_lines = 16;
				*p='\0';
			} else if (!strcmp(p+1, "hdmi") || !strcmp(p+1, "24")) {
				data_lines = 24;
				*p='\0';
			}
		}
		
		for (i=0; i<ARRAY_SIZE(logic_panels); ++i)
			if (!strcmp(panel_name, logic_panels[i].name)) {
				default_panel = logic_panels[i];
				default_panel.data_lines = data_lines;
				found = 1;
				break;
			}
	}

	if (found) {
		printf("Found '%s' display panel\n", panel_name);
		panel_info.vl_col = default_panel.timing.x_res;
		panel_info.vl_row = default_panel.timing.y_res;
		if (default_panel.data_lines == 16)
			panel_info.vl_bpix = LCD_COLOR16;
		else
			panel_info.vl_bpix = LCD_COLOR24;
		lcd_line_length = (panel_info.vl_col * NBITS (panel_info.vl_bpix)) / 8;
		return &default_panel;
	}
	printf("display='%s' does not describe a valid screen!\n", panel);
	return NULL;
}

/* Return size of largest framebuffer (so system can reserve memory on start) */
ulong board_lcd_setmem(ulong addr)
{
	return 1600*1200*4;	/* sxga at 24bpp(w 8bits of alpha) */
}

void touchup_display_env(void)
{
	// enable the splash screen
	char splash_bmp_gz_str[12];

	sprintf(splash_bmp_gz_str, "0x%08X", (unsigned int)splash_bmp_gz);
	setenv("splashimage", splash_bmp_gz_str);
	setenv("splashpos", "m,m");
}

void lcd_setup_pinmux(int data_lines)
{
	u32 arch_number;

	arch_number = gd->bd->bi_arch_number;

	/* Setup common pins */
	MUX_VAL(CP(DSS_PCLK),		(IDIS | PTD | DIS | M0)); /*DSS_PCLK*/
	MUX_VAL(CP(DSS_HSYNC),		(IDIS | PTD | DIS | M0)); /*DSS_HSYNC*/
	MUX_VAL(CP(DSS_VSYNC),		(IDIS | PTD | DIS | M0)); /*DSS_VSYNC*/
	MUX_VAL(CP(DSS_ACBIAS),		(IDIS | PTD | DIS | M0)); /*DSS_ACBIAS*/
	MUX_VAL(CP(DSS_DATA6),		(IDIS | PTD | DIS | M0)); /*DSS_DATA6*/
	MUX_VAL(CP(DSS_DATA7),		(IDIS | PTD | DIS | M0)); /*DSS_DATA6*/
	MUX_VAL(CP(DSS_DATA8),		(IDIS | PTD | DIS | M0)); /*DSS_DATA6*/
	MUX_VAL(CP(DSS_DATA9),		(IDIS | PTD | DIS | M0)); /*DSS_DATA6*/
	MUX_VAL(CP(DSS_DATA10),		(IDIS | PTD | DIS | M0)); /*DSS_DATA6*/
	MUX_VAL(CP(DSS_DATA11),		(IDIS | PTD | DIS | M0)); /*DSS_DATA6*/
	MUX_VAL(CP(DSS_DATA12),		(IDIS | PTD | DIS | M0)); /*DSS_DATA6*/
	MUX_VAL(CP(DSS_DATA13),		(IDIS | PTD | DIS | M0)); /*DSS_DATA6*/
	MUX_VAL(CP(DSS_DATA14),		(IDIS | PTD | DIS | M0)); /*DSS_DATA6*/
	MUX_VAL(CP(DSS_DATA15),		(IDIS | PTD | DIS | M0)); /*DSS_DATA6*/

	/* omap35x use DSS_DATA0:15
	 * dm37x SOM uses DSS_DATA0:15
	 * dm37x Torpedo uses DSS_DATA18:23 as DSS_DATA0:5 */

	if (arch_number == MACH_TYPE_DM3730_TORPEDO) {
		MUX_VAL(CP(DSS_DATA18),	(IDIS | PTD | DIS | M3)); /*DSS_DATA0*/
		MUX_VAL(CP(DSS_DATA19),	(IDIS | PTD | DIS | M3)); /*DSS_DATA1*/
		MUX_VAL(CP(DSS_DATA20),	(IDIS | PTD | DIS | M3)); /*DSS_DATA2*/
		MUX_VAL(CP(DSS_DATA21),	(IDIS | PTD | DIS | M3)); /*DSS_DATA3*/
		MUX_VAL(CP(DSS_DATA22),	(IDIS | PTD | DIS | M3)); /*DSS_DATA4*/
		MUX_VAL(CP(DSS_DATA23),	(IDIS | PTD | DIS | M3)); /*DSS_DATA5*/
	} else {
		MUX_VAL(CP(DSS_DATA0),	(IDIS | PTD | DIS | M0)); /*DSS_DATA0*/
		MUX_VAL(CP(DSS_DATA1),	(IDIS | PTD | DIS | M0)); /*DSS_DATA1*/
		MUX_VAL(CP(DSS_DATA2),	(IDIS | PTD | DIS | M0)); /*DSS_DATA2*/
		MUX_VAL(CP(DSS_DATA3),	(IDIS | PTD | DIS | M0)); /*DSS_DATA3*/
		MUX_VAL(CP(DSS_DATA4),	(IDIS | PTD | DIS | M0)); /*DSS_DATA4*/
		MUX_VAL(CP(DSS_DATA5),	(IDIS | PTD | DIS | M0)); /*DSS_DATA5*/
	}

	/* No need to mux the top 8 pins if not using them */
	if (data_lines <= 16)
		return;

	MUX_VAL(CP(DSS_DATA16),	(IDIS | PTD | DIS | M0)); /*DSS_DATA16*/
	MUX_VAL(CP(DSS_DATA17),	(IDIS | PTD | DIS | M0)); /*DSS_DATA17*/

	if (arch_number == MACH_TYPE_DM3730_TORPEDO) {
		MUX_VAL(CP(SYS_BOOT0),	(IDIS | PTD | DIS | M3)); /*DSS_DATA18*/
		MUX_VAL(CP(SYS_BOOT1),	(IDIS | PTD | DIS | M3)); /*DSS_DATA19*/
		MUX_VAL(CP(SYS_BOOT3),	(IDIS | PTD | DIS | M3)); /*DSS_DATA20*/
		MUX_VAL(CP(SYS_BOOT4),	(IDIS | PTD | DIS | M3)); /*DSS_DATA21*/
		MUX_VAL(CP(SYS_BOOT5),	(IDIS | PTD | DIS | M3)); /*DSS_DATA22*/
		MUX_VAL(CP(SYS_BOOT6),	(IDIS | PTD | DIS | M3)); /*DSS_DATA23*/
	} else {
		MUX_VAL(CP(DSS_DATA18),	(IDIS | PTD | DIS | M0)); /*DSS_DATA18*/
		MUX_VAL(CP(DSS_DATA19),	(IDIS | PTD | DIS | M0)); /*DSS_DATA19*/
		MUX_VAL(CP(DSS_DATA20),	(IDIS | PTD | DIS | M0)); /*DSS_DATA20*/
		MUX_VAL(CP(DSS_DATA21),	(IDIS | PTD | DIS | M0)); /*DSS_DATA21*/
		MUX_VAL(CP(DSS_DATA22),	(IDIS | PTD | DIS | M0)); /*DSS_DATA22*/
		MUX_VAL(CP(DSS_DATA23),	(IDIS | PTD | DIS | M0)); /*DSS_DATA23*/
	}	
}

void lcd_ctrl_init(void *lcdbase)
{
	struct dispc_regs *dispc = (struct dispc_regs *) OMAP3_DISPC_BASE;
	struct logic_panel *panel;
	struct panel_config dss_panel;

	memset(&panel_info, 0, sizeof(panel_info));
	panel = logic_find_panel();
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
	dss_panel.timing_v |= ((panel->timing.vbp - 1)  << 20);
	dss_panel.pol_freq = panel->acb;
	dss_panel.pol_freq |= ((panel->config & 0x3f) << 12);
	dss_panel.lcd_size = panel->timing.x_res - 1;
	dss_panel.lcd_size |= (panel->timing.y_res - 1) << 16;
	if (panel->data_lines == 16)
		dss_panel.data_lines = 1;
	else if (panel->data_lines == 24)
		dss_panel.data_lines = 3;
	else {
		printf("%s: Invalid data_lines!\n", __FUNCTION__);
		memset(&panel_info, 0, sizeof(panel_info));
		return;
	}

	dss_panel.pixel_clock = panel->timing.pixel_clock;

	lcd_setup_pinmux(panel->data_lines);

	/* configure DSS for single graphics layer */
	omap3_dss_panel_config(&dss_panel);

	writel((ulong)lcdbase, &dispc->gfx_ba0); /* frame buffer address */
	writel((ulong)lcdbase, &dispc->gfx_ba1); /* frame buffer address */
	writel(dss_panel.lcd_size, &dispc->gfx_size); /* size - same as LCD */
	if (panel->data_lines == 16)
		writel((1<<0)|(6<<1), &dispc->gfx_attributes); /* 6=RGB16,8=RGB24 */
	else
		writel(0x91, &dispc->gfx_attributes); /* 6=RGB16,8=RGB24 */


	if (panel->data_lines == 16) {
		lcd_color_fg = 0xffff;
		lcd_color_bg = 0x0000;
	} else {
		lcd_color_fg = 0x00ffffff;
		lcd_color_bg = 0x00000000;
	}
}

void lcd_enable(void)
{
	int mSec;
	u32 arch_number;
	int gpio_backlight_pwr;
	int gpio_panel_pwr;

	arch_number = gd->bd->bi_arch_number;
	if (arch_number == MACH_TYPE_DM3730_TORPEDO
		|| arch_number == MACH_TYPE_OMAP3_TORPEDO) {
		gpio_backlight_pwr = 154;
		gpio_panel_pwr = 155;
		MUX_VAL(CP(MCBSP4_DX), (IDIS | PTD | EN  | M4)); /*GPIO_154*/

	} else {
		MUX_VAL(CP(SYS_BOOT6), (IDIS | PTU | DIS | M4)); /*GPIO_8 */
		gpio_backlight_pwr = 8;
		gpio_panel_pwr = 155;
	}
	MUX_VAL(CP(MCBSP4_FSX), (IDIS | PTD | EN  | M4)); /*GPIO_155*/

	/* Kill LCD_BACKLIGHT_PWR */
	if (!omap_request_gpio(gpio_backlight_pwr)) {
		omap_set_gpio_direction(gpio_backlight_pwr, 0);
		omap_set_gpio_dataout(gpio_backlight_pwr, 0);
	}

	lcd_is_enabled = 0; /* keep console messages on the serial port */

	/* Start clocks */
	omap3_dss_enable();

	/* Delay 300mS to allow 4.3" panel to initialize */
	for (mSec=0; mSec<300; ++mSec)
		udelay(1000);

	/*
	 * panel_enable = 155
	 * backlight 154 (torpedo); 8 (som)
	 */

	/* turn on LCD_PANEL_PWR */
	if (!omap_request_gpio(gpio_panel_pwr)) {
		omap_set_gpio_direction(gpio_panel_pwr, 0);
		omap_set_gpio_dataout(gpio_panel_pwr, 1);
	} else
		printf("%s:%d fail!\n", __FUNCTION__, __LINE__);


	/* Torpedo-20 boards uses GPIO_56 as their backlight SOM
	 * use GPIO.6 on TWL4030 */

	if (arch_number == MACH_TYPE_DM3730_TORPEDO
		|| arch_number == MACH_TYPE_OMAP3_TORPEDO) {

		MUX_VAL(CP(GPMC_NCS5), (IEN | PTD | EN | M3)); /*GPT10 backlight */
		init_gpt_timer(10, 70, 100);
	} else {
		twl4030_set_pwm0(70, 100); /* 70% backlight brighntess */
	}

	/* Sleep 300mS to allow panel to stabilize */
	for (mSec=0; mSec < 300; ++mSec)
		udelay(1000);

	/* turn on LCD_BACKLIGHT_PWR SOM LV */
	if (!omap_request_gpio(gpio_backlight_pwr)) {
		omap_set_gpio_direction(gpio_backlight_pwr, 0);
		omap_set_gpio_dataout(gpio_backlight_pwr, 1);
	} else
		printf("%s:%d fail!\n", __FUNCTION__, __LINE__);

}

int do_backlight(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	u32 arch_number;
	ulong level;

	level = simple_strtoul(argv[1], NULL, 10);

	arch_number = gd->bd->bi_arch_number;
	if (arch_number == MACH_TYPE_DM3730_TORPEDO
		|| arch_number == MACH_TYPE_OMAP3_TORPEDO) {

		/* Adjust Torpedo GPT10 timer (used for LCD_PWM0) */
		init_gpt_timer(10, level, 100);
	} else {
		/* Adjust SOM LV TWL4030 PWM0 (used for LCD_PWM0) */
		twl4030_set_pwm0(level, 100);
	}

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
	" - dump TWL PWM registers",
	""
);

struct fb_bitfield {
	u8 length;
	u8 offset;
};
static struct fb_var_screeninfo {
	int bits_per_pixel;
	u32 xres, yres;
	struct fb_bitfield red, green, blue;
	u32 line_length; /* bytes/line */
} fb_var;

void *fb_ptr;
unsigned int fb_stride;
unsigned int fb_size;
int fb_max_x, fb_max_y;

typedef struct {
  unsigned int x,y;
} point_t;

typedef unsigned int color_t;

void
draw_pixel_32(point_t *p, color_t c)
{
  unsigned int offset;

  if (p->x >= fb_max_x || p->y >= fb_max_y)
    printf("%s: point [%u:%u] out of range\n", __FUNCTION__, p->x, p->y);

  offset = p->y * fb_stride + p->x*sizeof(int);
  *((unsigned int *)((unsigned char *)fb_ptr + offset)) = c;
}

void
draw_pixel_16(point_t *p, color_t c)
{
  unsigned int offset;

  if (p->x >= fb_max_x || p->y >= fb_max_y)
    printf("%s: point [%u:%u] out of range\n", __FUNCTION__, p->x, p->y);

  offset = p->y * fb_stride + p->x*sizeof(short);
  *((unsigned short *)((unsigned char *)fb_ptr + offset)) = c;
}

void
draw_pixel_8(point_t *p, color_t c)
{
  unsigned int offset;

  if (p->x >= fb_max_x || p->y >= fb_max_y)
    printf("%s: point [%u:%u] out of range\n", __FUNCTION__, p->x, p->y);

  offset = p->y * fb_stride + p->x*sizeof(char);
  *((unsigned char *)((unsigned char *)fb_ptr + offset)) = c;
}

void (*draw_pixel)(point_t *p, color_t c);

void draw_rect(point_t *p1, point_t *p2, color_t c, int fill)
{
  point_t p;

  if (fill) {
    for (p.y = p1->y; p.y <= p2->y; ++p.y)
      for (p.x = p1->x; p.x <= p2->x; ++p.x)
	(*draw_pixel)(&p, c);
  } else {
    for (p.x = p1->x; p.x <= p2->x; ++p.x) {
      p.y = p1->y;
      (*draw_pixel)(&p, c);
      p.y = p2->y;
      (*draw_pixel)(&p, c);
    }
    for (p.y = p1->y; p.y <= p2->y; ++p.y) {
      p.x = p1->x;
      (*draw_pixel)(&p, c);
      p.x = p2->x;
      (*draw_pixel)(&p, c);
    }
  }
}

void draw_test_frame(int x, int y, int margin, color_t c)
{
  point_t start, end;

  start.x = margin;
  end.x = x - margin - 1;
  start.y = margin;
  end.y = y - margin - 1;

  draw_rect(&start, &end, c, 0);
}

void
clear_video_frame(void)
{
  memset(fb_ptr, 0, fb_size);
}

/* Draw a ramp, [l,r] to [l+w,r+h], in color *color */
void draw_ramp (point_t *start, point_t *end, struct fb_bitfield *fb_bits, struct fb_var_screeninfo *fb_var)
{
	point_t ul, br;
	int i, width_round_up;
	int colors;

	ul = *start;
	br = *end;

	colors = 1<<fb_bits->length;
	width_round_up = (end->x % colors) ? 1 : 0;

	for (i=0; i<colors; ++i) {
		draw_rect(&ul, &br, i<<fb_bits->offset, 1);
		ul.x = br.x;
		br.x += fb_var->xres / colors + ((i % 2) * width_round_up);
		if (br.x >= fb_var->xres)
			br.x = fb_var->xres-1;
	}
}

void scribble_frame_buffer(void)
{
	unsigned int x,y;
	color_t color_white, color_black, color_blue, color_red;
	unsigned int colorbitwidth, width_round_up;
	point_t start, end;

	if (fb_var.bits_per_pixel == 8)
		draw_pixel = draw_pixel_8;
	else if (fb_var.bits_per_pixel == 16)
		draw_pixel = draw_pixel_16;
	else
		draw_pixel = draw_pixel_32;

	printf("%s:%d draw_pixel %p\n", __FUNCTION__, __LINE__, draw_pixel);
  
	// white is all bits on
	color_white = (((1<<fb_var.red.length)-1) << fb_var.red.offset)
		| (((1<<fb_var.green.length)-1) << fb_var.green.offset)
		| (((1<<fb_var.blue.length)-1) << fb_var.blue.offset);

	// black is all bits off
	color_black = 0;

	color_blue = (((1 << fb_var.blue.length) - 1) << fb_var.blue.offset);

	color_red = (((1 << fb_var.red.length) - 1) << fb_var.red.offset);



	x = fb_var.xres;
	y = fb_var.yres;
	colorbitwidth = 32;

	start.x = 0;
	width_round_up = (x % colorbitwidth) ? 1 : 0;
	end.x = x / colorbitwidth + width_round_up;

	end.y = y/4;
	start.y = 0;
	draw_ramp(&start, &end, &fb_var.red, &fb_var);
	start.y = end.y;
	end.y += y/4;
	draw_ramp(&start, &end, &fb_var.green, &fb_var);
	start.y = end.y;
	end.y += y/4;
	draw_ramp(&start, &end, &fb_var.blue, &fb_var);
	/* draw stipple, stop test when error is encountered */
	for (start.y = 3 * (y / 4); start.y < y; start.y++) {
		for (start.x = 0; start.x < (x / 3); start.x++) {
			draw_rect(&start, &start,
				(start.x ^ start.y) & 1 ? color_white : color_black,
				0);
		}
	}

	/* draw vert-lines, stop test when error is encountered */
	start.y = 3 * (y / 4);
	end.y = y-1;
	for (start.x = x / 3; start.x < 2 * (x / 3); start.x++) {
		end.x = start.x;
		draw_rect(&start, &end,
			start.x & 1 ? color_white : color_black, 0);
	}

	/* draw horiz-lines, stop test when error is encountered */
	start.x = 2 * (x / 3);
	end.x = x-1;
	for (start.y = 3 * (y / 4); start.y < y; start.y++) {
		end.y = start.y;
		draw_rect(&start, &end,
			start.y & 1 ? color_white : color_black, 0);
	}


	// Draw some frames
	draw_test_frame(x, y, 0, color_white);
	draw_test_frame(x, y, 1, color_red);
	draw_test_frame(x, y, 2, color_blue);
}

int do_draw_test(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	if (panel_info.vl_col && panel_info.vl_row) {
		fb_var.xres = panel_info.vl_col;
		fb_var.yres = panel_info.vl_row;
		if (panel_info.vl_bpix == LCD_COLOR16) {
			fb_var.bits_per_pixel = 16;
			fb_var.red.offset = 11;
			fb_var.red.length = 5;
			fb_var.blue.offset = 5;
			fb_var.blue.length = 6;
			fb_var.green.offset = 0;
			fb_var.green.length = 5;
		} else  if (panel_info.vl_bpix == LCD_COLOR24) {
			fb_var.bits_per_pixel = 32;
			fb_var.red.offset = 16;
			fb_var.red.length = 8;
			fb_var.blue.offset = 8;
			fb_var.blue.length = 8;
			fb_var.green.offset = 0;
			fb_var.green.length = 8;
		}
		fb_var.line_length = fb_var.xres * (fb_var.bits_per_pixel / 8);
		fb_ptr = (void *)gd->fb_base;
		fb_stride = fb_var.line_length;
		fb_size = fb_stride * fb_var.yres;
		fb_max_x = fb_var.xres;
		fb_max_y = fb_var.yres;
		scribble_frame_buffer();
	}
	return 0;
}

U_BOOT_CMD(draw_test, 1, 1, do_draw_test,
	" - Draw ramps/stipples/boarders on LCD",
	""
);

int do_info_video(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	struct omap_video_timings *timing;
	printf("Current display parameters:\n");
	printf("name: %s\n", default_panel.name);
	printf("config: %#x\n", default_panel.config);
	printf("acb: %#x\n", default_panel.acb);
	printf("data_lines: %d\n", default_panel.data_lines);
	timing = &default_panel.timing;
	printf("x_res: %d\n", timing->x_res);
	printf("y_res: %d\n", timing->y_res);
	printf("pixel_clock: %d\n", timing->pixel_clock);
	printf("hsw: %d\n", timing->hsw);
	printf("hfp: %d\n", timing->hfp);
	printf("hbp: %d\n", timing->hbp);
	printf("vsw: %d\n", timing->vsw);
	printf("vfp: %d\n", timing->vfp);
	printf("vbp: %d\n", timing->vbp);

	return 0;
}

U_BOOT_CMD(dump_video, 1, 1, do_info_video,
	" - Displays information on video parameters",
	""
);

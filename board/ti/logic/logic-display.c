#include <common.h>
#include <asm/io.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mux.h>
#include <asm/arch/dss.h>
#include <asm/arch/timer.h>
#include <twl4030.h>
#include <lcd.h>
#include "splash-480x272.h"
#include "logic-proto.h"

DECLARE_GLOBAL_DATA_PTR;

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

	if (data_lines == 16)
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
	dss_panel.timing_v |= ((panel->timing.vbp - 1)  << 20);
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

	lcd_setup_pinmux(dss_panel.data_lines);

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

#if 0
	/* Kill SOM LCD_BACKLIGHT_PWR */
	if (!omap_request_gpio(8)) {
		omap_set_gpio_direction(8, 0);
		omap_set_gpio_dataout(8, 0);
	}
#endif
	 
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

#if 0
		MUX_VAL(CP(GPMC_NCS5), (IDIS | PTU | DIS | M4)); /*GPT10 backlight */
		omap_request_gpio(56);
		omap_set_gpio_direction(56, 0);
		omap_set_gpio_dataout(56, 1);
#else
		MUX_VAL(CP(GPMC_NCS5), (IEN | PTD | EN | M3)); /*GPT10 backlight */
		init_gpt_timer(10, 70, 100);
#endif
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

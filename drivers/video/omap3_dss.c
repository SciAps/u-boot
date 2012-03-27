/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 * Syed Mohammed Khasim <khasim <at> ti.com>
 *
 * Referred to Linux Kernel DSS driver files for OMAP3 by
 * Tomi Valkeinen from drivers/video/omap2/dss/
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation's version 2 and any
 * later version the License.
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
#include <asm/arch/dss.h>
#include <asm/arch/sys_proto.h>

#undef DEBUG
#ifdef DEBUG
#define DSS_DBG_CLK(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define DSS_DBG_CLK(fmt, ...)
#endif

/*
 * Configure VENC for a given Mode (NTSC / PAL)
 */
void omap3_dss_venc_config(const struct venc_regs *venc_cfg,
                           u32 height, u32 width)
{
    struct venc_regs *venc = (struct venc_regs *) OMAP3_VENC_BASE;
    struct dss_regs *dss = (struct dss_regs *) OMAP3_DSS_BASE;
    struct dispc_regs *dispc = (struct dispc_regs *) OMAP3_DISPC_BASE;

    writel(venc_cfg->status, &venc->status);
    writel(venc_cfg->f_control, &venc->f_control);
    writel(venc_cfg->vidout_ctrl, &venc->vidout_ctrl);
    writel(venc_cfg->sync_ctrl, &venc->sync_ctrl);
    writel(venc_cfg->llen, &venc->llen);
    writel(venc_cfg->flens, &venc->flens);
    writel(venc_cfg->hfltr_ctrl, &venc->hfltr_ctrl);
    writel(venc_cfg->cc_carr_wss_carr, &venc->cc_carr_wss_carr);
    writel(venc_cfg->c_phase, &venc->c_phase);
    writel(venc_cfg->gain_u, &venc->gain_u);
    writel(venc_cfg->gain_v, &venc->gain_v);
    writel(venc_cfg->gain_y, &venc->gain_y);
    writel(venc_cfg->black_level, &venc->black_level);
    writel(venc_cfg->blank_level, &venc->blank_level);
    writel(venc_cfg->x_color, &venc->x_color);
    writel(venc_cfg->m_control, &venc->m_control);
    writel(venc_cfg->bstamp_wss_data, &venc->bstamp_wss_data);
    writel(venc_cfg->s_carr, &venc->s_carr);
    writel(venc_cfg->line21, &venc->line21);
    writel(venc_cfg->ln_sel, &venc->ln_sel);
    writel(venc_cfg->l21__wc_ctl, &venc->l21__wc_ctl);
    writel(venc_cfg->htrigger_vtrigger, &venc->htrigger_vtrigger);
    writel(venc_cfg->savid__eavid, &venc->savid__eavid);
    writel(venc_cfg->flen__fal, &venc->flen__fal);
    writel(venc_cfg->lal__phase_reset, &venc->lal__phase_reset);
    writel(venc_cfg->hs_int_start_stop_x,
            &venc->hs_int_start_stop_x);
    writel(venc_cfg->hs_ext_start_stop_x,
            &venc->hs_ext_start_stop_x);
    writel(venc_cfg->vs_int_start_x,
            &venc->vs_int_start_x);
    writel(venc_cfg->vs_int_stop_x__vs_int_start_y,
            &venc->vs_int_stop_x__vs_int_start_y);
    writel(venc_cfg->vs_int_stop_y__vs_ext_start_x,
            &venc->vs_int_stop_y__vs_ext_start_x);
    writel(venc_cfg->vs_ext_stop_x__vs_ext_start_y,
            &venc->vs_ext_stop_x__vs_ext_start_y);
    writel(venc_cfg->vs_ext_stop_y,
            &venc->vs_ext_stop_y);
    writel(venc_cfg->avid_start_stop_x,
            &venc->avid_start_stop_x);
    writel(venc_cfg->avid_start_stop_y,
            &venc->avid_start_stop_y);
    writel(venc_cfg->fid_int_start_x__fid_int_start_y,
            &venc->fid_int_start_x__fid_int_start_y);
    writel(venc_cfg->fid_int_offset_y__fid_ext_start_x,
            &venc->fid_int_offset_y__fid_ext_start_x);
    writel(venc_cfg->fid_ext_start_y__fid_ext_offset_y,
            &venc->fid_ext_start_y__fid_ext_offset_y);
    writel(venc_cfg->tvdetgp_int_start_stop_x,
            &venc->tvdetgp_int_start_stop_x);
    writel(venc_cfg->tvdetgp_int_start_stop_y,
            &venc->tvdetgp_int_start_stop_y);
    writel(venc_cfg->gen_ctrl, &venc->gen_ctrl);
    writel(venc_cfg->output_control, &venc->output_control);
    writel(venc_cfg->dac_b__dac_c, &venc->dac_b__dac_c);

    /* Configure DSS for VENC Settings */
    writel(VENC_DSS_CONFIG, &dss->control);

    /* Configure height and width for Digital out */
    writel(((height << DIG_LPP_SHIFT) | width), &dispc->size_dig);
}

/*
 * Configure Panel Specific Parameters
 */
void omap3_dss_panel_config(const struct panel_config *panel_cfg)
{
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
    struct dispc_regs *dispc = (struct dispc_regs *) OMAP3_DISPC_BASE;
    int ret;
    u32 divisor, cm_clksel_dss;
    u32 fck_div;

    /* Calculate timing of DISPC_DIVISOR; LCD in 16:23, PCD in 0:7 */
    ret = omap3_dss_calc_divisor(panel_cfg->panel_type == 1,
			    panel_cfg->pixel_clock * 1000, &divisor, &fck_div);
    DSS_DBG_CLK("%s: Need to program CM_CLKSEL_DSS:clksel_dss1 to %u !\n", __FUNCTION__, fck_div);
    cm_clksel_dss = readl(&prcm_base->clksel_dss);
    DSS_DBG_CLK("%s: &cm_clksel_dss %p val %#x\n", __FUNCTION__, &prcm_base->clksel_dss, cm_clksel_dss);
    cm_clksel_dss &= ~0x3f;  /* clear CLKSELDSS1 */
    cm_clksel_dss |= fck_div; /* or in new clksel_dss1 */
    writel(cm_clksel_dss, &prcm_base->clksel_dss);
    DSS_DBG_CLK("%s: cm_clksel_dss %#x\n", __FUNCTION__, cm_clksel_dss);

    writel(panel_cfg->timing_h, &dispc->timing_h);
    writel(panel_cfg->timing_v, &dispc->timing_v);
    writel(panel_cfg->pol_freq, &dispc->pol_freq);
#if 1
    writel(divisor, &dispc->divisor);
#else
    writel(panel_cfg->divisor, &dispc->divisor);
#endif
    writel(panel_cfg->lcd_size, &dispc->size_lcd);
    writel((panel_cfg->load_mode << FRAME_MODE_SHIFT), &dispc->config);
    writel(((panel_cfg->panel_type << TFTSTN_SHIFT) |
            (panel_cfg->data_lines << DATALINES_SHIFT)), &dispc->control);
    writel(panel_cfg->panel_color, &dispc->default_color0);
}

struct dss_clock_info {
	/* rates that we get with dividers below */
	unsigned long fck;

	/* dividers */
	u16 fck_div;
};

struct dispc_clock_info {
	/* rates that we get with dividers below */
	unsigned long lck;
	unsigned long pck;

	/* dividers */
	u16 lck_div;
	u16 pck_div;
};

static inline int abs(int x)
{
	if (x < 0)
		return -x;
	return x;
}

/* with fck as input clock rate, find dispc dividers that produce req_pck */
void dispc_find_clk_divs(int is_tft, unsigned long req_pck, unsigned long fck,
		struct dispc_clock_info *cinfo)
{
	u16 pcd_min = is_tft ? 2 : 3;
	unsigned long best_pck;
	u16 best_ld, cur_ld;
	u16 best_pd, cur_pd;

	best_pck = 0;
	best_ld = 0;
	best_pd = 0;

	for (cur_ld = 1; cur_ld <= 255; ++cur_ld) {
		unsigned long lck = fck / cur_ld;

		for (cur_pd = pcd_min; cur_pd <= 255; ++cur_pd) {
			unsigned long pck = lck / cur_pd;
			long old_delta, new_delta;

			old_delta = abs(best_pck - req_pck);
			new_delta = abs(pck - req_pck);

			if (best_pck == 0 || new_delta < old_delta) {
				DSS_DBG_CLK("%s: cur_ld %u cur_pd %u old_delta %ld new_delta %ld\n", __FUNCTION__, cur_ld, cur_pd, old_delta, new_delta);

				best_pck = pck;
				best_ld = cur_ld;
				best_pd = cur_pd;

				DSS_DBG_CLK("%s: best_ld %u best_pd %u\n", __FUNCTION__, best_ld, best_pd);

				if (pck == req_pck)
					goto found;
			}

			if (pck < req_pck)
				break;
		}

		if (lck / pcd_min < req_pck)
			break;
	}

found:

	cinfo->lck_div = best_ld;
	cinfo->pck_div = best_pd;
	cinfo->lck = fck / cinfo->lck_div;
	cinfo->pck = cinfo->lck / cinfo->pck_div;
	DSS_DBG_CLK("%s:%d fck %lu best_ld %u best_pd %u pck %lu\n", __FUNCTION__, __LINE__, fck, best_ld, best_pd, cinfo->pck);
}

int omap3_dss_calc_divisor(int is_tft, unsigned int req_pck,
			unsigned int *dispc_divisor,
			unsigned int *result_fck_div)
{
	unsigned long prate;
	u16 fck_div, fck_div_max, fck_min_div = 1, fck_div_factor;
	int min_fck_per_pck;
	unsigned long fck, max_dss_fck = 173000000; /* max DSS VP_CLK */
	u32 cpu_family = get_cpu_family();
	struct dispc_clock_info cur_dispc;
	struct dss_clock_info best_dss;
	struct dispc_clock_info best_dispc;

	prate = 864000000; /* Fclk of DSS (864Mhz) */

	memset(&best_dss, 0, sizeof(best_dss));
	memset(&best_dispc, 0, sizeof(best_dispc));

	min_fck_per_pck = 1;

#ifdef CONFIG_OMAP44XX	
	fck_div_max = 32;
	fck_div_factor = 2;
#else
	fck_div_max = 16;
	fck_div_factor = 1;
#endif

	for (fck_div = fck_div_max; fck_div >= fck_min_div; --fck_div) {
		DSS_DBG_CLK("%s:%d fck_div %d\n", __FUNCTION__, __LINE__, fck_div);
		fck = prate / fck_div * fck_div_factor;

		if (fck > max_dss_fck) {
			DSS_DBG_CLK("%s:%d fck %lu > max_dss_fck %lu\n", __FUNCTION__, __LINE__, fck, max_dss_fck);
			continue;
		}

		if (min_fck_per_pck &&
			fck < req_pck * min_fck_per_pck) {
			DSS_DBG_CLK("%s:%d\n", __FUNCTION__, __LINE__);
			continue;
		}

		dispc_find_clk_divs(is_tft, req_pck, fck, &cur_dispc);

		DSS_DBG_CLK("%s:%d cur.pck(%d-%d) %u < best_pck(%d-%d) %u?\n", __FUNCTION__, __LINE__,
				cur_dispc.pck, req_pck,
				abs(cur_dispc.pck - req_pck),
				best_dispc.pck, req_pck,
				abs(best_dispc.pck - req_pck));

		if (abs(cur_dispc.pck - req_pck) <
			abs(best_dispc.pck - req_pck)) {

			DSS_DBG_CLK("%s:%d yes fck %lu fck_div %u\n", __FUNCTION__, __LINE__, fck, fck_div);
			best_dss.fck = fck;
			best_dss.fck_div = fck_div;

			best_dispc = cur_dispc;

			if (cur_dispc.pck == req_pck)
				break;
		}
	}

	/* Setup divisor */
	*dispc_divisor = (cur_dispc.lck_div << 16) | cur_dispc.pck_div;
	DSS_DBG_CLK("%s: fck_div %u best_dss.fck_div %u\n", __FUNCTION__, fck_div, best_dss.fck_div); 
	*result_fck_div = best_dss.fck_div;
	return 0;
}

/*
 * Enable LCD and DIGITAL OUT in DSS
 */
void omap3_dss_enable(void)
{
    struct dispc_regs *dispc = (struct dispc_regs *) OMAP3_DISPC_BASE;
    u32 l = 0;

    l = readl(&dispc->control);
    l |= DISPC_ENABLE;
    writel(l, &dispc->control);
}

#ifdef CONFIG_DSS_DUMP

#define DUMP_IT(offset) printf("%p = %08x " #offset "\n", &dispc->offset, readl(&dispc->offset))

void omap3_dss_dump(void)
{
	struct dispc_regs *dispc = (struct dispc_regs *) OMAP3_DISPC_BASE;
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;

	DUMP_IT(control);
	DUMP_IT(config);
	DUMP_IT(timing_h);
	DUMP_IT(timing_v);
	DUMP_IT(pol_freq);
	DUMP_IT(divisor);
	DUMP_IT(size_lcd);
	DUMP_IT(gfx_attributes);
	DUMP_IT(default_color0);
	printf("%p = %08x cd_clksel_dss\n", &prcm_base->clksel_dss, readl(&prcm_base->clksel_dss));
}

static int do_dss_dump (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	omap3_dss_dump();
	return (0);
}

U_BOOT_CMD(
	dss_dump,	1,	1,	do_dss_dump,
	"dump DSS registers",
	""
);

#endif

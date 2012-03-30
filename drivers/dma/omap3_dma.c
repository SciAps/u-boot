/* Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz <at> corscience.de>
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

/* This is a basic implementation of the SDMA/DMA4 controller of OMAP3
 * Tested on Silicon Revision major:0x4 minor:0x0
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/dma.h>
#include <asm/io.h>
#include <asm/errno.h>

static struct dma4 *dma4_cfg = (struct dma4 *)OMAP34XX_DMA4_BASE;
uint32_t dma_active; /* if a transfer is started the respective
	bit is set for the logical channel */

/* Initial config of omap dma */
void omap3_dma_init(void)
{
	dma_active = 0;
	writel(~0, &dma4_cfg->irqenable_l[0]);
}

/* Check if we have the given channel
 * PARAMETERS:
 * chan: Channel number
 *
 * RETURN of non-zero means error */
static inline int check_channel(uint32_t chan)
{
	if (chan < CHAN_NR_MIN || chan > CHAN_NR_MAX)
		return -EINVAL;
	return 0;
}

static inline void reset_irq(uint32_t chan)
{
	/* reset IRQ reason */
#if 1
	writel(CSR_ALL_MASK, &dma4_cfg->chan[chan].csr);
#else
	writel(0x1DFE, &dma4_cfg->chan[chan].csr);
#endif
	/* reset IRQ */
	writel((1 << chan), &dma4_cfg->irqstatus_l[0]);
	dma_active &= ~(1 << chan);
}

/* Set Source, Destination and Size of DMA transfer for the
 * specified channel.
 * PARAMETERS:
 * chan: channel to use
 * src: source of the transfer
 * dst: destination of the transfer
 * sze: Size of the transfer
 *
 * RETURN of non-zero means error */
int omap3_dma_conf_transfer(uint32_t chan, uint32_t *src, uint32_t *dst,
		uint32_t sze)
{
	if (check_channel(chan))
		return -EINVAL;
	/* CDSA0 */
	writel((uint32_t)src, &dma4_cfg->chan[chan].cssa);
	writel((uint32_t)dst, &dma4_cfg->chan[chan].cdsa);
	writel(sze, &dma4_cfg->chan[chan].cen);
	return 0;
}

/* Start the DMA transfer */
int omap3_dma_start_transfer(uint32_t chan)
{
	uint32_t val;

	if (check_channel(chan))
		return -EINVAL;

	val = readl(&dma4_cfg->chan[chan].ccr);
	/* Test for channel already in use */
	if (val & CCR_ENABLE_ENABLE) {
		printf("%s: ccr %x, returning -EBUSY\n", __FUNCTION__, val);
		return -EBUSY;
	}

	writel((val | CCR_ENABLE_ENABLE), &dma4_cfg->chan[chan].ccr);
	dma_active |= (1 << chan);
	debug("started transfer...\n");
	return 0;
}

void omap3_dma_channel_init(int channel, int next_channel, int csdp_size,
			int ccr_src_amode, int ccr_dst_amode)
{
	struct dma4_chan cfg;

	/* config the channel */
	omap3_dma_get_conf_chan(channel, &cfg);
#if 1
	cfg.csdp = CSDP_SRC_BURST_EN_64BYTES |
		CSDP_DST_BURST_EN_64BYTES | CSDP_DST_ENDIAN_LOCK_ADAPT |
		CSDP_DST_ENDIAN_LITTLE | CSDP_SRC_ENDIAN_LOCK_ADAPT |
		CSDP_SRC_ENDIAN_LITTLE;
	cfg.cfn = 1;
	cfg.ccr = CCR_READ_PRIORITY_HIGH;
	cfg.csdp |= csdp_size;
	cfg.ccr |= ccr_src_amode;
	cfg.ccr |= ccr_dst_amode;
#else
	cfg.cdsp |= CSDP_DATA_TYPE_32BIT;
	cfg.csdp = CSDP_DATA_TYPE_32BIT | CSDP_SRC_BURST_EN_64BYTES |
		CSDP_DST_BURST_EN_64BYTES | CSDP_DST_ENDIAN_LOCK_ADAPT |
		CSDP_DST_ENDIAN_LITTLE | CSDP_SRC_ENDIAN_LOCK_ADAPT |
		CSDP_SRC_ENDIAN_LITTLE;
	cfg.ccr = CCR_READ_PRIORITY_HIGH | CCR_DST_AMODE_POST_INC;
	if (src_post_incr)
	{
		cfg.ccr |= CCR_SRC_AMODE_CONSTANT;
	}
	else
	{
		cfg.ccr |= CCR_SRC_AMODE_POST_INC;
	}
#endif
	cfg.cicr = CICR_BLOCK_IE | CICR_MISALIGNED_ERR_IE
		| CICR_SUPERVISOR_ERR_IE | CICR_TRANS_ERR_IE;

	if (next_channel >= 0)
	{
		cfg.clnk_ctrl = CLNK_CTRL_ENABLE_LNK | next_channel;
	}
	else
	{
		cfg.clnk_ctrl = 0;
	}
	omap3_dma_set_conf_chan(channel, &cfg);
}

/* Busy-waiting for a DMA transfer
 * This has to be called before another transfer is started
 * PARAMETER
 * chan: Channel to wait for
 *
 * RETURN of non-zero means error*/
int omap3_dma_wait_for_transfer(uint32_t chan, int chained)
{
	uint32_t val;

	if (!chained)
	{
		if (!(dma_active & (1 << chan))) {
			val = readl(&dma4_cfg->irqstatus_l[0]);
			if (!(val & chan)) {
				debug("dma: The channel you are trying to wait for "
					"was never activated - ERROR\n");
				return -1; /* channel was never active */
			}
		}
	}

	/* all irqs on line 0 */
	while (!(readl(&dma4_cfg->irqstatus_l[0]) & (1 << chan)))
		asm("nop");

	val = readl(&dma4_cfg->chan[chan].csr);
	if ((val & CSR_TRANS_ERR) | (val & CSR_SUPERVISOR_ERR) |
			(val & CSR_MISALIGNED_ADRS_ERR)) {
		debug("err code: %X\n", val);
		debug("dma: transfer error detected\n");
		reset_irq(chan);
		return -1;
	}
	reset_irq(chan);
	return 0;
}

int omap3_dma_transfer(int channel, void *src, void *dst, int size, int flags)
{
	int res;
	res = omap3_dma_conf_transfer(channel, src, dst, (size + 3) / 4);
	if (res < 0)
	{
		printf("omap3_dma_conf_transfer = %d\n", res);
		return res;
	}
	if (flags & FB_DMA_START)
	{
		res = omap3_dma_start_transfer(channel);
		if (res < 0)
		{
			printf("omap3_dma_start_transfer = %d\n", res);
			return res;
		}
	}
	if (flags & FB_DMA_WAIT)
	{
		res = omap3_dma_wait_for_transfer(channel, 0);
	}
	return res;
}

/* set channel config to config
 *
 * RETURN of non-zero means error */
int omap3_dma_set_conf_chan(uint32_t chan, struct dma4_chan *config)
{
	if (check_channel(chan))
		return -EINVAL;
	/* Dinon - Use dma4_chan_padded instead of dma4_chan for
	 * channel 1 ~ 31 to work */
	*((struct dma4_chan *)&dma4_cfg->chan[chan]) = *config;
	return 0;
}

/* get channel config to config
 *
 * RETURN of non-zero means error */
int omap3_dma_get_conf_chan(uint32_t chan, struct dma4_chan *config)
{
	if (check_channel(chan))
		return -EINVAL;
	/* Dinon - Use dma4_chan_padded instead of dma4_chan for
	 * channel 1 ~ 31 to work */
	*config = *((struct dma4_chan *)&dma4_cfg->chan[chan]);
	return 0;
}

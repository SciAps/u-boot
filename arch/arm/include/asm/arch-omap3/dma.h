#ifndef __SDMA_H
#define __SDMA_H

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

// #ifndef __KERNEL_STRICT_NAMES
// #ifndef __ASSEMBLY__
struct dma4_chan {
	u32 ccr;
	u32 clnk_ctrl;
	u32 cicr;
	u32 csr;
	u32 csdp;
	u32 cen;
	u32 cfn;
	u32 cssa;
	u32 cdsa;
	u32 csel;
	u32 csfl;
	u32 cdel;
	u32 cdfl;
	u32 csac;
	u32 cdac;
	u32 ccen;
	u32 ccfn;
	u32 color;
};

struct dma4_chan_padded {
	u32 ccr;
	u32 clnk_ctrl;
	u32 cicr;
	u32 csr;
	u32 csdp;
	u32 cen;
	u32 cfn;
	u32 cssa;
	u32 cdsa;
	u32 csel;
	u32 csfl;
	u32 cdel;
	u32 cdfl;
	u32 csac;
	u32 cdac;
	u32 ccen;
	u32 ccfn;
	u32 color;
	u8 res1[0x60 - sizeof(struct dma4_chan)];
};

struct dma4 {
	u32 revision;
	u8 res1[0x4];
	u32 irqstatus_l[0x4];
	u32 irqenable_l[0x4];
	u32 sysstatus;
	u32 ocp_sysconfig;
	u8 res2[0x34];
	u32 caps_0;
	u8 res3[0x4];
	u32 caps_2;
	u32 caps_3;
	u32 caps_4;
	u32 gcr;
	u8 res4[0x4];
	/* Dinon - Use dma4_chan_padded instead of dma4_chan for
	 * channel 1 ~ 31 to work */
	struct dma4_chan_padded chan[32];
};
// #endif /*__ASSEMBLY__ */
// #endif /* __KERNEL_STRICT_NAMES */

/* Functions */
void omap3_dma_init(void);
int omap3_dma_conf_transfer(uint32_t chan, uint32_t *src, uint32_t *dst,
		uint32_t sze);
int omap3_dma_start_transfer(uint32_t chan);
int omap3_dma_wait_for_transfer(uint32_t chan, int chained);
int omap3_dma_conf_chan(uint32_t chan, struct dma4_chan *config);
int omap3_dma_set_conf_chan(uint32_t chan, struct dma4_chan *config);
int omap3_dma_get_conf_chan(uint32_t chan, struct dma4_chan *config);
int omap3_dma_transfer(int channel, void *src, void *dst, int size, int flags);
void omap3_dma_channel_init(int channel, int next_channel, int csdp_size,
			int ccr_src_amode, int ccr_dst_amode);

#define DMA_CHANNEL_NAND		0
#define DMA_CHANNEL_MMC			1

/* Register settings */
#define CSDP_DATA_TYPE_8BIT             0x0
#define CSDP_DATA_TYPE_16BIT            0x1
#define CSDP_DATA_TYPE_32BIT            0x2
#define CSDP_SRC_BURST_SINGLE           (0x0 << 7)
#define CSDP_SRC_BURST_EN_16BYTES       (0x1 << 7)
#define CSDP_SRC_BURST_EN_32BYTES       (0x2 << 7)
#define CSDP_SRC_BURST_EN_64BYTES       (0x3 << 7)
#define CSDP_DST_BURST_SINGLE           (0x0 << 14)
#define CSDP_DST_BURST_EN_16BYTES       (0x1 << 14)
#define CSDP_DST_BURST_EN_32BYTES       (0x2 << 14)
#define CSDP_DST_BURST_EN_64BYTES       (0x3 << 14)
#define CSDP_DST_ENDIAN_LOCK_ADAPT      (0x0 << 18)
#define CSDP_DST_ENDIAN_LOCK_LOCK       (0x1 << 18)
#define CSDP_DST_ENDIAN_LITTLE          (0x0 << 19)
#define CSDP_DST_ENDIAN_BIG             (0x1 << 19)
#define CSDP_SRC_ENDIAN_LOCK_ADAPT      (0x0 << 20)
#define CSDP_SRC_ENDIAN_LOCK_LOCK       (0x1 << 20)
#define CSDP_SRC_ENDIAN_LITTLE          (0x0 << 21)
#define CSDP_SRC_ENDIAN_BIG             (0x1 << 21)

#define CICR_MISALIGNED_ERR_IE		(0x1 << 11)
#define CICR_SUPERVISOR_ERR_IE		(0x1 << 10)
#define CICR_TRANS_ERR_IE		(0x1 << 8)
#define CICR_BLOCK_IE			(0x1 << 5)

#define CCR_READ_PRIORITY_LOW           (0x0 << 6)
#define CCR_READ_PRIORITY_HIGH          (0x1 << 6)
#define CCR_ENABLE_DISABLED             (0x0 << 7)
#define CCR_ENABLE_ENABLE               (0x1 << 7)
#define CCR_SRC_AMODE_CONSTANT          (0x0 << 12)
#define CCR_SRC_AMODE_POST_INC          (0x1 << 12)
#define CCR_SRC_AMODE_SINGLE_IDX        (0x2 << 12)
#define CCR_SRC_AMODE_DOUBLE_IDX        (0x3 << 12)
#define CCR_DST_AMODE_CONSTANT          (0x0 << 14)
#define CCR_DST_AMODE_POST_INC          (0x1 << 14)
#define CCR_DST_AMODE_SINGLE_IDX        (0x2 << 14)
#define CCR_DST_AMODE_SOUBLE_IDX        (0x3 << 14)

#define CCR_RD_ACTIVE_MASK              (1 << 9)
#define CCR_WR_ACTIVE_MASK              (1 << 10)

#define CSR_SUPER_BLOCK			(0x1 << 14)
#define CSR_DRAIN_END			(0x1 << 12)
#define CSR_MISALIGNED_ADRS_ERR		(0x1 << 11)
#define CSR_SUPERVISOR_ERR		(0x1 << 10)
#define CSR_TRANS_ERR			(0x1 << 8)
#define CSR_PKT				(0x1 << 7)
#define CSR_SYNC			(0x1 << 6)
#define CSR_BLOCK			(0x1 << 5)
#define CSR_LAST			(0x1 << 4)
#define CSR_FRAME			(0x1 << 3)
#define CSR_HALF			(0x1 << 2)
#define CSR_DROP			(0x1 << 1)

#if 1
#define CSR_ALL_MASK	\
	(CSR_MISALIGNED_ADRS_ERR \
	| CSR_SUPERVISOR_ERR | CSR_TRANS_ERR | CSR_PKT | CSR_SYNC \
	| CSR_BLOCK | CSR_LAST | CSR_FRAME | CSR_HALF | CSR_DROP)
#else
#define CSR_ALL_MASK	\
	(CSR_SUPER_BLOCK | CSR_DRAIN_END | CSR_MISALIGNED_ADRS_ERR \
	| CSR_SUPERVISOR_ERR | CSR_TRANS_ERR | CSR_PKT | CSR_SYNC \
	| CSR_BLOCK | CSR_LAST | CSR_FRAME | CSR_HALF | CSR_DROP)
#endif

#define CLNK_CTRL_ENABLE_LNK		(1 << 15)

/* others */
#define CHAN_NR_MIN			0
#define CHAN_NR_MAX			31


#define FB_DMA_START			(1 << 0)
#define FB_DMA_WAIT			(1 << 1)

#endif /* __SDMA_H */

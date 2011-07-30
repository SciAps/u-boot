/*
 * (C) Copyright 2004-2008
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *	Manikandan Pillai <mani.pillai@ti.com>
 *
 * Derived from Beagle Board and 3430 SDP code by
 *	Richard Woodruff <r-woodruff2@ti.com>
 *	Syed Mohammed Khasim <khasim@ti.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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
#include <netdev.h>
#include <asm/io.h>
#include <asm/arch/mem.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/gpio.h>
#include <i2c.h>
#include <asm/mach-types.h>
#include "logic.h"

DECLARE_GLOBAL_DATA_PTR;

#define MUX_LOGIC_HSUSB0_D5_GPIO_MUX()					\
 MUX_VAL(CP(HSUSB0_DATA5),	(IEN  | PTD | DIS | M4)) /*GPIO_189*/

#define MUX_LOGIC_HSUSB0_D5_DATA5()					\
 MUX_VAL(CP(HSUSB0_DATA5),	(IEN  | PTD | DIS | M0)) /*HSUSB0_DATA5*/

/*
 * Routine: logic_identify
 * Description: Detect if we are running on a Logic or Torpedo.
 *              This can be done by GPIO_189. If its low after driving it high,
 *              then its a SOM LV, else Torpedo.
 */
unsigned int logic_identify(void)
{
	unsigned int val = 0;
	u32 cpu_family = get_cpu_family();
	int i;

	MUX_LOGIC_HSUSB0_D5_GPIO_MUX();

	if (!omap_request_gpio(189)) {

		omap_set_gpio_direction(189, 0);
		omap_set_gpio_dataout(189, 1);

		/* Let it soak for a bit */
		for (i=0; i<0x100; ++i)
			asm("nop");

		omap_set_gpio_direction(189, 1);
		val = omap_get_gpio_datain(189);
		omap_free_gpio(189);

		printf("Board:");
		if (cpu_family == CPU_OMAP36XX) {
			printf(" DM37xx");
			if (val) {
				printf(" Torpedo\n");
				val = MACH_TYPE_DM3730_TORPEDO;
			} else {
				printf(" SOM LV\n");
				val = MACH_TYPE_DM3730_SOM_LV;
			}
		} else {
			printf(" OMAP35xx");
			if (val) {
				printf(" Torpedo\n");
				val = MACH_TYPE_OMAP3530_LV_SOM;
			} else {
				printf(" SOM LV\n");
				val = MACH_TYPE_OMAP3530_LV_SOM;
			}
		}
	}

	MUX_LOGIC_HSUSB0_D5_DATA5();

	return val;
}

#ifdef CONFIG_USB_OMAP3
/*
 * MUSB port on OMAP3EVM Rev >= E requires extvbus programming.
 */
u8 omap3_evm_need_extvbus(void)
{
	u8 retval = 0;

	retval = 1;

	return retval;
}
#endif

static void setup_nand_settings(void);

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */

	/* Update NAND settings */
	setup_nand_settings();

	/* board id for Linux (override later) */
	gd->bd->bi_arch_number = MACH_TYPE_DM3730_TORPEDO;
	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

	return 0;
}

/*
 * Check _SYSCONFIG registers and fixup bootrom code leaving them in
 * non forced-idle/smart-stdby mode
 */

static void check_sysconfig_regs(void)
{
	unsigned int temp, temp2;

	/* Since DM3730Logic boards have bootorder of 0x2f, the bootrom
	 * attempts to boot via USB and leaves OTG_SYSCONFIG in non-idle */
	temp = *(unsigned int *)OTG_SYSCONFIG;
	temp2 = OTG_SYSCONFIG_MIDLEMODE_SMART_STDBY
		| OTG_SYSCONFIG_SIDLEMODE_FORCE_IDLE
		| OTG_SYSCONFIG_AUTOIDLE;
	if (temp != temp2) {
		printf("OTG_SYSCONFIG: %08x - needs to be %08x\n", temp, temp2);
		*(unsigned int *)OTG_SYSCONFIG = temp2;
	}
}
/*
 * Routine: misc_init_r
 * Description: Init ethernet (done here so udelay works)
 */
int misc_init_r(void)
{

#ifdef CONFIG_DRIVER_OMAP34XX_I2C
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
#endif

#if defined(CONFIG_CMD_NET)
	setup_net_chip();
#endif
	gd->bd->bi_arch_number = logic_identify();

	dieid_num_r();

	check_sysconfig_regs();

	return 0;
}

/*
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers specific to the
 *		hardware. Many pins need to be moved from protect to primary
 *		mode.
 */
void set_muxconf_regs(void)
{
	MUX_EVM();
}

/* GPMC CS5 settings for Logic SOM LV/Torpedo LAN92xx Ethernet chip */
#define LOGIC_NET_GPMC_CONFIG1  0x00001000
#define LOGIC_NET_GPMC_CONFIG2  0x00080701
#define LOGIC_NET_GPMC_CONFIG3  0x00000000
#define LOGIC_NET_GPMC_CONFIG4  0x08010701
#define LOGIC_NET_GPMC_CONFIG5  0x00080a0a
#define LOGIC_NET_GPMC_CONFIG6  0x03000280
#define LOGIC_NET_GPMC_CONFIG7  0x00000848

/*
 * Routine: setup_net_chip
 * Description: Setting up the configuration GPMC registers specific to the
 *		Ethernet hardware.
 */
static void setup_net_chip(void)
{
	struct ctrl *ctrl_base = (struct ctrl *)OMAP34XX_CTRL_BASE;

	/* Configure GPMC registers */
	writel(LOGIC_NET_GPMC_CONFIG1, &gpmc_cfg->cs[1].config1);
	writel(LOGIC_NET_GPMC_CONFIG2, &gpmc_cfg->cs[1].config2);
	writel(LOGIC_NET_GPMC_CONFIG3, &gpmc_cfg->cs[1].config3);
	writel(LOGIC_NET_GPMC_CONFIG4, &gpmc_cfg->cs[1].config4);
	writel(LOGIC_NET_GPMC_CONFIG5, &gpmc_cfg->cs[1].config5);
	writel(LOGIC_NET_GPMC_CONFIG6, &gpmc_cfg->cs[1].config6);
	writel(LOGIC_NET_GPMC_CONFIG7, &gpmc_cfg->cs[1].config7);

	/* Enable off mode for NWE in PADCONF_GPMC_NWE register */
	writew(readw(&ctrl_base ->gpmc_nwe) | 0x0E00, &ctrl_base->gpmc_nwe);
	/* Enable off mode for NOE in PADCONF_GPMC_NADV_ALE register */
	writew(readw(&ctrl_base->gpmc_noe) | 0x0E00, &ctrl_base->gpmc_noe);
	/* Enable off mode for ALE in PADCONF_GPMC_NADV_ALE register */
	writew(readw(&ctrl_base->gpmc_nadv_ale) | 0x0E00,
		&ctrl_base->gpmc_nadv_ale);

}

/* GPMC CS0 settings for Logic SOM LV/Torpedo NAND settings */
#define LOGIC_NAND_GPMC_CONFIG1	0x00001800
#define LOGIC_NAND_GPMC_CONFIG2	0x00090900
#define LOGIC_NAND_GPMC_CONFIG3	0x00090902
#define LOGIC_NAND_GPMC_CONFIG4	0x07020702
#define LOGIC_NAND_GPMC_CONFIG5	0x00080909
#define LOGIC_NAND_GPMC_CONFIG6	0x000002CF
#define LOGIC_NAND_GPMC_CONFIG7	0x00000C70

static void setup_nand_settings(void)
{
	/* struct ctrl *ctrl_base = (struct ctrl *)OMAP34XX_CTRL_BASE; */

	/* Configure GPMC registers */
	writel(0x00000000, &gpmc_cfg->cs[0].config7);
	sdelay(1000);
	writel(LOGIC_NAND_GPMC_CONFIG1, &gpmc_cfg->cs[0].config1);
	writel(LOGIC_NAND_GPMC_CONFIG2, &gpmc_cfg->cs[0].config2);
	writel(LOGIC_NAND_GPMC_CONFIG3, &gpmc_cfg->cs[0].config3);
	writel(LOGIC_NAND_GPMC_CONFIG4, &gpmc_cfg->cs[0].config4);
	writel(LOGIC_NAND_GPMC_CONFIG5, &gpmc_cfg->cs[0].config5);
	writel(LOGIC_NAND_GPMC_CONFIG6, &gpmc_cfg->cs[0].config6);
	writel(LOGIC_NAND_GPMC_CONFIG7, &gpmc_cfg->cs[0].config7);
	sdelay(2000);
}

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_SMC911X
	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
#endif
	return rc;
}

#ifdef CONFIG_CMD_GPMC_CONFIG
int do_dump_gpmc(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
	gpmc_cfg = (struct gpmc *)GPMC_BASE;
	int i;
	printf("GPMC_SYSCONFIG: %08x\n", gpmc_cfg->sysconfig);
	printf("GPMC_CONFIG: %08x\n", gpmc_cfg->config);
	for (i=0; i<8; ++i) {
		struct gpmc_cs *p = &gpmc_cfg->cs[i];
		if (p->config7 & (1<<6)) {
			printf("GPMC%d: %08x %08x %08x %08x\n", i,
				p->config1, p->config2, p->config3, p->config4);
			printf("       %08x %08x %03x\n",
				p->config5, p->config6, p->config7);
		}
	}
	return 1;
}

U_BOOT_CMD(
	gpmc_config, 1, 1, do_dump_gpmc,
	"gpmc_config - dump GPMC settings",
	"dump valid GPMC configuration"
);
#endif

/*
 * (C) Copyright 2006-2008
 * Texas Instruments.
 * Author :
 *	Manikandan Pillai <mani.pillai@ti.com>
 * Derived from Beagle Board and 3430 SDP code by
 *	Richard Woodruff <r-woodruff2@ti.com>
 *	Syed Mohammed Khasim <khasim@ti.com>
 *
 * Manikandan Pillai <mani.pillai@ti.com>
 *
 * Configuration settings for the TI OMAP3 EVM board.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_ARMV7		1	/* This is an ARM V7 CPU core */
#define CONFIG_OMAP		1	/* in a TI OMAP core */
#define CONFIG_OMAP34XX		1	/* which is a 34XX */
#define CONFIG_OMAP3430		1	/* which is in a 3430 */
#define CONFIG_OMAP3_LOGIC	1	/* working with Logic OMAP boards */

/* Define following to enable new product ID code. */
#define CONFIG_OMAP3_LOGIC_USE_NEW_PRODUCT_ID	1

#define CONFIG_SDRC	/* The chip has SDRC controller */

#include <asm/arch/cpu.h>	/* get chip and board defs */
#include <asm/arch/omap3.h>

/*
 * Display CPU and Board information
 */
#define CONFIG_DISPLAY_CPUINFO		1
#define CONFIG_DISPLAY_BOARDINFO	1

/* Clock Defines */
#define V_OSCK			26000000	/* Clock output from T2 */
#define V_SCLK			(V_OSCK >> 1)

#undef CONFIG_USE_IRQ			/* no support for IRQs */
#define CONFIG_MISC_INIT_R

#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1
#define CONFIG_REVISION_TAG		1

#define CONFIG_CMDLINE_EDITING		1	/* cmd line edit/history */
#define CONFIG_ZERO_BOOTDELAY_CHECK	1	/* check keypress with zero bootdelay */

/*
 * Size of malloc() pool
 */
#define CONFIG_ENV_SIZE			(128 << 10)	/* 128 KiB */
						/* Sector */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (6 << 20))

/*
 * Add GDB information (section starts) into bdinfo; used in gdb
 * "add-symbol-file" to specify start of .data, .bss, .rodata sections
 */
#define CONFIG_GDB_SECTION_STARTS

/*
 * Hardware drivers
 */

/*
 * NS16550 Configuration
 */
#define V_NS16550_CLK			48000000	/* 48MHz (APLL96/2) */

#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		V_NS16550_CLK

/*
 * select serial console configuration
 */
#define CONFIG_CONS_INDEX		1
#define CONFIG_SYS_NS16550_COM1		OMAP34XX_UART1
#define CONFIG_SERIAL1			1	/* UART1 on OMAP3 EVMOMAP Logic boards */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
/* But don't allow overwriting "defaultecc" */
#define CONFIG_CHECK_SETENV

#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{4800, 9600, 19200, 38400, 57600,\
					115200}
#define CONFIG_MMC			1
#define CONFIG_OMAP3_MMC		1
#define CONFIG_DOS_PARTITION		1

/* DDR - I use Micron DDR */
#define CONFIG_OMAP3_MICRON_DDR		1

/* USB
 * Enable CONFIG_MUSB_HCD for Host functionalities MSC, keyboard
 * Enable CONFIG_MUSB_UDD for Device functionalities.
 */
#define CONFIG_USB_OMAP3		1
#define CONFIG_MUSB_HCD			1
/* #define CONFIG_MUSB_UDC		1 */

#ifdef CONFIG_USB_OMAP3

#ifdef CONFIG_MUSB_HCD
#define CONFIG_CMD_USB

#define CONFIG_USB_STORAGE
#define CONGIG_CMD_STORAGE
#define CONFIG_CMD_FAT

#ifdef CONFIG_USB_KEYBOARD
#define CONFIG_SYS_USB_EVENT_POLL
// #define CONFIG_PREBOOT "usb start"
#endif /* CONFIG_USB_KEYBOARD */

#endif /* CONFIG_MUSB_HCD */

#ifdef CONFIG_MUSB_UDC
/* USB device configuration */
#define CONFIG_USB_DEVICE		1
#define CONFIG_USB_TTY			1
#define CONFIG_SYS_CONSOLE_IS_IN_ENV	1
/* Change these to suit your needs */
#define CONFIG_USBD_VENDORID		0x0451
#define CONFIG_USBD_PRODUCTID		0x5678
#define CONFIG_USBD_MANUFACTURER	"Texas Instruments"
#define CONFIG_USBD_PRODUCT_NAME	"EVM"
#endif /* CONFIG_MUSB_UDC */

#endif /* CONFIG_USB_OMAP3 */

/* commands to include */
#include <config_cmd_default.h>

#define CONFIG_CMD_EXT2		/* EXT2 Support			*/
#define CONFIG_CMD_FAT		/* FAT support			*/
#define CONFIG_CMD_JFFS2	/* JFFS2 Support		*/
#define CONFIG_CMD_NAND_YAFFS	/* YAFFS NAND Support		*/
// #define CONFIG_YAFFS2		/* YAFFS2 Support		*/
#define CONFIG_YAFFS2_NEW	/* Newer YAFFS2 Support		*/

#define CONFIG_CMD_I2C		/* I2C serial bus support	*/
#define CONFIG_CMD_MMC		/* MMC support			*/
#define CONFIG_CMD_NAND		/* NAND support			*/
#define CONFIG_MTD_DEVICE	/* needed for MTD mtdparts support	*/
#define CONFIG_CMD_MTDPARTS	/* MTD partition support	*/
#define MTDIDS_DEFAULT			"nand0=omap2-nand.0"
#define MTDPARTS_DEFAULT		"mtdparts=omap2-nand.0:512k(x-loader),"\
					"1792k(u-boot),256k(u-boot-env),"\
					"4m(kernel),-(fs)"
#define CONFIG_NAND_SET_DEFAULT	/* Set default NAND access method */
#define CONFIG_NAND_MULTIPLE_ECC	/* NAND has multiple ECC methods */
#define CONFIG_TOUCHUP_ENV	/* Set board-specific environment vars */
#define CONFIG_CMD_NAND_LOCK_UNLOCK	/* nand (un)lock commands	*/
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING
#define CONFIG_CMD_CACHE	/* Cache control		*/
#define CONFIG_CMD_TIME		/* time command			*/

#define CONFIG_L2_OFF			1 /* Keep L2 Cache Disabled */

#define BOARD_LATE_INIT

#define CONFIG_CMD_FLASH		/* flinfo, erase, protect	*/
#undef CONFIG_CMD_FPGA		/* FPGA configuration Support	*/
#undef CONFIG_CMD_IMI		/* iminfo			*/
#undef CONFIG_CMD_IMLS		/* List all found images	*/

#define CONFIG_CMD_GPMC_CONFIG	/* gpmc_config */
#define CONFIG_CMD_MUX_CONFIG	/* mux_config */
#define CONFIG_CMD_SDRC_CONFIG	/* sdrc_config */

#define CONFIG_HARD_I2C			1
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_SYS_I2C_SLAVE		1
#define CONFIG_SYS_I2C_BUS		0
#define CONFIG_SYS_I2C_BUS_SELECT	1
#define CONFIG_DRIVER_OMAP34XX_I2C	1
#define CONFIG_I2C_MULTI_BUS           1

#define CONFIG_MTD_DEBUG		1
#define CONFIG_MTD_DEBUG_VERBOSE	-1
#define CONFIG_MTD_SKIP_BBTSCAN		1	/* Skip NAND bad block scan */
#define CONFIG_TOOL_SIGNGP		1

/*
 * TWL4030
 */
#define CONFIG_TWL4030_POWER		1
#define CONFIG_TWL4030_GPIO		1
#define CONFIG_TWL4030_PWM		1
#define CONFIG_TWL4030_CHARGING		1

/* Charge the batter unless $disablecharging == "yes" */
#define CONFIG_ENABLE_TWL4030_CHARGING	1

/*
 * Board NAND Info.
 */
#define CONFIG_SYS_NAND_ADDR		NAND_BASE	/* physical address */
							/* to access nand */
#define CONFIG_SYS_NAND_BASE		NAND_BASE	/* physical address */
							/* to access */
							/* nand at CS0 */

#define CONFIG_SYS_MAX_NAND_DEVICE	1		/* Max number of */
							/* NAND devices */
#define CONFIG_JFFS2_NAND
/* nand device jffs2 lives on */
#define CONFIG_JFFS2_DEV		"nand0"
/* start of jffs2 partition */
#define CONFIG_JFFS2_PART_OFFSET	0x680000
#define CONFIG_JFFS2_PART_SIZE		0xf980000	/* sz of jffs2 part */

/* Environment information */
#define CONFIG_BOOTDELAY	10

#define CONFIG_BOOTFILE		uImage

#define CONFIG_PREBOOT \
	"echo ======================NOTICE============================;"\
	"echo The u-boot environment is not set. - You are;"            \
	"echo required to set a valid display for your LCD panel.;"	\
	"echo Valid display options are:;"				\
	"echo \"  2 == LQ121S1DG31     TFT SVGA    (12.1)  Sharp\";"	\
	"echo \"  3 == LQ036Q1DA01     TFT QVGA    (3.6)   Sharp w/ASIC\";" \
	"echo \"  5 == LQ064D343       TFT VGA     (6.4)   Sharp\";"	\
	"echo \"  7 == LQ10D368        TFT VGA     (10.4)  Sharp\";"	\
	"echo \" 15 == LQ043T1DG01     TFT WQVGA   (4.3)   Sharp (DEFAULT)\";" \
	"echo \" vga[-16 OR -24]       LCD VGA     640x480\";"          \
	"echo \" svga[-16 OR -24]      LCD SVGA    800x600\";"          \
	"echo \" xga[-16 OR -24]       LCD XGA     1024x768\";"         \
	"echo \" 720p[-16 OR -24]      LCD 720P    1280x720\";"         \
	"echo \" sxga[-16 OR -24]      LCD SXGA    1280x1024\";"        \
	"echo \" uxga[-16 OR -24]      LCD UXGA    1600x1200\";"        \
	"echo MAKE SURE YOUR DISPLAY VARIABLE IS CORRECTLY ENTERED!;"	\
	"setenv display 15;"						\
	"setenv preboot;"						\
	"saveenv;"

#ifdef CONFIG_USB_TTY
#define OMAP3LOGIC_USBTTY "usbtty=cdc_acm\0"
#else
#define OMAP3LOGIC_USBTTY
#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
	"mtdids=" MTDIDS_DEFAULT "\0"	\
	"mtdparts=" MTDPARTS_DEFAULT "\0"	\
	"disablecharing=no\0" \
	"autoboot=if mmc init; then " \
			"if run loadbootscript; then " \
				"run bootscript; " \
			"else " \
				"run defaultboot;" \
			"fi; " \
		"else run defaultboot; fi\0" \
	"defaultboot=run mmcramboot\0" \
	"disablecharging no\0" \
	"loadaddr=0x81000000\0" \
	"serverip=192.168.3.5\0" \
	OMAP3LOGIC_USBTTY \
	"consoledevice=ttyO0\0" \
	"display=15\0" \
	"setconsole=setenv console ${consoledevice},${baudrate}n8\0" \
	"dump_bootargs=echo 'Bootargs: '; echo $bootargs\0" \
	"rotation=0\0" \
	"rootdevice=/dev/mtdblock4\0" \
	"vrfb_arg=if itest ${rotation} -ne 0; then " \
			"setenv bootargs ${bootargs} omapfb.vrfb=y omapfb.rotate=${rotation}; " \
		"fi\0" \
	"otherbootargs=ignore_loglevel early_printk no_console_suspend\0" \
	"addmtdparts=setenv bootargs ${bootargs} ${mtdparts}\0" \
	"common_bootargs=setenv bootargs ${bootargs} display=${display} " \
		"${otherbootargs};" \
		"run addmtdparts; " \
		"run vrfb_arg\0" \
	"mmcrootdevice=/dev/mmcblk0p2\0" \
	"mmcargs=run setconsole; setenv bootargs console=${console} " \
		"root=${mmcrootdevice} rw " \
		"rootfstype=ext3 rootwait\0" \
	"rootfstype=yaffs2\0" \
	"nandargs=run setconsole; setenv bootargs console=${console} " \
		"root=${rootdevice} rw " \
		"rootfstype=${rootfstype}\0" \
	"loadbootscript=fatload mmc 0 ${loadaddr} boot.scr\0" \
	"bootscript=echo 'Running bootscript from mmc ...'; " \
		"source ${loadaddr}\0" \
	"loaduimage=mmc init; " \
		"fatload mmc 0 ${loadaddr} ${bootfile}\0" \
	"mmcboot=echo 'Booting from mmc ...'; " \
		"run mmcargs; " \
		"run common_bootargs; " \
		"run dump_bootargs; " \
		"run loaduimage; " \
		"bootm ${loadaddr}\0" \
	"nandkerneloffset=0x280000\0" \
	"nandkernelsize=0x300000\0" \
	"nandboot=echo 'Booting from nand ...'; " \
		"run nandargs; " \
		"run common_bootargs; " \
		"run dump_bootargs; " \
		"nand read.i ${loadaddr} ${nandkerneloffset} ${nandkernelsize}; " \
		"bootm ${loadaddr}\0" \
	"mtdboot=echo \"Booting kernel from ram w/${rootfstype} rootfs...\"; " \
		"run nandargs; " \
		"run common_bootargs; " \
		"bootm ${loadaddr}\0" \
	"tftpmtdboot=echo \"Booting kernel from tftp w/${rootfstype} rootfs...\"; " \
		"run nandargs; " \
		"run common_bootargs; " \
		"tftpboot $loadaddr ${bootfile}; " \
		"bootm ${loadaddr}\0" \
	"rootpath=/opt/nfs-exports/ltib-omap\0" \
	"nfsoptions=,wsize=1500,rsize=1500\0"				\
	"nfsargs=run setconsole; setenv bootargs console=${console} " \
		"root=/dev/nfs rw " \
		"nfsroot=${serverip}:${rootpath}${nfsoptions} ip=dhcp\0" \
	"nfsboot=echo 'Booting from network using NFS rootfs...'; " \
		"run nfsargs; " \
		"run common_bootargs; " \
		"run dump_bootargs; " \
		"tftpboot $loadaddr ${bootfile}; "\
		"bootm ${loadaddr}\0" \
	"ramdisksize=64000\0" \
	"ramdiskaddr=0x82000000\0" \
	"ramdiskimage=rootfs.ext2.gz.uboot\0" \
	"ramargs=run setconsole; setenv bootargs console=${console} " \
		"root=/dev/ram rw ramdisk_size=${ramdisksize}\0" \
	"mmcramboot=echo 'Booting from mmc w/ramdisk...'; " \
		"run ramargs; " \
		"run common_bootargs; " \
		"run dump_bootargs; " \
		"run loaduimage; " \
		"fatload mmc 0 ${ramdiskaddr} ${ramdiskimage}; "\
		"bootm ${loadaddr} ${ramdiskaddr}\0" \
	"ramboot=echo 'Booting kernel/ramdisk rootfs from tftp...'; " \
		"run ramargs; " \
		"run common_bootargs; " \
		"run dump_bootargs; " \
		"tftpboot ${loadaddr} ${bootfile}; "\
		"tftpboot ${ramdiskaddr} ${ramdiskimage}; "\
		"bootm ${loadaddr} ${ramdiskaddr}\0" \
	"xipboot=echo 'Booting kernel/ramdisk rootfs from ram(assumed loaded from .elf file)...'; " \
		"run ramargs; " \
		"run common_bootargs; " \
		"run dump_bootargs; " \
		"bootm ${loadaddr} ${ramdiskaddr}\0"

#define CONFIG_BOOTCOMMAND \
	"run autoboot"

#define CONFIG_AUTO_COMPLETE	1
/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser */
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_SYS_PROMPT		"OMAP Logic # "
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		64	/* max number of command */
						/* args */
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		(CONFIG_SYS_CBSIZE)
/* memtest works on */
#define CONFIG_SYS_MEMTEST_START	(OMAP34XX_SDRC_CS0)
#define CONFIG_SYS_MEMTEST_END		(OMAP34XX_SDRC_CS0 + \
					0x01F00000) /* 31MB */

#define CONFIG_SYS_LOAD_ADDR		(OMAP34XX_SDRC_CS0) /* default load */
								/* address */

/*
 * OMAP3 has 12 GP timers, they can be driven by the system clock
 * (12/13/16.8/19.2/38.4MHz) or by 32KHz clock. We use 13MHz (V_SCLK).
 * This rate is divided by a local divisor.
 */
#define CONFIG_SYS_TIMERBASE		OMAP34XX_GPT2
#define CONFIG_SYS_PTV			2	/* Divisor: 2^(PTV+1) => 8 */
#define CONFIG_SYS_HZ			1000

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128 << 10)	/* regular stack 128 KiB */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4 << 10)	/* IRQ stack 4 KiB */
#define CONFIG_STACKSIZE_FIQ	(4 << 10)	/* FIQ stack 4 KiB */
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	2	/* CS1 may or may not be populated */
#define PHYS_SDRAM_1		OMAP34XX_SDRC_CS0
#define PHYS_SDRAM_1_SIZE	(32 << 20)	/* at least 32 MiB */
#define PHYS_SDRAM_2		OMAP34XX_SDRC_CS1

/* SDRAM Bank Allocation method */
#define SDRC_R_B_C		1

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

/* variable that's non-zero if flash exists */
#define CONFIG_SYS_FLASH_PRESENCE omap3logic_flash_exists
#ifndef __ASSEMBLY__
extern int omap3logic_flash_exists;
#endif
#define CONFIG_SYS_FLASH_BASE		0x10000000 /* FLASH base address */
#define CONFIG_SYS_FLASH_SIZE			8 /* 8MB */
#define CONFIG_SYS_MAX_FLASH_SECT		(64+8) /* 8MB/128K */
#define CONFIG_SYS_MAX_FLASH_BANKS		1
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE
#undef	CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_CFI		/* use the Common Flash Interface */
#define CONFIG_FLASH_CFI_DRIVER	/* use the CFI driver */


/* **** PISMO SUPPORT *** */

/* Configure the PISMO */
#define PISMO1_NAND_SIZE		GPMC_SIZE_128M

#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 2 sectors */

/* Monitor at start of flash */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_FLASH_BASE

#define SMNAND_ENV_OFFSET		0x240000 /* environment starts here */

#if defined(CONFIG_CMD_NAND)
#define CONFIG_NAND_OMAP_GPMC
#define CONFIG_MTD_NAND_BCH
#define CONFIG_MTD_NAND_ECC_BCH
#define CONFIG_BCH
#define GPMC_NAND_ECC_LP_x16_LAYOUT	1
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET		SMNAND_ENV_OFFSET
#endif

#define CONFIG_ENV_ADDR			CONFIG_ENV_OFFSET
#define CONFIG_ENV_RANGE		(2 * CONFIG_ENV_SIZE)

/*
 * Support for relocation
 */
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_ADDR	0x4020f800
#define CONFIG_SYS_INIT_RAM_SIZE	0x800
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - \
					 GENERATED_GBL_DATA_SIZE)

/*
 * Define the board revision statically
 */
/* #define CONFIG_STATIC_BOARD_REV	OMAP3EVM_BOARD_GEN_2 */

/*----------------------------------------------------------------------------
 * SMSC9115 Ethernet from SMSC9118 family
 *----------------------------------------------------------------------------
 */
#if defined(CONFIG_CMD_NET)

#define CONFIG_NET_MULTI
#define CONFIG_SMC911X
#define CONFIG_SMC911X_16_BIT
#define CONFIG_SMC911X_BASE	0x08000000

#endif /* (CONFIG_CMD_NET) */

/*
 * BOOTP fields
 */

#define CONFIG_BOOTP_SUBNETMASK		0x00000001
#define CONFIG_BOOTP_GATEWAY		0x00000002
#define CONFIG_BOOTP_HOSTNAME		0x00000004
#define CONFIG_BOOTP_BOOTPATH		0x00000010

/* Add graphics support */
#define CONFIG_VIDEO_OMAP3
#define CONFIG_LCD
#define LCD_BPP LCD_COLOR16
#define CONFIG_CMD_BMP
#define CONFIG_BMP_16BPP
#define CONFIG_VIDEO_BMP_GZIP
#define CONFIG_SYS_VIDEO_LOGO_MAX_SIZE (2 << 20)
#define CONFIG_SPLASH_SCREEN
#define CONFIG_SPLASH_SCREEN_ALIGN	1
#define CONFIG_SYS_WHITE_ON_BLACK	/* White characters on black background */

/* Have board_set_lcdmem() function */
#define CONFIG_BOARD_LCD_SETMEM

/* Have percent ouptut function for LCD */
#define CONFIG_LCD_PERCENT

#endif /* __CONFIG_H */

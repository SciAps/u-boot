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
#define CONFIG_OMAP_DMA		1

#define CONFIG_USE_ARCH_MEMSET	1
#define CONFIG_USE_ARCH_MEMCPY	1

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
#define CONFIG_SERIAL_OUTPUT_FIFO	/* Use full serial tx fifo */

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

#define CONFIG_USB_OMAP3		1
#define CONFIG_MUSB_UDC			1
#define CONFIG_FASTBOOT			1
#define CONFIG_TWL4030_USB		1
#define CONFIG_USB_DEVICE		1

#if 0
/* USB
 * Enable CONFIG_MUSB_HCD for Host functionalities MSC, keyboard
 * Enable CONFIG_MUSB_UDD for Device functionalities.
 */
#define CONFIG_USB_OMAP3		1
#define CONFIG_MUSB_HCD			1
#endif
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
# ifdef CONFIG_FASTBOOT
#  define CONFIG_CMD_FASTBOOT
#  define CONFIG_FASTBOOT_TRANSFER_BUFFER         (PHYS_SDRAM_1 + SZ_16M)
#  define CONFIG_FASTBOOT_TRANSFER_BUFFER_SIZE    (SZ_128M)
   /* if already present, use already existing NAND macros for block & oob size */
#  define FASTBOOT_NAND_BLOCK_SIZE                2048
#  define FASTBOOT_NAND_OOB_SIZE                  64
   /* Fastboot product name */
#  define FASTBOOT_PRODUCT_NAME   "dm3730logic"
   /* Use HS */
#  define USB_BCD_VERSION                 0x0200

#  define CONFIG_USBD_VENDORID		0x0451
#  define CONFIG_USBD_PRODUCTID		0x5678
#  define CONFIG_USBD_MANUFACTURER	"LogicPD"
#  define CONFIG_USBD_PRODUCT_NAME	"EVM"
//#  define CONFIG_USBD_PRODUCT_NAME	getenv("logic_model")
# else
/* USB device configuration */
#  define CONFIG_USB_DEVICE		1
#  define CONFIG_USB_TTY			1
#  define CONFIG_SYS_CONSOLE_IS_IN_ENV	1
/* Change these to suit your needs */
#  define CONFIG_USBD_VENDORID		0x0451
#  define CONFIG_USBD_PRODUCTID		0x5678
#  define CONFIG_USBD_MANUFACTURER	"Texas Instruments"
#  define CONFIG_USBD_PRODUCT_NAME	"EVM"
# endif /* CONFIG_FASTBOOT */
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
#define MTDIDS_NAND_DEFAULT		"nand0=omap2-nand.0"
#define MTDIDS_NOR_DEFAULT		"nor0=physmap-flash.0"
#define MTDPARTS_NAND_DEFAULT		"mtdparts=omap2-nand.0:512k(x-loader),"\
					"1664k(u-boot),384k(u-boot-env),"\
					"10240k(boot),128000k(system),"\
					"128000k(userdata),255488k(cache)"
#define MTDFLAGS_NAND_DEFAULT	"default=ecc_chip;x-loader=ecc_hw,repeat;system=yaffs;userdata=yaffs;cache=yaffs"

#define MTDPARTS_NOR_DEFAULT		"physmap-flash.0:-(nor)"

/* mtdparts/id default values are dynamic since some modules may
 * not have NOR flash */
#define CONFIG_MTDPARTS_DYNAMIC_DEFAULT

#define CONFIG_NAND_SET_DEFAULT	/* Set default NAND access method */
#define CONFIG_NAND_MULTIPLE_ECC	/* NAND has multiple ECC methods */
#define CONFIG_TOUCHUP_ENV	/* Set board-specific environment vars */
#define CONFIG_CMD_NAND_LOCK_UNLOCK	/* nand (un)lock commands	*/
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING
#define CONFIG_CMD_CACHE	/* Cache control		*/
#define CONFIG_CMD_TIME		/* time command			*/

#if 1
/* L2 was disabled since observed that large displays (720p) weren't working
 * Verify can boot kernel using either XGA or 720p HDMI displays settings */
#define CONFIG_L2_OFF			1 /* Keep L2 Cache Disabled */
#endif

#define BOARD_LATE_INIT

#define CONFIG_CMD_FLASH		/* flinfo, erase, protect	*/
#undef CONFIG_CMD_FPGA		/* FPGA configuration Support	*/
#undef CONFIG_CMD_IMI		/* iminfo			*/
#undef CONFIG_CMD_IMLS		/* List all found images	*/

#define CONFIG_CMD_GPMC_CONFIG	/* gpmc_config */
#define CONFIG_CMD_MUX_CONFIG	/* mux_config */
#define CONFIG_CMD_SDRC_CONFIG	/* sdrc_config */
#define CONFIG_CMD_SETEXPR	/* setexpr command */

#define CONFIG_HARD_I2C			1
#define CONFIG_SYS_I2C_SPEED		400000
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
#define CONFIG_BOOTDELAY	3

#define CONFIG_BOOTFILE		uImage

#ifdef FASTBOOT_ONLY
# define CONFIG_PREBOOT "fastboot;poweroff\n"
#else
# define CONFIG_PREBOOT \
	"if true;then\n" \
	"  echo\n"                                                                                  \
	"  echo =================================== NOTICE ===================================\n"   \
	"  echo \"The U-Boot environment was not found. If the display is not set properly     \"\n"\
	"  echo \"linux will not have video support.\"\n"	                                    \
	"  echo \n"                                                                                 \
	"  echo \"Valid display options are:\"\n"                                                   \
	"  echo \"  2 == LQ121S1DG31     TFT SVGA    (12.1)  Sharp\"\n"	                            \
	"  echo \"  3 == LQ036Q1DA01     TFT QVGA    (3.6)   Sharp w/ASIC\"\n"                      \
	"  echo \"  5 == LQ064D343       TFT VGA     (6.4)   Sharp\"\n"	                            \
	"  echo \"  7 == LQ10D368        TFT VGA     (10.4)  Sharp\"\n"	                            \
	"  echo \" 15 == LQ043T1DG01     TFT WQVGA   (4.3)   Sharp \"\n"                            \
	"  echo \" 28 == LQ043T1DG28     TFT WQVGA   (4.3)   Sharp (DEFAULT)\"\n"                   \
	"  echo \" vga[-16 OR -24]       LCD VGA     640x480\"\n"                                   \
	"  echo \" svga[-16 OR -24]      LCD SVGA    800x600\"\n"                                   \
	"  echo \" xga[-16 OR -24]       LCD XGA     1024x768\"\n"                                  \
	"  echo \" 720p[-16 OR -24]      LCD 720P    1280x720\"\n"                                  \
	"  echo \" sxga[-16 OR -24]      LCD SXGA    1280x1024\"\n"                                 \
	"  echo \" uxga[-16 OR -24]      LCD UXGA    1600x1200\"\n"                                 \
	"  echo \n "                                                                                \
	"  echo \"Default `display` environment variable is now being set to: 28\"\n"               \
	"  setenv display 28\n"						                            \
	"  setenv preboot\n"						                            \
	"  echo \n "                                                                                \
	"  echo \"At the U-Boot prompt type commands: `setenv display <num>`, then type\"\n"        \
	"  echo \"`saveenv` to save the environment to NAND flash.  This will avoid seeing\"\n"     \
	"  echo \"this notice on future boots\" \n "                                                \
	"  echo =================================== NOTICE ===================================\n"   \
	"  echo \n " \
	"fi"
#endif

#ifdef CONFIG_USB_TTY
#define OMAP3LOGIC_USBTTY "usbtty=cdc_acm\0"
#else
#define OMAP3LOGIC_USBTTY
#endif

#define CONFIG_BOOTCOMMAND \
	"run check_rebootreason;run autoboot;fastboot"

#define CONFIG_MMC_BOOTSCRIPT_NAME \
        "boot.scr"

/*****************************************************************************/
/* Default Environment                                                       */
/*                                                                           */
/* When reconfiguring the boot environment, be sure to set the environment   */
/* variables as indicated below for a successful boot.                       */
/*                                                                           */
/* - $kernel_location                                                        */
/*   Must always be set.  Values can be ram, nand, nand-part, mmc, or tftp.  */
/*                                                                           */
/* - $rootfs_location                                                        */
/*   Must always be set.  Values can be:                                     */
/*           ram, tftp, /dev, nfs, mmc, nand, nand-part.                     */
/*                                                                           */
/* - $rootfs_type                                                            */
/*   Must always be set.  Values can be ramdisk, jffs, yaffs, ext3, or nfs.  */
/*                                                                           */
/* - $loadaddr                                                               */
/*   Must always be set.                                                     */
/*                                                                           */
/* - If booting from /dev based file system, then $rootfs_device must be     */
/*   set.                                                                    */
/* - If booting kernel from nand-part location, $kernel_partition            */
/*   must be set.                                                            */
/* - If rootfs is coming from nand-part (i.e. ramdisk in NAND), then         */
/*   $ramdisk_partition must be set.                                         */
/*   must be set.                                                            */
/* - If booting from a ramdisk image, then $ramdisksize, and $ramdiskaddr    */
/*   must be set.                                                            */
/* - If booting from an nfs location, then $serverip, $nfsrootpath, and      */
/*   $nfsoptions must be set.                                                */
/* - If booting from nand, $ramdisk_partition,                               */
/*   $kernel_partition must be set.                                          */
/* - If booting from a file system, $ramdiskimage, and $kernelimage must be  */
/*   set.                                                                    */
/* - Optionally, a boot script named "boot.scr" can be placed in SD to       */
/*   override any other boot scripts.                                        */
/*****************************************************************************/

/*****************************************************************************/
/* When using the makenandboot script, be sure to set:                       */
/*                                                                           */
/* - xloadimage         File name of x-loader on the SD card                 */
/* - ubootimage         File name of u-boot on the SD card                   */
/* - kernelimage        File name of the kernel image on the SD card         */
/* - ramdiskimage       File name of the ramdisk image on the SD card        */
/* - xloader_partition  Name of the x-loader partition in mtdparts           */
/* - uboot_partition    Name of the u-boot partition in mtdparts             */
/* - kernel_partition   Name of the kernel partition in mtdparts             */
/* - ramdisk_partition  Name of the ramdisk partition in mtdparts            */
/*                                                                           */
/* When using makeyaffsboot script, be sure to set:                          */
/*                                                                           */
/* - xloadimage         File name of x-loader on the SD card                 */
/* - ubootimage         File name of u-boot on the SD card                   */
/* - kernelimage        File name of the kernel image on the SD card         */
/* - yaffsimage         File name of the YAFFS root FS on the SD card        */
/* - xloader_partition  Name of the x-loader partition in mtdparts           */
/* - uboot_partition    Name of the u-boot partition in mtdparts             */
/* - kernel_partition   Name of the kernel partition in mtdparts             */
/* - yaffs_partition    Name of the YAFFS root FS in mtdparts                */
/*****************************************************************************/

#define CONFIG_EXTRA_ENV_SETTINGS \
	OMAP3LOGIC_USBTTY \
	"bootargs=\0" \
	"androidbootargs=setenv bootargs ${bootargs} omap_vout.vid1_static_vrfb_alloc=y " \
	"vram=32M omapfb.vram=0:16M " \
	"androidboot.console=${consoledevice} androidboot.serialno=${logic_serial} " \
	"androidboot.charger=true" \
	"\0" \
	"otherbootargs= \0" \
	"consoledevice=ttyO0\0" \
	"autoload=no\0" \
	"setconsole=setenv console ${consoledevice},${baudrate}n8\0" \
	"disablecharging=no\0" \
	"mmc_bootscript_addr=0x80FF0000\0" \
	"disablecharging no\0" \
	"display=28\0" \
	"loadaddr=0x81000000\0" \
	"kernel_location=mmc \0" \
	"rootfs_location=blah \0" \
	"rootfs_type=ramdisk \0" \
	"rootfs_device=/dev/mtdblock5 \0" \
	"xloadimage=mlo\0" \
	"ubootimage=u-boot.bin.ift\0" \
	"kernelimage=boot.img\0" \
	"ramdiskimage=rootfs.ext2.gz.uboot\0" \
	"yaffsimage=rootfs.yaffs2\0" \
	"xloader_partition=x-loader\0" \
	"uboot_partition=u-boot\0" \
	"kernel_partition=boot\0" \
	"ramdisk_partition=ramdisk\0" \
	"yaffs_partition=fs\0" \
	"arg_loadcmd=fatload mmc 1\0" \
	"ramdisksize=64000\0" \
	"ramdiskaddr=0x82000000\0" \
	"tftpdir=\0" \
	"serverip=192.168.3.10\0" \
	"nfsrootpath=/opt/nfs-exports/ltib-omap\0" \
	"nfsoptions=,wsize=1500,rsize=1500\0"				\
	"rotation=0\0" \
	"autoboot=run _autoboot\0_autoboot=echo \"\n== Checking mmc1 for alternate boot script " CONFIG_MMC_BOOTSCRIPT_NAME " ==\";" \
	"         if mmc init; then \n" \
	"             if run _loadbootscript; then \n" \
	"                 echo \"\"; \n" \
	"                 echo \"== Found script on mmc 1, starting ==\"; \n" \
	"                 run bootscript; \n" \
	"             else \n" \
	"                 echo \"\"; \n" \
	"                 echo \"== Script not found on mmc 1, proceeding with defaultboot ==\"; \n" \
	"                 run _defaultboot;\n" \
	"             fi; \n" \
	"         else run _defaultboot; fi\0" \
	"_loadbootscript=fatload mmc 1 $mmc_bootscript_addr " CONFIG_MMC_BOOTSCRIPT_NAME "\0" \
	"bootscript=source ${mmc_bootscript_addr}\0" \
	"_vrfb_arg=if itest ${rotation} -ne 0; then \n" \
	"              setenv bootargs ${bootargs} omapfb.vrfb=y omapfb.rotate=${rotation}; \n" \
	"          fi\0" \
	"dump_bootargs=echo \"\"; echo \"== Kernel bootargs ==\"; echo $bootargs; echo \"\"; \0" \
	"_dump_boot_sources=echo \"kernel_location: $kernel_location, " \
			  "rootfs_location: $rootfs_location, " \
			  "rootfs_type: $rootfs_type\"; " \
			  "echo \"\"; " \
	"\0" \
	\
	\
	"check_rebootreason=mw ${loadaddr} 0x424d0062;if cmp ${loadaddr} 0x48002914 1;then mw 0x48002914 0;fastboot;fi;mw 0x48002914 0\0" \
	/* load_kernel_* targets */ \
	"_load_kernel_ram=if true;then;\n" \
	"                  echo \"== kernel located at $Loadaddr ==\n\"\n" \
	"                  setenv bootm_arg1 ${loadaddr}\n" \
	"                fi\0" \
	"_load_kernel_nand=if true;then;\n" \
	"                   echo \"== Loading kernel from nand to $loadaddr ==\"; \n" \
	"                   nand read.i $loadaddr $kernel_partition; \n" \
	"                   echo \"\"; \n" \
	"                   setenv bootm_arg1 ${loadaddr};\n" \
	"                 fi\0" \
	"_load_kernel_nand-part=if true;then;\n" \
	"                        echo \"== Loading kernel from nand partition $kernel_partition to $loadaddr ==\"; \n" \
	"                        nboot $loadaddr $kernel_partition; \n" \
	"                        echo \"\"; \n" \
	"                        setenv bootm_arg1 ${loadaddr};\n" \
	"                      fi\0" \
	"_load_kernel_mmc=if true;then;\n" \
	"                     echo \"== Loading kernel file $kernelimage to $loadaddr ==\"; \n" \
	"                     mmc init; \n" \
	"                     fatload mmc 1 $loadaddr $kernelimage; \n" \
	"                     echo \"\"; \n" \
	"                     setenv bootm_arg1 ${loadaddr};\n" \
	"                 fi\0" \
	"_load_kernel_tftp=if true;then;\n" \
	"                     echo \"== Loading kernel file $tftpdir$kernelimage to $loadaddr ==\"; \n" \
	"                     tftpboot $loadaddr $tftpdir$kernelimage; \n" \
	"                     echo \"\"; \n" \
	"                     setenv bootm_arg1 ${loadaddr};\n" \
	"                 fi\0" \
	"_load_kernel=setenv bootm_arg1;if run _load_kernel_$kernel_location; then;else \n" \
	"                echo \"\033[31m== kernel_location must be set to ram, nand, mmc, or tftp!! ==\033[0m\"; \n" \
	"                echo \"\"; \n" \
	"            fi\n\0" \
	\
	/* load_rootfs_* targets */ \
	"_load_rootfs_ram=if true;then;\n" \
	"                    echo \"== rootfs located at $ramdiskaddr ==\"; \n" \
	"                    echo \"\"; \n" \
	"                    setenv bootm_arg2 ${ramdiskaddr}; \n" \
	"                fi\0" \
	"_load_rootfs_tftp=if true;then;\n" \
	"                     echo \"== Loading rootfs file $tftpdir$ramdiskimage to $ramdiskaddr ==\"; \n" \
	"                     tftpboot $ramdiskaddr $tftpdir$ramdiskimage;\n" \
	"                     echo \"\"; \n" \
	"                     setenv bootm_arg2 ${ramdiskaddr}; \n" \
	"                 fi\0" \
	"_load_rootfs_/dev=if true;then;\n" \
	"                     echo \"== rootfs located in $rootfs_device ==\"; \n" \
	"                     echo \"\"; \n" \
	"                     setenv bootargs ${bootargs} root=${rootfs_device}; \n" \
	"                 fi\0" \
	"_load_rootfs_nfs=if true;then;\n" \
	"                    echo \"== rootfs located at $nfsrootpath on server $serverip ==\"; \n" \
	"                    echo \"\"; \n" \
	"                    setenv bootargs ${bootargs} root=/dev/nfs; \n" \
	"                fi\0" \
	"_load_rootfs_mmc=if true;then;\n" \
	"                    echo \"== Loading rootfs file $ramdiskimage to $ramdiskaddr ==\"; \n" \
	"                    fatload mmc 1 ${ramdiskaddr} ${ramdiskimage}; \n"\
	"                    setenv bootm_arg2 ${ramdiskaddr}; \n" \
	"                fi\0" \
	"_load_rootfs_nand=if true;then;\n" \
	"                     echo \"== Loading rootfs from nand to $ramdiskaddr ==\"; \n" \
	"                     nand read.i $ramdiskaddr $ramdisk_partition; \n" \
	"                     setenv bootm_arg2 ${ramdiskaddr}; \n" \
	"                 fi\0" \
	"_load_rootfs_nand-part=if true;then;\n" \
	"                          echo \"== Loading rootfs from nand partition $ramdisk_partition to $ramdiskaddr ==\"; \n" \
	"                          nand read.i $ramdiskaddr $ramdisk_partition; \n" \
	"                          setenv bootm_arg2 ${ramdiskaddr}; \n" \
	"                      fi\0" \
	"_load_rootfs=setenv bootm_arg2;if run _load_rootfs_$rootfs_location; then;else \n" \
	"                echo \"\033[31m== rootfs_location must be set to ram, tftp, /dev, nfs, mmc, nand-part or nand!! == \033[0m\"; \n" \
	"                echo \"\"; \n" \
	"            fi\n\0" \
	\
	/* set_rootfs_type_* targets */ \
	"_set_rootfs_type_ramdisk=setenv bootargs ${bootargs} root=/dev/ram rw ramdisk_size=${ramdisksize}\0" \
	"_set_rootfs_type_jffs=setenv bootargs ${bootargs} rw rootfstype=jffs2\0" \
	"_set_rootfs_type_yaffs=setenv bootargs ${bootargs} rw rootfstype=yaffs2\0" \
	"_set_rootfs_type_ext3=setenv bootargs ${bootargs} rw rootfstype=ext3 rootwait\0" \
	"_set_rootfs_type_nfs=setenv bootargs ${bootargs} rw nfsroot=${serverip}:${nfsrootpath}${nfsoptions} ip=dhcp\0" \
	"_set_rootfs_type=if run _set_rootfs_type_${rootfs_type}; then;else \n" \
	"                    echo \"\033[31m== $rootfs_type must be set to ramdisk, jffs, yaffs, ext3, or nfs ==\033[0m\"; \n" \
	"                    echo \"\"; \n" \
	"                fi\0" \
	\
	/* Other misc. scripts */ \
	"_addmtdparts=setenv bootargs ${bootargs} ${mtdparts} \0" \
	"_common_bootargs=if true;then;\n" \
	"                    setenv bootargs ${bootargs} display=${display} ${otherbootargs};\n" \
	"                    run androidbootargs;\n" \
	"                    run _addmtdparts;\n" \
	"                    run _vrfb_arg;\n" \
	"                fi\0" \
	"_dump_run_bootm=if true;then\n" \
	"                   echo \"bootm $bootm_arg1 $bootm_arg2\";\n" \
	"                   echo \"\";\n" \
	"                   bootm $bootm_arg1 $bootm_arg2\n" \
	"               fi\0" \
	"defaultboot=run _defaultboot\0_defaultboot=if true;then\n" \
	"                 run _dump_boot_sources;\n" \
	"                 run setconsole; setenv bootargs console=${console};\n" \
	"                 run _common_bootargs;\n" \
	"                 run _load_kernel;\n" \
	"                 run _load_rootfs;\n" \
	"                 run _set_rootfs_type;\n" \
	"                 run dump_bootargs;\n" \
	"                 run _dump_run_bootm;\n" \
	"             fi\0" \
	"nfsboot=if true;then\n" \
	"            setenv kernel_location tftp\n" \
	"            setenv rootfs_location nfs\n" \
	"            setenv rootfs_type nfs\n" \
	"            run defaultboot\n" \
	"        fi\0" \
	"ramboot=if true;then\n" \
	"            setenv kernel_location tftp\n" \
	"            setenv rootfs_location tftp\n" \
	"            setenv rootfs_type ramdisk\n" \
	"            run defaultboot\n" \
	"        fi\0" \
	"_checkerror=if test $error = '';\n" \
	"           then\n" \
	"               echo \033[31m${error}\033[0m\n" \
	"               echo_lcd /pAA/k${error}\n" \
	"           else\n" \
	"               echo \033[1mDone!\033[m\n" \
	"               echo_lcd /pAA/kCompleted burning sucessfully./n\n" \
	"               echo_lcd /kIt is safe to remove the SD card/n\n" \
	"               echo_lcd /kand restart the devkit./n\n" \
	"           fi" \
	"\0" \
	"_initmmc=if test $error = '';then;else\n" \
	"            if mmc init;then;else;\n" \
	"                setenv error \"Failed to initialize MMC\"\n" \
	"            fi\n" \
	"        fi" \
	"\0" \
	"_checkmmcfile=if test $error = '';then;else\n" \
	"                 if fatload mmc 1 ${loadaddr} ${arg_filename} 1;then;else;\n" \
	"                     setenv error \"Unable to load ${arg_filename}\";\n" \
	"                 fi\n" \
	"             fi" \
	"\0" \
	"_burnfile=if test $error = '';then;else\n" \
	"                echo \"\033[1m== Loading ${arg_filename} ==\033[0m\"\n" \
	"                echo_lcd /pAA/kPartition ${arg_partition}:\n" \
	"                echo_lcd /pBA/kLoading ${arg_filename}/aC;lcd_percent \"/gC/k /P%...\"\n" \
	"                if ${arg_loadcmd} ${loadaddr} ${arg_filename};then;\n" \
	"                     echo \"\033[1m== Burning ${arg_partition} ==\033[0m\"\n" \
	"                     lcd_percent \"/pBA/kErasing Partition /P%...\"\n" \
	"                     nand erase.part ${arg_partition}\n" \
	"                     echo_lcd /pBA/kWriting ${arg_filename} to Partition/aC;lcd_percent \"/gC/k /P%...\"\n" \
	"                     nand write.auto ${loadaddr} ${arg_partition} ${filesize}\n" \
	"                     lcd_percent \"\"\n" \
	"                     echo_lcd /pAA/k/pAB/k\n" \
	"                else\n" \
	"                     setenv error \"Unable to load ${arg_filename}\"\n" \
	"                fi\n" \
	"            fi" \
	"\0" \
	"_burncommon=if test $error = '';then;else\n" \
	"                   arg_filename=${xloadimage};run _checkmmcfile;\n" \
	"                   arg_filename=${ubootimage};run _checkmmcfile;\n" \
	"                   arg_filename=${kernelimage};run _checkmmcfile;\n" \
	"                   arg_filename=${xloadimage};arg_partition=${xloader_partition};run _burnfile;\n" \
	"                   arg_filename=${ubootimage};arg_partition=${uboot_partition};run _burnfile;\n" \
	"                   arg_filename=${kernelimage};arg_partition=${kernel_partition};run _burnfile;\n" \
	"            fi" \
	"\0" \
	"makenandboot=run _makenandboot\0_makenandboot=if true;then;\n" \
	"                 setenv error;\n" \
	"                 run _initmmc;\n" \
	"                 arg_filename=${ramdiskimage};run _checkmmcfile;\n" \
	"                 run _burncommon;\n" \
	"                 arg_filename=${ramdiskimage};arg_partition=${ramdisk_partition};run _burnfile;\n" \
	"                 if test $error = '';then;else\n" \
	"                     setenv kernel_location nand-part\n" \
	"                     setenv rootfs_location nand-part\n" \
	"                     setenv rootfs_type ramdisk\n" \
	"                     saveenv\n" \
	"                 fi\n" \
	"                 run _checkerror;\n" \
	"             fi" \
	"\0" \
	"makeyaffsboot=run _makeyaffsboot\0_makeyaffsboot=if true;then;\n" \
	"                 setenv error;\n" \
	"                 run _initmmc;\n" \
	"                 arg_filename=${yaffsimage};run _checkmmcfile;\n" \
	"                 run _burncommon;\n" \
	"                 arg_filename=${yaffsimage};arg_partition=${yaffs_partition};run _burnfile\n" \
	"                 if test $error = '';then;else\n" \
	"                     setenv kernel_location nand-part\n" \
	"                     setenv rootfs_location /dev\n" \
	"                     setenv rootfs_type yaffs\n" \
	"                     setenv rootfs_device /dev/mtdblock5\n" \
	"                     saveenv\n" \
	"                 fi\n" \
	"                 run _checkerror;\n" \
	"             fi" \
	"\0" \
	"makeandroidboot=run _makeandroidboot\0_makeandroidboot=if true;then;\n" \
	"                 setenv error;\n" \
	"                 run _initmmc;\n" \
	"                 arg_filename=system.img;run _checkmmcfile;\n" \
	"                 arg_filename=userdata.img;run _checkmmcfile;\n" \
	"                 run _burncommon;\n" \
	"                 arg_filename=system.img;arg_partition=system;run _burnfile\n" \
	"                 arg_filename=userdata.img;arg_partition=userdata;run _burnfile\n" \
	"                 if test $error = '';then;else\n" \
	"                     nand erase.part cache\n" \
	"                     setenv kernel_location nand-part\n" \
	"                     setenv rootfs_location /dev\n" \
	"                     setenv rootfs_type yaffs\n" \
	"                     setenv rootfs_device /dev/mtdblock5\n" \
	"                     saveenv\n" \
	"                 fi\n" \
	"                 run _checkerror;\n" \
	"             fi" \
	"\0"


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
#define CONFIG_SYS_FLASH_PRESENCE omap3logic_nor_exists
#ifndef __ASSEMBLY__
extern int omap3logic_nor_exists;
#endif
#define CONFIG_SYS_FLASH_BASE		0x10000000 /* FLASH base address */
#define CONFIG_SYS_FLASH_SIZE			8 /* 8MB */
#define CONFIG_SYS_MAX_FLASH_SECT		(64+8) /* 8MB/128K */
#define CONFIG_SYS_MAX_FLASH_BANKS		1
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE
#undef	CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_CFI		/* use the Common Flash Interface */
#define CONFIG_FLASH_CFI_DRIVER	/* use the CFI driver */
#define CONFIG_FLASH_CFI_MTD	/* use the CFI MTD interface */


/* **** PISMO SUPPORT *** */

/* Configure the PISMO */
#define PISMO1_NAND_SIZE		GPMC_SIZE_128M

#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 2 sectors */

/* Monitor at start of flash */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_FLASH_BASE

#define SMNAND_ENV_OFFSET		0x220000 /* environment starts here */

#if defined(CONFIG_CMD_NAND)
# define CONFIG_SYS_NAND_QUIET_TEST	1
# define CONFIG_NAND_OMAP_GPMC
# define CONFIG_MTD_NAND_BCH
# define CONFIG_MTD_NAND_ECC_BCH
# define CONFIG_BCH
# define GPMC_NAND_ECC_LP_x16_LAYOUT	1
# define CONFIG_ENV_IS_IN_NAND
# ifdef FORCED_ENVIRONMENT
#  define CONFIG_ENV_FORCED
# endif
# define CONFIG_ENV_OFFSET		SMNAND_ENV_OFFSET
#endif

#define CONFIG_ENV_ADDR			CONFIG_ENV_OFFSET
#define CONFIG_ENV_RANGE		(3 * CONFIG_ENV_SIZE)

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

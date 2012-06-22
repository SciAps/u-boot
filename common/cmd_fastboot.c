/*
 * Copyright (C) 2010 Texas Instruments
 *
 * Author : Mohammed Afzal M A <afzal@ti.com>
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
 *
 *
 * Fastboot is implemented using gadget stack, many of the ideas are
 * derived from fastboot implemented in OmapZoom by
 * Tom Rix <Tom.Rix@windriver.com>, and portion of the code has been
 * ported from OmapZoom.
 *
 * Part of OmapZoom was copied from Android project, Android source
 * (legacy bootloader) was used indirectly here by using OmapZoom.
 *
 * This is Android's Copyright:
 *
 * Copyright (C) 2008 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <common.h>
#include <command.h>
#include <fastboot.h>
#include <asm/sizes.h>
#include <jffs2/load_kernel.h>

#define	ERR
#define	WARN
#define	INFO
//#define DEBUG

void output_lcd_string(char *p);

#ifdef DEBUG
#define FBTDBG(fmt,args...)\
        printf("DEBUG: [%s]: %d: \n"fmt, __FUNCTION__, __LINE__,##args)
#else
#define FBTDBG(fmt,args...) do{}while(0)
#endif

#ifdef INFO
#define FBTINFO(fmt,args...)\
        printf("INFO: [%s]: "fmt, __FUNCTION__, ##args)
#else
#define FBTINFO(fmt,args...) do{}while(0)
#endif

#ifdef WARN
#define FBTWARN(fmt,args...)\
        printf("WARNING: [%s]: "fmt, __FUNCTION__, ##args)
#else
#define FBTWARN(fmt,args...) do{}while(0)
#endif

#ifdef ERR
#define FBTERR(fmt,args...)\
        printf("ERROR: [%s]: "fmt, __FUNCTION__, ##args)
#else
#define FBTERR(fmt,args...) do{}while(0)
#endif

#ifdef	FASTBOOT_PORT_OMAPZOOM_NAND_FLASHING
#include <nand.h>
#include <environment.h>
#endif

/* USB specific */

#include <usb_defs.h>

#if defined(CONFIG_PPC)
#include <usb/mpc8xx_udc.h>
#elif defined(CONFIG_OMAP1510)
#include <usb/omap1510_udc.h>
#elif defined(CONFIG_MUSB_UDC)
#include <usb/musb_udc.h>
#elif defined(CONFIG_PXA27X)
#include <usb/pxa27x_udc.h>
#elif defined(CONFIG_SPEAR3XX) || defined(CONFIG_SPEAR600)
#include <usb/spr_udc.h>
#endif

#define STR_LANG		0x00
#define STR_MANUFACTURER	0x01
#define STR_PRODUCT		0x02
#define STR_SERIAL		0x03
#define STR_CONFIGURATION	0x04
#define STR_INTERFACE		0x05
#define STR_COUNT		0x06

#define CONFIG_USBD_CONFIGURATION_STR	"Android Fastboot Configuration"
#define CONFIG_USBD_INTERFACE_STR	"Android Fastboot Interface"

#define USBFBT_BCD_DEVICE	0x00
#define	USBFBT_MAXPOWER		0x32

#define	NUM_CONFIGS	1
#define	NUM_INTERFACES	1
#define	NUM_ENDPOINTS	2

#define	RX_EP_INDEX	1
#define	TX_EP_INDEX	2

struct _fbt_config_desc {
	struct usb_configuration_descriptor configuration_desc;
	struct usb_interface_descriptor interface_desc;
	struct usb_endpoint_descriptor endpoint_desc[NUM_ENDPOINTS];
};

static int fbt_handle_response(void);

/* defined and used by gadget/ep0.c */
extern struct usb_string_descriptor **usb_strings;

static struct cmd_fastboot_interface priv;

/* USB Descriptor Strings */
static char serial_number[28]; /* what should be the length ?, 28 ? */
static char product_name[32];
static u8 wstr_lang[4] = {4,USB_DT_STRING,0x9,0x4};
static u8 wstr_manufacturer[2 + 2*(sizeof(CONFIG_USBD_MANUFACTURER)-1)];
static u8 wstr_product[2 + 2*(sizeof(product_name) - 1)];
static u8 wstr_serial[2 + 2*(sizeof(serial_number) - 1)];
static u8 wstr_configuration[2 + 2*(sizeof(CONFIG_USBD_CONFIGURATION_STR)-1)];
static u8 wstr_interface[2 + 2*(sizeof(CONFIG_USBD_INTERFACE_STR)-1)];

/* USB descriptors */
static struct usb_device_descriptor device_descriptor = {
	.bLength = sizeof(struct usb_device_descriptor),
	.bDescriptorType =	USB_DT_DEVICE,
	.bcdUSB =		cpu_to_le16(USB_BCD_VERSION),
	.bDeviceClass =		0xFF,
	.bDeviceSubClass =	0x00,
	.bDeviceProtocol =	0x00,
	.bMaxPacketSize0 =	EP0_MAX_PACKET_SIZE,
	.idVendor =		cpu_to_le16(CONFIG_USBD_VENDORID),
	.bcdDevice =		cpu_to_le16(USBFBT_BCD_DEVICE),
	.iManufacturer =	STR_MANUFACTURER,
	.iProduct =		STR_PRODUCT,
	.iSerialNumber =	STR_SERIAL,
	.bNumConfigurations =	NUM_CONFIGS
};

static struct _fbt_config_desc fbt_config_desc = {
	.configuration_desc = {
		.bLength = sizeof(struct usb_configuration_descriptor),
		.bDescriptorType = USB_DT_CONFIG,
		.wTotalLength =	cpu_to_le16(sizeof(struct _fbt_config_desc)),
		.bNumInterfaces = NUM_INTERFACES,
		.bConfigurationValue = 1,
		.iConfiguration = STR_CONFIGURATION,
		.bmAttributes =	BMATTRIBUTE_SELF_POWERED | BMATTRIBUTE_RESERVED,
		.bMaxPower = USBFBT_MAXPOWER,
	},
	.interface_desc = {
		.bLength  = sizeof(struct usb_interface_descriptor),
		.bDescriptorType = USB_DT_INTERFACE,
		.bInterfaceNumber = 0,
		.bAlternateSetting = 0,
		.bNumEndpoints = 0x2,
		.bInterfaceClass = FASTBOOT_INTERFACE_CLASS,
		.bInterfaceSubClass = FASTBOOT_INTERFACE_SUB_CLASS,
		.bInterfaceProtocol = FASTBOOT_INTERFACE_PROTOCOL,
		.iInterface = STR_INTERFACE,
	},
	.endpoint_desc = {
		{
			.bLength = sizeof(struct usb_endpoint_descriptor),
			.bDescriptorType = USB_DT_ENDPOINT,
			/* XXX: can't the address start from 0x1, currently
				seeing problem with "epinfo" */
			.bEndpointAddress = RX_EP_INDEX | USB_DIR_OUT,
			.bmAttributes =	USB_ENDPOINT_XFER_BULK,
			.bInterval = 0xFF,
		},
		{
			.bLength = sizeof(struct usb_endpoint_descriptor),
			.bDescriptorType = USB_DT_ENDPOINT,
			/* XXX: can't the address start from 0x1, currently
				seeing problem with "epinfo" */
			.bEndpointAddress = TX_EP_INDEX | USB_DIR_IN,
			.bmAttributes = USB_ENDPOINT_XFER_BULK,
			.bInterval = 0xFF,
		},
	},
};

static struct usb_interface_descriptor interface_descriptors[NUM_INTERFACES];
static struct usb_endpoint_descriptor *ep_descriptor_ptrs[NUM_ENDPOINTS];

static struct usb_string_descriptor *fbt_string_table[STR_COUNT];
static struct usb_device_instance device_instance[1];
static struct usb_bus_instance bus_instance[1];
static struct usb_configuration_instance config_instance[NUM_CONFIGS];
static struct usb_interface_instance interface_instance[NUM_INTERFACES];
static struct usb_alternate_instance alternate_instance[NUM_INTERFACES];
static struct usb_endpoint_instance endpoint_instance[NUM_ENDPOINTS + 1];

/* FASBOOT specific */

#define	GETVARLEN	30
#define	SECURE		"no"

/* U-boot version */
extern char version_string[];

static struct cmd_fastboot_interface priv =
{
        .transfer_buffer       = (void *)CONFIG_FASTBOOT_TRANSFER_BUFFER,
        .transfer_buffer_size  = CONFIG_FASTBOOT_TRANSFER_BUFFER_SIZE,
};

static int fbt_init_endpoints (void);

#ifdef	FASTBOOT_PORT_OMAPZOOM_NAND_FLASHING
/* Use do_bootm and do_go for fastboot's 'boot' command */
extern int do_go (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
/* Use do_env_set and do_env_save to permenantly save data */
extern int do_env_save (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
extern int do_env_set ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
extern int do_nand(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[]);
extern int do_flerase (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
#endif /* FASTBOOT_PORT_OMAPZOOM_NAND_FLASHING */

static int fbt_lastidle;

static void fbt_pokeidle(void)
{
	fbt_lastidle = get_ticks();
}

static int fbt_getidle(void)
{
	return get_ticks() - fbt_lastidle;
}

/* USB specific */

/* utility function for converting char* to wide string used by USB */
static void str2wide (char *str, u16 * wide)
{
	int i;
	for (i = 0; i < strlen (str) && str[i]; i++){
		#if defined(__LITTLE_ENDIAN)
			wide[i] = (u16) str[i];
		#elif defined(__BIG_ENDIAN)
			wide[i] = ((u16)(str[i])<<8);
		#else
			#error "__LITTLE_ENDIAN or __BIG_ENDIAN undefined"
		#endif
	}
}

/* fastboot_init has to be called before this fn to get correct serial string */
static int fbt_init_strings(void)
{
	struct usb_string_descriptor *string;
	const char *model = getenv("logic_model");
	fbt_string_table[STR_LANG] =
		(struct usb_string_descriptor*)wstr_lang;

	string = (struct usb_string_descriptor *) wstr_manufacturer;
	string->bLength = sizeof(wstr_manufacturer);
	string->bDescriptorType = USB_DT_STRING;
	str2wide (CONFIG_USBD_MANUFACTURER, string->wData);
	fbt_string_table[STR_MANUFACTURER] = string;

	string = (struct usb_string_descriptor *) wstr_product;
	string->bLength = sizeof(wstr_product);
	string->bDescriptorType = USB_DT_STRING;

	if(model)
	{
		strncpy (product_name, model, sizeof(product_name));
	} else {
		// Otherwise, populate generically.
		strncpy (product_name, "LogicPD SOM", sizeof(product_name));
	}
	product_name[sizeof(product_name)-1] = '\0';
	str2wide (product_name, string->wData);
	fbt_string_table[STR_PRODUCT] = string;

	string = (struct usb_string_descriptor *) wstr_serial;
	string->bLength = sizeof(wstr_serial);
	string->bDescriptorType = USB_DT_STRING;
	str2wide (serial_number, string->wData);
	fbt_string_table[STR_SERIAL] = string;

	string = (struct usb_string_descriptor *) wstr_configuration;
	string->bLength = sizeof(wstr_configuration);
	string->bDescriptorType = USB_DT_STRING;
	str2wide (CONFIG_USBD_CONFIGURATION_STR, string->wData);
	fbt_string_table[STR_CONFIGURATION] = string;

	string = (struct usb_string_descriptor *) wstr_interface;
	string->bLength = sizeof(wstr_interface);
	string->bDescriptorType = USB_DT_STRING;
	str2wide (CONFIG_USBD_INTERFACE_STR, string->wData);
	fbt_string_table[STR_INTERFACE] = string;

	/* Now, initialize the string table for ep0 handling */
	usb_strings = fbt_string_table;

	return 0;
}

static void fbt_event_handler (struct usb_device_instance *device,
				  usb_device_event_t event, int data)
{
	switch (event) {
	case DEVICE_RESET:
	case DEVICE_BUS_INACTIVE:
		output_lcd_string("/pAA/kFastboot is waiting for connection from PC/pBA/k/pCA/k");

		priv.configured = 0;

		// Clear active transfers and flags.
		priv.flag = 0;
		priv.d_size = 0;
		priv.d_bytes = 0;
		priv.u_size = 0;
		priv.u_bytes = 0;
		break;
	case DEVICE_CONFIGURED:
		priv.configured = 1;
		break;

	case DEVICE_ADDRESS_ASSIGNED:
		output_lcd_string("/pAA/kFastboot is connected to PC");

		fbt_init_endpoints ();

	default:
		break;
	}
}

/* fastboot_init has to be called before this fn to get correct serial string */
static int fbt_init_instances(void)
{
	int i;

	/* initialize device instance */
	memset (device_instance, 0, sizeof (struct usb_device_instance));
	device_instance->device_state = STATE_INIT;
	device_instance->device_descriptor = &device_descriptor;
	device_instance->event = fbt_event_handler;
	device_instance->cdc_recv_setup = NULL;
	device_instance->bus = bus_instance;
	device_instance->configurations = NUM_CONFIGS;
	device_instance->configuration_instance_array = config_instance;

	/* XXX: what is this bus instance for ?, can't it be removed by moving
	    endpoint_array and serial_number_str is moved to device instance */
	/* initialize bus instance */
	memset (bus_instance, 0, sizeof (struct usb_bus_instance));
	bus_instance->device = device_instance;
	bus_instance->endpoint_array = endpoint_instance;
	/* XXX: what is the relevance of max_endpoints & maxpacketsize ? */
	bus_instance->max_endpoints = 1;
	bus_instance->maxpacketsize = 64;
	bus_instance->serial_number_str = serial_number;

	/* configuration instance */
	memset (config_instance, 0,
		sizeof (struct usb_configuration_instance));
	config_instance->interfaces = NUM_INTERFACES;
	config_instance->configuration_descriptor =
		(struct usb_configuration_descriptor *)&fbt_config_desc;
	config_instance->interface_instance_array = interface_instance;

	/* XXX: is alternate instance required in case of no alternate ? */
	/* interface instance */
	memset (interface_instance, 0,
		sizeof (struct usb_interface_instance));
	interface_instance->alternates = 1;
	interface_instance->alternates_instance_array = alternate_instance;

	/* alternates instance */
	memset (alternate_instance, 0,
		sizeof (struct usb_alternate_instance));
	alternate_instance->interface_descriptor = interface_descriptors;
	alternate_instance->endpoints = NUM_ENDPOINTS;
	alternate_instance->endpoints_descriptor_array = ep_descriptor_ptrs;

	/* endpoint instances */
	memset (&endpoint_instance[0], 0,
		sizeof (struct usb_endpoint_instance));
	endpoint_instance[0].endpoint_address = 0;
	endpoint_instance[0].rcv_packetSize = EP0_MAX_PACKET_SIZE;
	endpoint_instance[0].rcv_attributes = USB_ENDPOINT_XFER_CONTROL;
	endpoint_instance[0].tx_packetSize = EP0_MAX_PACKET_SIZE;
	endpoint_instance[0].tx_attributes = USB_ENDPOINT_XFER_CONTROL;
	/* XXX: following statement to done along with other endpoints
		at another place ? */
	udc_setup_ep (device_instance, 0, &endpoint_instance[0]);

	for (i = 1; i <= NUM_ENDPOINTS; i++) {
		memset (&endpoint_instance[i], 0,
			sizeof (struct usb_endpoint_instance));

		endpoint_instance[i].endpoint_address =
			ep_descriptor_ptrs[i - 1]->bEndpointAddress;

		endpoint_instance[i].rcv_attributes =
			ep_descriptor_ptrs[i - 1]->bmAttributes;

		endpoint_instance[i].rcv_packetSize =
			le16_to_cpu(ep_descriptor_ptrs[i - 1]->wMaxPacketSize);

		endpoint_instance[i].tx_attributes =
			ep_descriptor_ptrs[i - 1]->bmAttributes;

		endpoint_instance[i].tx_packetSize =
			le16_to_cpu(ep_descriptor_ptrs[i - 1]->wMaxPacketSize);

		endpoint_instance[i].tx_attributes =
			ep_descriptor_ptrs[i - 1]->bmAttributes;

		urb_link_init (&endpoint_instance[i].rcv);
		urb_link_init (&endpoint_instance[i].rdy);
		urb_link_init (&endpoint_instance[i].tx);
		urb_link_init (&endpoint_instance[i].done);

		if (endpoint_instance[i].endpoint_address & USB_DIR_IN)
			endpoint_instance[i].tx_urb =
				usbd_alloc_urb (device_instance,
						&endpoint_instance[i]);
		else
			endpoint_instance[i].rcv_urb =
				usbd_alloc_urb (device_instance,
						&endpoint_instance[i]);
	}

	return 0;
}

/* XXX: ep_descriptor_ptrs can be removed by making better use of
	fbt_config_desc.endpoint_desc */
static int fbt_init_endpoint_ptrs(void)
{
	ep_descriptor_ptrs[0] = &fbt_config_desc.endpoint_desc[0];
	ep_descriptor_ptrs[1] = &fbt_config_desc.endpoint_desc[1];

	return 0;
}

static int fbt_init_endpoints(void)
{
	int i;

	/* XXX: should it be moved to some other function ? */
	bus_instance->max_endpoints = NUM_ENDPOINTS + 1;

	/* XXX: is this for loop required ?, yes for MUSB it is */
	for (i = 1; i <= NUM_ENDPOINTS; i++) {

		/* configure packetsize based on HS negotiation status */
		if (device_instance->speed == USB_SPEED_FULL) {
			FBTINFO("setting up FS USB device ep%x\n",
				endpoint_instance[i].endpoint_address);
			ep_descriptor_ptrs[i - 1]->wMaxPacketSize =
				CONFIG_USBD_FASTBOOT_BULK_PKTSIZE_FS;
		} else if (device_instance->speed == USB_SPEED_HIGH) {
			FBTINFO("setting up HS USB device ep%x\n",
				endpoint_instance[i].endpoint_address);
			ep_descriptor_ptrs[i - 1]->wMaxPacketSize =
				CONFIG_USBD_FASTBOOT_BULK_PKTSIZE_HS;
		}

		endpoint_instance[i].tx_packetSize =
			le16_to_cpu(ep_descriptor_ptrs[i - 1]->wMaxPacketSize);
		endpoint_instance[i].rcv_packetSize =
			le16_to_cpu(ep_descriptor_ptrs[i - 1]->wMaxPacketSize);

		udc_setup_ep (device_instance, i, &endpoint_instance[i]);

	}

	return 0;
}

static struct urb *next_urb (struct usb_device_instance *device,
			     struct usb_endpoint_instance *endpoint)
{
	struct urb *current_urb = NULL;
	int space;

	/* If there's a queue, then we should add to the last urb */
	if (!endpoint->tx_queue) {
		current_urb = endpoint->tx_urb;
	} else {
		/* Last urb from tx chain */
		current_urb =
			p2surround (struct urb, link, endpoint->tx.prev);
	}

	/* Make sure this one has enough room */
	space = current_urb->buffer_length - current_urb->actual_length;
	if (space > 0) {
		return current_urb;
	} else {		/* No space here */
		/* First look at done list */
		current_urb = first_urb_detached (&endpoint->done);
		if (!current_urb) {
			current_urb = usbd_alloc_urb (device, endpoint);
		}

		urb_append (&endpoint->tx, current_urb);
		endpoint->tx_queue++;
	}
	return current_urb;
}

/* FASBOOT specific */
static void set_env(char *var, char *val)
{
	char *setenv[4]  = { "setenv", NULL, NULL, NULL, };

	setenv[1] = var;
	setenv[2] = val;

	do_env_set(NULL, 0, 3, setenv);
}

int do_lcd_percent (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[]);

void set_lcd_percent_string(char *string)
{
	char *lcd_percent[] = { "lcd_percent", string};

	do_lcd_percent (NULL, 0, ARRAY_SIZE(lcd_percent), lcd_percent);
}

static void set_serial_number(void)
{
	char *dieid = getenv("logic_serial");
	if (dieid == NULL) {
		priv.serial_no = "00123";
	} else {
		int len;

		memset(&serial_number[0], 0, 28);
		len = strlen(dieid);
		if (len > 28)
			len = 26;

		strncpy(&serial_number[0], dieid, len);

		priv.serial_no = &serial_number[0];
	}
}

static int fbt_fastboot_init(void)
{
	priv.flag = 0;
	priv.d_size = 0;
	priv.d_bytes = 0;
	priv.u_size = 0;
	priv.u_bytes = 0;
	priv.exit = 0;

	priv.product_name = FASTBOOT_PRODUCT_NAME;
	set_serial_number();

	return 0;
}

#ifdef	FASTBOOT_PORT_OMAPZOOM_NAND_FLASHING

static int get_part_size(const char *partname, u8 *dev_type)
{
#ifdef CONFIG_CMD_MTDPARTS
	struct mtd_device *dev;
	struct part_info *part;
	u8 pnum;
	int ret;

	ret = mtdparts_init();
	if (ret)
		return ret;

	ret = find_dev_and_part(partname, &dev, &pnum, &part, 0);
	if (ret)
		return ret;

	if(dev_type)
		*dev_type = dev->id->type;

	return part->size;
#else
	return 0;
#endif
}

static int fbt_nandpart_erase(char *part)
{
	char *erase[]  = { "nand", "erase.part",  part};
	return do_nand(NULL, 0, ARRAY_SIZE(erase), erase);
}

static int fbt_nandpart_write(char *part, void *address, int size)
{
	char addr_str[32], size_str[32];
	char *write[]  = { "nand", "write.auto",  addr_str, part, size_str };

	// Prep command line arguments
	sprintf(addr_str, "0x%x", (unsigned int)address);
	sprintf(size_str, "0x%x", size);

	return do_nand(NULL, 0, ARRAY_SIZE(write), write);
}

static int fbt_handle_erase(char *cmdbuf)
{
	int status;

	if((status = fbt_nandpart_erase(cmdbuf + 6)))
		sprintf(priv.response, "FAILfailed to erase partition");
	else
		sprintf(priv.response, "OKAY");
	return status;
}

static int fbt_handle_flash(char *cmdbuf)
{
	int status = 0;

	if (priv.d_bytes) {
		u8 dev_type = 0;
		char *part = cmdbuf + 6;
		int size = get_part_size(part, &dev_type);

		if(size < 0)
		{
			sprintf(priv.response, "FAILpartition does not exist");
			return status;
		} else if(priv.d_bytes > size)
		{
			sprintf(priv.response, "FAILimage too large for partition");
			return status;
		}

		FBTINFO("Flashing '%s'\n", part);
		switch(dev_type)
		{
			case MTD_DEV_TYPE_NOR:
				FBTINFO("Fastboot currently doesn't support NOR\n");
				sprintf(priv.response, "FAILUnsupported partition type (nor)");
				return 1;
				break;
			case MTD_DEV_TYPE_NAND:
				if(fbt_nandpart_erase(part) ||
				   fbt_nandpart_write(part, priv.transfer_buffer, priv.d_bytes))
				{
					FBTINFO("Failed flashing '%s'\n", part);
					sprintf(priv.response, "FAILFailed erasing or writing partition");
					return 1;
				}
				break;
			case MTD_DEV_TYPE_ONENAND:
				FBTINFO("Fastboot currently doesn't support ONENAND.\n");
				sprintf(priv.response, "FAILUnsupported partition type (onenand)");
				return 1;
		}
		FBTINFO("Done flashing '%s'\n", part);
		sprintf(priv.response, "OKAY");
	} else {
		sprintf(priv.response, "FAILno image downloaded");
	}

	return status;
}
#endif /* FASTBOOT_PORT_OMAPZOOM_NAND_FLASHING */


static int fbt_handle_getvar(char *cmdbuf)
{
	strcpy(priv.response, "OKAY");
	if(!strcmp(cmdbuf + strlen("getvar:"), "version")) {
		FBTDBG("getvar version\n");
		strcpy(priv.response + 4, FASTBOOT_VERSION);
	} else if(!strcmp(cmdbuf + strlen("getvar:"), "version-bootloader")) {
		strncpy(priv.response + 4, version_string,
			min(strlen(version_string), GETVARLEN));
	} else if(!strcmp(cmdbuf + strlen("getvar:"), "secure")) {
		strcpy(priv.response + 4, SECURE);
	} else if(!strcmp(cmdbuf + strlen("getvar:"), "product")) {
		if (priv.product_name)
			strcpy(priv.response + 4, priv.product_name);
	} else if(!strcmp(cmdbuf + strlen("getvar:"), "serialno")) {
		if (priv.serial_no)
			strcpy(priv.response + 4, priv.serial_no);
	}
	return 0;
}

static int fbt_handle_reboot(char *cmdbuf)
{
	strcpy(priv.response,"OKAY");
	priv.flag |= FASTBOOT_FLAG_RESPONSE;
	fbt_handle_response();
	udelay (1000000); /* 1 sec */

	do_reset (NULL, 0, 0, NULL);

	return 0;
}

static int fbt_handle_boot(char *cmdbuf)
{
	if ((priv.d_bytes) &&
		(CONFIG_FASTBOOT_MKBOOTIMAGE_PAGE_SIZE < priv.d_bytes)) {
		char start[32];
		char *bootm[3] = { "bootm", NULL, NULL, };
		char *go[3]    = { "go",    NULL, NULL, };

		/*
		 * Use this later to determine if a command line was passed
		 * for the kernel.
		 */
		struct fastboot_boot_img_hdr *fb_hdr =
			(struct fastboot_boot_img_hdr *) priv.transfer_buffer;

		/* Skip the mkbootimage header */
		image_header_t *hdr = (image_header_t *)
		  &priv.transfer_buffer[CONFIG_FASTBOOT_MKBOOTIMAGE_PAGE_SIZE];

		bootm[1] = go[1] = start;
		sprintf (start, "0x%x", (unsigned int)hdr);

		/* Execution should jump to kernel so send the response
		   now and wait a bit.  */
		sprintf(priv.response, "OKAY");
		priv.flag |= FASTBOOT_FLAG_RESPONSE;
		fbt_handle_response();
		output_lcd_string("/pCA/k/pBA/kFastboot is booting uploaded image");
		udelay (1000000); /* 1 sec */
		udc_disconnect();

		if (ntohl(hdr->ih_magic) == IH_MAGIC) {
			/* Looks like a kernel.. */
			FBTINFO("Booting kernel..\n");

			/*
			 * Check if the user sent a bootargs down.
			 * If not, do not override what is already there
			 */
			if (strlen ((char *) &fb_hdr->cmdline[0]))
				set_env ("bootargs", (char *) &fb_hdr->cmdline[0]);

			do_bootm (NULL, 0, 2, bootm);
		} else {
			/* Raw image, maybe another uboot */
			FBTINFO("Booting raw image..\n");

			do_go (NULL, 0, 2, go);
		}

		FBTERR("booting failed, reset the board\n");
	}
	sprintf(priv.response, "FAILinvalid boot image");

	return 0;
}

/* XXX: Replace magic number & strings with macros */
static int fbt_rx_process(unsigned char *buffer, int length)
{
	/* Generic failed response */
	strcpy(priv.response, "FAIL");

	if (!priv.d_size) {
		/* command */
		char *cmdbuf = (char *) buffer;

		FBTDBG("command\n");
		output_lcd_string("/pCA/k/pBA/kFastboot is processing command ");
		output_lcd_string(cmdbuf);

		set_lcd_percent_string("");

		if(memcmp(cmdbuf, "getvar:", 7) == 0) {
			FBTDBG("getvar\n");
			fbt_handle_getvar(cmdbuf);
		}

		if(memcmp(cmdbuf, "erase:", 6) == 0) {
			FBTDBG("erase\n");
			set_lcd_percent_string("/pCA/kErasing is /P% done");
			fbt_handle_erase(cmdbuf);
		}

		if(memcmp(cmdbuf, "flash:", 6) == 0) {
			FBTDBG("flash\n");
			set_lcd_percent_string("/pCA/kFlashing is /P% done");
			fbt_handle_flash(cmdbuf);
		}

		if((memcmp(cmdbuf, "reboot", 6) == 0) ||
			(memcmp(cmdbuf, "reboot-bootloader", 17) == 0)) {
			FBTDBG("reboot/reboot-bootloader\n");
			fbt_handle_reboot(cmdbuf);
		}

		if(memcmp(cmdbuf, "continue", 8) == 0) {
			FBTDBG("continue\n");
			strcpy(priv.response,"OKAY");
			priv.exit = 1;
		}

		if(memcmp(cmdbuf, "boot", 4) == 0) {
			FBTDBG("boot\n");
			output_lcd_string("/pAA/kFastboot is continuing boot (timeout)/pBA/k/pCA/k");
			fbt_handle_boot(cmdbuf);
		}

		if(memcmp(cmdbuf, "download:", 9) == 0) {
			FBTDBG("download\n");

			/* XXX: need any check for size & bytes ? */
			priv.d_size =
				simple_strtoul (cmdbuf + 9, NULL, 16);
			priv.d_bytes = 0;

			FBTINFO ("starting download of %d bytes\n",
				priv.d_size);

			if (priv.d_size == 0) {
				strcpy(priv.response, "FAILdata invalid size");
			} else if (priv.d_size >
					priv.transfer_buffer_size) {
				priv.d_size = 0;
				strcpy(priv.response, "FAILdata too large");
			} else {
				sprintf(priv.response, "DATA%08x", priv.d_size);
			}

			set_lcd_percent_string("/pCA/kDownload /P% done");
			lcd_percent_init(priv.d_size);
		}

		if((memcmp(cmdbuf, "upload:", 7) == 0) ||
			(memcmp(cmdbuf, "uploadraw", 10) == 0)) {
			FBTDBG("upload/uploadraw\n");

			set_lcd_percent_string("/pCA/kUpload /P% done");
			sprintf(priv.response, "FAILUpload not supported");
		}
		priv.flag |= FASTBOOT_FLAG_RESPONSE;
	} else {
		if (length) {
			unsigned int xfr_size;

			xfr_size = priv.d_size - priv.d_bytes;
			if (xfr_size > length)
				xfr_size = length;
			memcpy(priv.transfer_buffer + priv.d_bytes,
				buffer, xfr_size);
			priv.d_bytes += xfr_size;

			lcd_percent_update(priv.d_bytes);

#ifdef	INFO
			/* Inform via prompt that download is happening */
			if (! (priv.d_bytes % (16 * 0x1000)))
				printf(".");
			if (! (priv.d_bytes % (80 * 16 * 0x1000)))
				printf("\n");
#endif
			if (priv.d_bytes >= priv.d_size) {
				priv.d_size = 0;
				strcpy(priv.response, "OKAY");
				priv.flag |= FASTBOOT_FLAG_RESPONSE;
#ifdef	INFO
				printf(".\n");
#endif
				FBTINFO("downloaded %d bytes\n", priv.d_bytes);
			}
		} else
			FBTWARN("empty buffer download\n");
	}

	return 0;
}

static int fbt_handle_rx(void)
{
	struct usb_endpoint_instance *ep = &endpoint_instance[RX_EP_INDEX];

	/* XXX: Or update status field, if so,
		"usbd_rcv_complete" [gadget/core.c] also need to be modified */
	if (ep->rcv_urb->actual_length) {
		FBTDBG("rx length: %u\n", ep->rcv_urb->actual_length);
		fbt_rx_process(ep->rcv_urb->buffer, ep->rcv_urb->actual_length);
		/* Required to poison rx urb buffer as in omapzoom ?,
		    yes, as fastboot command are sent w/o NULL termination.
		    Attempt is made here to reduce poison length, may be safer
		    to posion the whole buffer, also it is assumed that at
		    the time of creation of urb it is poisoned 	*/
		memset(ep->rcv_urb->buffer, 0, ep->rcv_urb->actual_length);
		ep->rcv_urb->actual_length = 0;
	}

	return 0;
}

static int fbt_response_process(void)
{
	struct usb_endpoint_instance *ep = &endpoint_instance[TX_EP_INDEX];
	struct urb *current_urb = NULL;
	unsigned char *dest = NULL;
	int n, ret = 0;

	current_urb = next_urb (device_instance, ep);
	if (!current_urb) {
		FBTERR("%s: current_urb NULL", __func__);
		return -1;
	}

	dest = current_urb->buffer + current_urb->actual_length;
	n = MIN (64, strlen(priv.response));
	memcpy(dest, priv.response, n);
	current_urb->actual_length += n;
	FBTDBG("response urb length: %u\n", current_urb->actual_length);
	if (ep->last == 0) {
		ret = udc_endpoint_write (ep);
		return ret;
	}

	return ret;
}

static int fbt_handle_response(void)
{
	if (priv.flag & FASTBOOT_FLAG_RESPONSE) {
		if(!strncmp(priv.response, "FAIL", 4))
		{
			output_lcd_string("/pCA/kCommand failed");
		} else
		if(!strncmp(priv.response, "OKAY", 4))
		{
			output_lcd_string("/pCA/k/pBA/kFastboot is idle");
		}
		fbt_response_process();
		priv.flag &= ~FASTBOOT_FLAG_RESPONSE;
	}

	return 0;
}

static int fbt_tx_process(void)
{
	struct usb_endpoint_instance *ep = &endpoint_instance[TX_EP_INDEX];
	struct urb *current_urb = NULL;
	unsigned char *dest = NULL;
	int n = 0, ret = 0;

	current_urb = next_urb (device_instance, ep);
	if (!current_urb) {
		FBTERR("%s: current_urb NULL", __func__);
		return -1;
	}

	dest = current_urb->buffer + current_urb->actual_length;
	n = MIN (64, priv.u_size - priv.u_bytes);
	memcpy(dest, priv.transfer_buffer + priv.u_bytes, n);
	current_urb->actual_length += n;
	if (ep->last == 0) {
		ret = udc_endpoint_write (ep);
		/* XXX: "ret = n" should be done iff n bytes has been
		 * transmitted, "udc_endpoint_write" to be changed for it,
		 * now it always return 0.
		 */
		return n;
	}

	return ret;
}

static int fbt_handle_tx(void)
{
	if (priv.u_size) {
		int bytes_written = fbt_tx_process();

		if (bytes_written > 0) {
			/* XXX: is this the right way to update priv.u_bytes ?,
			 * may be "udc_endpoint_write()" can be modified to
			 * return number of bytes transmitted or error and
			 * update based on hence obtained value
			 */
			priv.u_bytes += bytes_written;

			lcd_percent_update(priv.u_bytes);
#ifdef	INFO
			/* Inform via prompt that upload is happening */
			if (! (priv.d_bytes % (16 * 0x1000)))
				printf(".");
			if (! (priv.d_bytes % (80 * 16 * 0x1000)))
				printf("\n");
#endif
			if (priv.u_bytes >= priv.u_size)
#ifdef	INFO
				printf(".\n");
#endif
				priv.u_size = priv.u_bytes = 0;
				FBTINFO("data upload finished\n");
		} else {
			FBTERR("bytes_written: %d\n", bytes_written);
			return -1;
		}

	}

	return 0;
}

/* command */
int do_fastboot(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = -1;
	int timeout = 0;

	ret = fbt_fastboot_init();
	ret = fbt_init_endpoint_ptrs();

	if ((ret = udc_init()) < 0) {
		FBTERR("%s: MUSB UDC init failure\n", __func__);
		return ret;
	}

	ret = fbt_init_strings();
	ret = fbt_init_instances();

	udc_startup_events (device_instance);
	udc_connect();

	FBTINFO("fastboot initialized\n");
	output_lcd_string("/pAA/kFastboot is waiting for connection from PC");

	if(argc > 1)
	{
		timeout = simple_strtoul(argv[1], NULL, 10) * get_tbclk();
		fbt_pokeidle();
	}

	while(1) {
		udc_irq();
		if (priv.configured) {
			fbt_pokeidle();
			fbt_handle_rx();
			fbt_handle_response();
			fbt_handle_tx();
		}
		priv.exit |= ctrlc();
		if (priv.exit) {
			output_lcd_string("/pAA/kFastboot is continuing boot/pBA/k/pCA/k");
			set_lcd_percent_string("");
			FBTINFO("fastboot end\n");
			break;
		}
		if(timeout && fbt_getidle() > timeout)
		{
			output_lcd_string("/pAA/kFastboot is continuing boot (timeout)/pBA/k/pCA/k");
			FBTINFO("Timed out\n");
			break;
		}
	}

	udc_disconnect();

	return ret;
}

U_BOOT_CMD(fastboot, 2,	1, do_fastboot,
	"fastboot [timeout in seconds] - use USB Fastboot protocol\n", NULL);



#include <common.h>
#include <errno.h>
#include <spl.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <asm/emif.h>
#include <asm/gpio.h>
#include <i2c.h>
#include <malloc.h>

#include "../common/interface.h"
#include "../common/crc-15.h"
#include "../common/id-errno.h"


typedef struct __attribute__ ((__packed__)) eeprom_header {
	u8 id[4];			/* 0x00 - 0x03 EEPROM Tag 'LpId' */
	u8 idfmtver;		/* 0x04 ID format version */
	u8 unused0;			/* 0x05	not used */
	u16 data_length;	/* 0x06 - 0x07 data length */
	u16 header_xsum;	/* 0x08 - 0x0a header crc15 checksum */
	u16 data_xsum;		/* data crc15 checksum */
} eeprom_header;

typedef struct __attribute__ ((__packed__)) eeprom {
	eeprom_header header;
	u8 data[0x400];
} eeprom;

static int isHeaderValid(eeprom_header* header)
{
	int retval;

	retval = 
		header->id[0] == 'L' &&
		header->id[1] == 'p' &&
		header->id[2] == 'I' &&
		header->id[3] == 'd' &&
		header->header_xsum == crc_15(header, 8);

	return retval;
}

static int isDataValid(eeprom* eeprom_img)
{
	int retval;

	retval = eeprom_img->header.data_xsum == crc_15(eeprom_img->data, eeprom_img->header.data_length);

	return retval;
}

#ifdef debug
#define DISP_LINE_LEN	16
void printBytes(unsigned char *cp, unsigned int nbytes)
{
	unsigned int j;
	unsigned int linebytes;
	unsigned char* linebuf;
	unsigned int addr = 0;
	do {
		linebytes = MIN(DISP_LINE_LEN, nbytes);

		printf("%04x:", addr);
		linebuf = cp;
		for (j=0; j<linebytes; j++) {
			printf(" %02x", *cp++);
			addr++;
		}
		puts ("    ");
		for (j=0; j<linebytes; j++) {
			if ((*linebuf < 0x20) || (*linebuf > 0x7e))
				puts (".");
			else
				printf("%c", *linebuf);
			linebuf++;
		}
		putc ('\n');

		nbytes -= linebytes;
	} while (nbytes > 0);
}

#endif

int read_eeprom(struct id_data *iddata)
{
	unsigned int bus;
	size_t size;
	eeprom eeprom_img;

	debug("starting to read from eeprom\n");

	/* temporally save the current bus number and set the eeprom bus number */
	bus = i2c_get_bus_num();

	i2c_set_bus_num(CONFIG_SYS_EEPROM_BUS_NUM);

	// read header
	i2c_read(CONFIG_SYS_I2C_EEPROM_ADDR, 0, CONFIG_SYS_I2C_EEPROM_ADDR_LEN, (void*)&eeprom_img, sizeof(eeprom_header));

	#ifdef DEBUG
	debug("EEPROM header:\n");
	printBytes((void*)&eeprom_img, sizeof(eeprom_header));
	#endif

	if(!isHeaderValid(&eeprom_img.header)){
		printf("incorrect header checksum");
		return ID_ENODEV;
	}

	//read the data
	printf("size of data is: %d\n", eeprom_img.header.data_length);
	size = eeprom_img.header.data_length;
	if(size > sizeof(eeprom_img.data)){
		printf("Error: data is too big!\n");
	}
	size = MIN(size, sizeof(eeprom_img.data));
	i2c_read(CONFIG_SYS_I2C_EEPROM_ADDR, sizeof(eeprom_header), CONFIG_SYS_I2C_EEPROM_ADDR_LEN, (void*)&eeprom_img.data, size);

	#ifdef DEBUG
	debug("EEPROM data:\n");
	printBytes((void*)&eeprom_img.data, size);
	#endif

	if(!isDataValid(&eeprom_img)){
		printf("incorrect data checksum");
		return ID_ENODEV;
	}

	
	
	i2c_set_bus_num(bus);

	return ID_EOK;

}
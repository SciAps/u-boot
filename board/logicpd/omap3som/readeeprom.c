

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


static struct __attribute__ ((__packed__)) eeprom_header {
	u8 id[4];			/* 0x00 - 0x03 EEPROM Tag 'LpId' */
	u8 idfmtver;		/* 0x04 ID format version */
	u8 unused0;			/* 0x05	not used */
	u16 data_length;	/* 0x06 - 0x07 data length */
	u16 xsum;			/* 0x08 - 0x09 xsum */
} header;

typedef struct __attribute__ ((__packed__)) eeprom_data {
	u16 xsum;
	u8* data;
} eeprom_data;

static int isHeaderValid()
{
	int retval;

	retval = 
		header.id[0] == 'L' &&
		header.id[1] == 'p' &&
		header.id[2] == 'I' &&
		header.id[3] == 'd' &&
		header.xsum == crc_15(&header, 8);


	return retval;
}

static int isDataValid(eeprom_data* data)
{
	int retval;

	retval = data->xsum == crc_15(data->data, header.data_length);

	return retval;
}

int read_eeprom(struct id_data *iddata)
{
	unsigned int bus;
	size_t size;
	eeprom_data* e_data;

	/* temporally save the current bus number and set the eeprom bus number */
	bus = i2c_get_bus_num();
	i2c_set_bus_num(CONFIG_SYS_EEPROM_BUS_NUM);

	// read header
	i2c_read(CONFIG_SYS_I2C_EEPROM_ADDR, 0, CONFIG_SYS_I2C_EEPROM_ADDR_LEN, (void*)&header, sizeof(header));

	if(!isHeaderValid()){
		printf("incorrect header checksum");
		return ID_ENODEV;
	}

	//read the data
	size = sizeof(u16) + header.data_length;
	e_data = malloc(size);
	i2c_read(CONFIG_SYS_I2C_EEPROM_ADDR, sizeof(header), CONFIG_SYS_I2C_EEPROM_ADDR_LEN, (void*)e_data, size);

	if(!isDataValid(e_data)){
		printf("incorrect data checksum");
		return ID_ENODEV;
	}
	


	i2c_set_bus_num(bus);

	return ID_EOK;

}
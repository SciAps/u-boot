#include <linux/stddef.h>
#include <linux/string.h>
#include "interface.h"
#include "internals.h"
#include "id-errno.h"
#include "crc-15.h"

/* struct id_data id_data; */

struct __attribute__ ((packed)) id_header { 
	unsigned char signature[4];
	unsigned char id_fmt_ver;
	unsigned char unused0;
	unsigned short data_length;
} ;

struct __attribute__ ((packed)) id_checksums { 
	unsigned short header;
	unsigned short data;
} ;

int id_startup(struct id_data *data)
{
	int i, err;
	struct id_cookie cookie;
	unsigned char byte, *p;
	char *header_tag= "LpId";
	unsigned short xsum;
	struct id_header hdr;
	struct id_checksums xsums;
	unsigned char *mem_ptr = data->mem_ptr;

	/* Clear out data->mem_ptr since we want all the fetches to come
	 * from the AT24 chip.  Once we've validated the CRCs, restore
	 * data->mem_ptr to allow id_fetch_byte to read from data->mem_ptr
	 * instead of the AT24 chip.  Should speed up accesses dramatically */
	data->mem_ptr = NULL;

	memset(&cookie, 0, sizeof(cookie));
	/* Data starts with the header, should be 'LpId' */
	for (i=0; i<4; ++i) {
		byte = id_fetch_byte(NULL, cookie.offset, &err);
		if (mem_ptr)
			mem_ptr[cookie.offset] = byte;
		hdr.signature[i] = byte;
		cookie.offset++;
		if (err != ID_EOK) {
			id_printf("%s[%u]\n", __FILE__, __LINE__);
			goto err_ret;
		}
		if (hdr.signature[i] != header_tag[i]) {
			id_printf("%s[%u]\n", __FILE__, __LINE__);
			err = ID_ENODEV;
			goto err_ret;
		}
	}

	/* First LE 8-bit value is ID format version */
	byte = id_fetch_byte(NULL, cookie.offset, &err);
	if (mem_ptr)
		mem_ptr[cookie.offset] = byte;
	hdr.id_fmt_ver = byte;
	cookie.offset++;
	
	/* Second LE 8-bit value is currently not used */
	byte = id_fetch_byte(NULL, cookie.offset, &err);
	if (mem_ptr)
		mem_ptr[cookie.offset] = byte;
	hdr.unused0 = byte;
	cookie.offset++;
	
	/* Next LE 16-bit value is length of data */
	byte = id_fetch_byte(NULL, cookie.offset, &err);
	if (mem_ptr)
		mem_ptr[cookie.offset] = byte;
	hdr.data_length = byte;
	cookie.offset++;

	byte = id_fetch_byte(NULL, cookie.offset, &err);
	if (mem_ptr)
		mem_ptr[cookie.offset] = byte;
	hdr.data_length |= byte << 8;
	cookie.offset++;
	
	/* Next LE 16-bit value is xsum of header */
	byte = id_fetch_byte(NULL, cookie.offset, &err);
	if (mem_ptr)
		mem_ptr[cookie.offset] = byte;
	xsums.header = byte;
	cookie.offset++;

	byte = id_fetch_byte(NULL, cookie.offset, &err);
	if (mem_ptr)
		mem_ptr[cookie.offset] = byte;
	xsums.header |= byte << 8;
	cookie.offset++;

	/* Checksum the header */
	xsum = 0;
	p = (unsigned char *)&hdr;
	for (i = 0; i < sizeof(hdr); ++i)
		crc_15_step(&xsum, p[i]);

	if (xsum != xsums.header) {
		id_printf("%s[%u] xsum: 0x%04x, xsums.header: 0x%04x\n", 
		        __FILE__, __LINE__, xsum, xsums.header);
		err = -ID_EL2NSYNC;
		goto err_ret;
	}

	/* Next LE 16-bit value is xsum of data */
	byte = id_fetch_byte(NULL, cookie.offset, &err);
	if (mem_ptr)
		mem_ptr[cookie.offset] = byte;
	xsums.data = byte;
	cookie.offset++;

	byte = id_fetch_byte(NULL, cookie.offset, &err);
	if (mem_ptr)
		mem_ptr[cookie.offset] = byte;
	xsums.data |= byte << 8;
	cookie.offset++;

	/* Checksum the data (next id_len bytes), must match xsums.data */
	xsum = 0;
	for (i = 0; i < hdr.data_length; ++i) {
		byte = id_fetch_byte(NULL, cookie.offset + i, &err);
		if (mem_ptr)
			mem_ptr[cookie.offset + i] = byte;
		if (err != ID_EOK) {
			id_printf("%s[%u]\n", __FILE__, __LINE__);
			goto err_ret;
		}
		crc_15_step(&xsum, byte);
	}
	if (xsum != xsums.data) {
		id_printf("%s[%u] xsum: 0x%04x, xsums.data: 0x%04x\n", 
		        __FILE__, __LINE__, xsum, xsums.data);
		err = -ID_EL2NSYNC;
		goto err_ret;
	}

	/* offset is now at the first byte of the root dictionary which
	   contains its span */
	data->root_offset = cookie.offset;
	data->root_size = extract_unsigned_pnum(&cookie, 5, &err);
	if (err != ID_EOK) {
		id_printf("%s[%u]\n", __FILE__, __LINE__);
		goto err_ret;
	}

	data->root_size += cookie.offset - data->root_offset;

#if 0
	id_printf("Data format version: %u\n", hdr.id_fmt_ver);	
#endif	

	/* Restore data->mem_ptr to allow id_fetch_byte to read
	 * from the cached data instead of the AT24 chip */
	data->mem_ptr = mem_ptr;

	return ID_EOK;

err_ret:

	/* Error return - make sure signature in SRAM is invalid */
	if (mem_ptr)
		mem_ptr[0] = 0;

	return err;
}

/*
 * Reset the cookie to cover the whole root dictionary
 */
int id_init_cookie(struct id_data *data, struct id_cookie *cookie)
{
	if (!cookie)
		return -ID_EINVAL;
	cookie->mem_ptr = data->mem_ptr;
	cookie->start_offset = data->root_offset;
	cookie->size = data->root_size;
	cookie->offset = cookie->start_offset;
	return ID_EOK;
}

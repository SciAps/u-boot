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

static int _id_startup(struct id_data *data, int first_run)
{
	int i, err;
	struct id_cookie cookie;
	unsigned char byte, *p;
	char *header_tag= "LpId";
	unsigned short xsum;
	struct id_header hdr;
	struct id_checksums xsums;
	unsigned char *mem_ptr;
	unsigned char *cache_ptr;

	if (first_run) {
		cache_ptr = data->mem_ptr;
		data->mem_ptr = NULL;
		mem_ptr = NULL;
	} else {
		cache_ptr = NULL;
		mem_ptr = data->mem_ptr;
	}
		
	id_dbg_printf("%s: first_run %d cach_ptr %p mem_ptr %p\n", __FUNCTION__, first_run, cache_ptr, mem_ptr);

	memset(&cookie, 0, sizeof(cookie));
	/* Data starts with the header, should be 'LpId' */
	for (i=0; i<4; ++i) {
		byte = id_fetch_byte(cache_ptr, cookie.offset, &err);
		if (mem_ptr)
			mem_ptr[cookie.offset] = byte;
		hdr.signature[i] = byte;
		cookie.offset++;
		if (err != ID_EOK) {
			id_dbg_printf("%s[%u]\n", __FILE__, __LINE__);
			goto err_ret;
		}
		if (hdr.signature[i] != header_tag[i]) {
			id_dbg_printf("%s[%u]\n", __FILE__, __LINE__);
			err = ID_ENODEV;
			goto err_ret;
		}
	}

	/* First LE 8-bit value is ID format version */
	byte = id_fetch_byte(cache_ptr, cookie.offset, &err);
	if (mem_ptr)
		mem_ptr[cookie.offset] = byte;
	hdr.id_fmt_ver = byte;
	cookie.offset++;
	
	/* Second LE 8-bit value is currently not used */
	byte = id_fetch_byte(cache_ptr, cookie.offset, &err);
	if (mem_ptr)
		mem_ptr[cookie.offset] = byte;
	hdr.unused0 = byte;
	cookie.offset++;
	
	/* Next LE 16-bit value is length of data */
	byte = id_fetch_byte(cache_ptr, cookie.offset, &err);
	if (mem_ptr)
		mem_ptr[cookie.offset] = byte;
	hdr.data_length = byte;
	cookie.offset++;

	byte = id_fetch_byte(cache_ptr, cookie.offset, &err);
	if (mem_ptr)
		mem_ptr[cookie.offset] = byte;
	hdr.data_length |= byte << 8;
	cookie.offset++;
	
	/* Next LE 16-bit value is xsum of header */
	byte = id_fetch_byte(cache_ptr, cookie.offset, &err);
	if (mem_ptr)
		mem_ptr[cookie.offset] = byte;
	xsums.header = byte;
	cookie.offset++;

	byte = id_fetch_byte(cache_ptr, cookie.offset, &err);
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
		id_dbg_printf("%s[%u] xsum: 0x%04x, xsums.header: 0x%04x\n", 
		        __FILE__, __LINE__, xsum, xsums.header);
		err = -ID_EL2NSYNC;
		goto err_ret;
	}

	/* Next LE 16-bit value is xsum of data */
	byte = id_fetch_byte(cache_ptr, cookie.offset, &err);
	if (mem_ptr)
		mem_ptr[cookie.offset] = byte;
	xsums.data = byte;
	cookie.offset++;

	byte = id_fetch_byte(cache_ptr, cookie.offset, &err);
	if (mem_ptr)
		mem_ptr[cookie.offset] = byte;
	xsums.data |= byte << 8;
	cookie.offset++;

	/* Checksum the data (next id_len bytes), must match xsums.data */
	xsum = 0;
	id_dbg_printf("%s: mem_ptr %p\n", __FUNCTION__, mem_ptr);
	if (mem_ptr) {
		id_fetch_bytes(mem_ptr+cookie.offset, cookie.offset, hdr.data_length, &err);
		if (err != ID_EOK) {
			id_error("%s:%d err %d", __FUNCTION__, __LINE__, err);
			goto hard_way;
		}
		for (i = 0; i < hdr.data_length; ++i) {
			crc_15_step(&xsum, mem_ptr[cookie.offset + i]);
		}
	} else {
	hard_way:
		for (i = 0; i < hdr.data_length; ++i) {
			byte = id_fetch_byte(cache_ptr, cookie.offset + i, &err);
			if (mem_ptr)
				mem_ptr[cookie.offset + i] = byte;
			if (err != ID_EOK) {
				id_dbg_printf("%s[%u]\n", __FILE__, __LINE__);
				goto err_ret;
			}
			crc_15_step(&xsum, byte);
		}
	}
	if (xsum != xsums.data) {
		id_dbg_printf("%s[%u] xsum: 0x%04x, xsums.data: 0x%04x\n", 
		        __FILE__, __LINE__, xsum, xsums.data);
		err = -ID_EL2NSYNC;
		goto err_ret;
	}


	/* offset is now at the first byte of the root dictionary which
	   contains its span */
	data->root_offset = cookie.offset;
	cookie.mem_ptr = cache_ptr;
	data->root_size = extract_unsigned_pnum(&cookie, 5, &err);
	if (err != ID_EOK) {
		id_dbg_printf("%s[%u]\n", __FILE__, __LINE__);
		goto err_ret;
	}

	/* Restore data->mem_ptr to allow id_fetch_byte to read
	 * from the cached data instead of the AT24 chip */
	data->mem_ptr = mem_ptr;

	id_dbg_printf("%s:%d root_size (old %u) now %u + %u\n", __FUNCTION__, __LINE__,
		data->root_size, cookie.offset, data->root_offset);
	data->root_size += cookie.offset - data->root_offset;

	id_dbg_printf("Data format version: %u\n", hdr.id_fmt_ver);	

	return ID_EOK;

err_ret:

	/* Error return - make sure signature in SRAM is invalid */
	if (mem_ptr)
		mem_ptr[0] = 0;

	return err;
}

int id_startup(struct id_data *data, int (*setup_id_chip)(void), int (*shutdown_id_chip)(void))
{
	int ret, ret2;
	void *mem_ptr = data->mem_ptr;

	/* See if product ID already exists in SRAM (i.e. from x-loader) */
	if (data->mem_ptr) {
		ret = _id_startup(data, 1);
		data->mem_ptr = mem_ptr;
		if (!ret) {
			printf("Product ID data cached to: %p\n", mem_ptr);
			return ret;
		}
	}

	/* No ID data in SRAM, setup the ID chip and read for real */
	ret = (*setup_id_chip)();
	if (ret) {
		id_error("%s: setup_id_chip failed!", __FUNCTION__);
		return ret;
	}
	if (!ret)
		printf("Product ID data cached to: %p\n", mem_ptr);

	ret = _id_startup(data, 0);
	ret2 = (*shutdown_id_chip)();
	if (ret2)
		id_error("%s: shutdown_id_chip failed!", __FUNCTION__);

	if (!ret && ret2)
		ret = ret2;

	return ret;
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

/*
 * YAFFS: Yet Another Flash File System. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2007 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * yaffscfg.c  The configuration for the "direct" use of yaffs in U-boot.
 *
 */

#include <common.h>
#include "malloc.h"

#ifdef YAFFS_DEBUG_MALLOC
void *yaffs_malloc(const char *func, int line, size_t size)
{
	void *ptr;
	ptr =  malloc(size);
	if (!ptr)
		printf("%s:%d %s failed to allocate %u bytes!\n", func, line, __FUNCTION__, (unsigned int) size);
	return ptr;
}
#else
void *yaffs_malloc(size_t size)
{
	void *ptr;

	ptr = malloc(size);
	if (!ptr)
		printf("%s failed to allocate %u bytes!\n", __FUNCTION__, (unsigned int)size);
	return ptr;
}
#endif

void yaffs_free(void *ptr)
{
	free(ptr);
}

u32 yaffsfs_CurrentTime(void)
{
	return 0;
}

unsigned int yaffs_wr_attempts;
unsigned int yaffs_trace_mask = 0xffffffff;

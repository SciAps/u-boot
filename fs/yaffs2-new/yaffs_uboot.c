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

#include "yaffscfg.h"
#include "yaffsfs_errno.h"

#include "yaffsfs.h"

void yaffsfs_Lock(void)
{
}

void yaffsfs_Unlock(void)
{
}

void *yaffs_malloc(size_t size)
{
	void *ptr;

	ptr = malloc(size);
	if (!ptr)
		printf("%s failed to allocate %u bytes!\n", __FUNCTION__, (unsigned int)size);
	return ptr;
}

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

void yaffs_startup(void)
{
#if 0
	yaffs_initialise(NULL);
#endif
}

int cmd_yaffs_mount(const char *mp)
{
	int retval;

	yaffsfs_set_error(0);
	yaffs_startup();
	retval = yaffsfs_mount(mp);
	if (retval == -1)
		printf("Error mounting %s, return value: %d\n", mp, yaffsfs_get_error());
	return yaffsfs_get_error();
}

int cmd_yaffs_umount(char *mp)
{
	yaffsfs_set_error(0);
	if (yaffsfs_unmount(mp) == -1)
		printf("Error unmounting %s, return value: %d\n", mp, yaffsfs_get_error());
	return yaffsfs_get_error();
}

int cmd_yaffs_ls(const char *mountpt, int longlist)
{
	return -EINVAL;
}

int cmd_yaffs_df(const char *path, loff_t *size)
{
	return -EINVAL;
}

int cmd_yaffs_mwrite_file(char *fn, char *addr, int size)
{
	return -EINVAL;
}

int cmd_yaffs_mread_file(char *fn, char *addr, long *size)
{
	return -EINVAL;
}

int cmd_yaffs_mkdir(const char *dir)
{
	return -EINVAL;
}

int cmd_yaffs_rmdir(const char *dir)
{
	return -EINVAL;
}

int cmd_yaffs_rm(const char *path)
{
	return -EINVAL;
}

int cmd_yaffs_mv(const char *oldPath, const char *newPath)
{
	return -EINVAL;
}

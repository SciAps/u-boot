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

#include "yportenv.h"
#include "yaffsfs.h"
#include "yaffs_trace.h"

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
unsigned int yaffs_trace_mask = 0;

void yaffs_startup(void)
{
	yaffs_trace_mask = YAFFS_TRACE_ERROR | YAFFS_TRACE_BUG | YAFFS_TRACE_ALWAYS;
	yaffsfs_initialise();
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
	int i;
	yaffs_DIR *d;
	yaffs_dirent *de;
	struct yaffs_stat stat;
	char tempstr[255];

	d = yaffs_opendir(mountpt);

	if(!d)
	{
		printf("opendir failed\n");
		return -EINVAL;
	}
	else
	{
		for(i = 0; (de = yaffs_readdir(d)) != NULL; i++)
		{
			if (longlist)
			{
				sprintf(tempstr, "%s/%s", mountpt, de->d_name);
				yaffs_stat(tempstr, &stat);
				printf("%-25s\t%7ld\n",de->d_name, stat.st_size);
			}
			else
			{
				printf("%s\n",de->d_name);
			}
		}
	}
	return 0;
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
	int h;
	struct yaffs_stat s;

	yaffs_stat(fn,&s);

	printf ("Copy %s to 0x%p... ", fn, addr);
	h = yaffs_open(fn, O_RDWR,0);
	if(h<0)
	{
		printf("File not found\n");
		return -ENOENT;
	}

	yaffs_read(h,addr,(int)s.st_size);
	printf("\t[DONE]\n");
	*size = s.st_size;

	yaffs_close(h);
	return yaffsfs_get_error();
}

int cmd_yaffs_mkdir(const char *dir)
{
	int retval = yaffs_mkdir(dir, 0);

	if ( retval < 0)
		printf("yaffs_mkdir returning error: %d\n", yaffsfs_get_error());
	return yaffsfs_get_error();
}

int cmd_yaffs_rmdir(const char *dir)
{
	int retval = yaffs_rmdir(dir);

	if ( retval < 0)
		printf("yaffs_rmdir returning error: %d\n", yaffsfs_get_error());
	return yaffsfs_get_error();
}

int cmd_yaffs_rm(const char *path)
{
	int retval = yaffs_unlink(path);

	if ( retval < 0)
		printf("yaffs_unlink returning error: %d\n", yaffsfs_get_error());
	return yaffsfs_get_error();
}

int cmd_yaffs_mv(const char *oldPath, const char *newPath)
{
	int retval = yaffs_rename(newPath, oldPath);

	if ( retval < 0)
		printf("yaffs_unlink returning error: %d\n", yaffsfs_get_error());
	return yaffsfs_get_error();
}

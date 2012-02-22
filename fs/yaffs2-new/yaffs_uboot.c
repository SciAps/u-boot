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
	char *p;

	yaffsfs_set_error(0);
	yaffs_startup();
	retval = yaffsfs_mount(mp);
	if (retval == -1) {
		retval = yaffsfs_get_error(&p);
		printf("Failed: %s\n", p);
	}
	return yaffsfs_get_error(NULL);
}

int cmd_yaffs_umount(char *mp)
{
	char *p;
	int retval;

	yaffsfs_set_error(0);
	if (yaffsfs_unmount(mp) == -1) {
		retval = yaffsfs_get_error(&p);
		printf("Failed: %s\n", p);
	}
	return yaffsfs_get_error(NULL);
}

int cmd_yaffs_ls(const char *mountpt, int longlist)
{
	int i, ret;
	yaffs_DIR *d;
	yaffs_dirent *de;
	struct yaffs_stat stat;
	char tempstr[255];
	char *p;

	yaffsfs_set_error(0);

	d = yaffs_opendir(mountpt);

	if(!d)
	{
		ret = yaffsfs_get_error(&p);
		printf("%s: %s\n", mountpt, p);
		return ret;
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
	loff_t free_space;
	int ret = 0;
	char *p;

	yaffsfs_set_error(0);

	free_space = yaffs_freespace(path);
	if (free_space == -1) {
		ret = yaffsfs_get_error(&p);
		printf("Failed: %s\n", p);
	} else
		*size = free_space;

	return ret;
}

int cmd_yaffs_mwrite_file(char *fn, char *addr, int size)
{
	int h, ret;
	char *p;

	yaffsfs_set_error(0);
	printf ("Copy 0x%p to %s ... ", addr, fn);
	h = yaffs_open(fn, O_CREAT | O_RDWR | O_TRUNC, S_IREAD | S_IWRITE);
	if (h >= 0) {

		yaffs_write(h,addr,size);
		yaffs_close(h);

	}

	ret = yaffsfs_get_error(&p);
	if (ret)
		printf("[Failed: %s]\n", p);
	else
		printf("[DONE]\n");

	return ret;
}

int cmd_yaffs_mread_file(char *fn, char *addr, long *size)
{
	int h, ret;
	struct yaffs_stat s;
	char *p;

	yaffsfs_set_error(0);
	printf ("Copy %s to 0x%p... ", fn, addr);

	yaffs_stat(fn,&s);
	h = yaffs_open(fn, O_RDWR,0);
	if (h<0) {
		yaffsfs_set_error(-ENOENT);
	} else {
		yaffs_read(h,addr,(int)s.st_size);
		yaffs_close(h);
	}

	ret = yaffsfs_get_error(&p);
	if (ret)
		printf("[Failed: %s]\n", p);
	else {
		printf("[DONE]\n");
		*size = s.st_size;
	}

	return ret;
}

int cmd_yaffs_mkdir(const char *dir)
{
	int ret;
	char *p;

	yaffsfs_set_error(0);
	ret = yaffs_mkdir(dir, 0);
	if (ret < 0) {
		ret = yaffsfs_get_error(&p);
		printf("[Failed: %p]\n", p);
	}
	return ret;
}

int cmd_yaffs_rmdir(const char *dir)
{
	int ret;
	char *p;

	yaffsfs_set_error(0);
	ret = yaffs_rmdir(dir);
	if (ret < 0) {
		ret = yaffsfs_get_error(&p);
		printf("[Failed: %s]\n", p);
	}
	return ret;
}

int cmd_yaffs_rm(const char *path)
{
	int ret;
	char *p;

	yaffsfs_set_error(0);
	ret = yaffs_unlink(path);
	if (ret < 0) {
		ret = yaffsfs_get_error(&p);
		printf("[Failed: %s]\n", p);
	}
	return ret;
}

int cmd_yaffs_mv(const char *oldPath, const char *newPath)
{
	int ret;
	char *p;

	yaffsfs_set_error(0);
	ret = yaffs_rename(newPath, oldPath);

	if (ret < 0) {
		ret = yaffsfs_get_error(&p);
		printf("[Failed: %s]\n", p);
	}
	return ret;
}

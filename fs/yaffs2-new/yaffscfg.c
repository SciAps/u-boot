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
 * yaffscfg.c  The configuration for the "direct" use of yaffs.
 *
 * This file is intended to be modified to your requirements.
 * There is no need to redistribute this file.
 */

/* XXX U-BOOT XXX */
#include <common.h>

#include <config.h>
#include "nand.h"
#include "yaffscfg.h"
#include "yaffsfs.h"
#include "yaffs_packedtags2.h"
#include "yaffs_mtdif.h"
#include "yaffs_mtdif2.h"
#if 0
#include <errno.h>
#else
#include "malloc.h"
#endif

unsigned yaffs_traceMask = 0x0; /* Disable logging */
static int yaffs_errno = 0;

void yaffsfs_SetError(int err)
{
	//Do whatever to set error
	yaffs_errno = err;
}

int yaffsfs_GetError(void)
{
	return yaffs_errno;
}

void yaffsfs_Lock(void)
{
}

void yaffsfs_Unlock(void)
{
}

__u32 yaffsfs_CurrentTime(void)
{
	return 0;
}

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
}
#endif

void yaffs_free(void *ptr)
{
	free(ptr);
}

void yaffsfs_LocalInitialisation(void)
{
	// Define locking semaphore.
}

// Configuration for:
// /ram  2MB ramdisk
// /boot 2MB boot disk (flash)
// /flash 14MB flash disk (flash)
// NB Though /boot and /flash occupy the same physical device they
// are still disticnt "yaffs_Devices. You may think of these as "partitions"
// using non-overlapping areas in the same device.
//

#include "yaffs_ramdisk.h"
#include "yaffs_flashif.h"

static int isMounted = 0;
#define MOUNT_POINT "/flash"
extern nand_info_t nand_info[];



int yaffs_StartUp(void)
{
	yaffs_initialise(NULL);

	return 0;
}

void cmd_yaffs_mount(char *mp)
{
	yaffs_StartUp();
	int retval = yaffs_mount(mp);
	if( retval != -1)
		isMounted = 1;
	else
		printf("Error mounting %s, return value: %d\n", mp, yaffsfs_GetError());
}

void cmd_yaffs_umount(char *mp)
{
	if( yaffs_unmount(mp) == -1)
		printf("Error umounting %s, return value: %d\n", mp, yaffsfs_GetError());
}

void cmd_yaffs_mread_file(char *fn, char *addr, long *size)
{
	int h;
	struct yaffs_stat s;

	yaffs_stat(fn,&s);

	printf ("Copy %s to 0x%p... ", fn, addr);
	h = yaffs_open(fn, O_RDWR,0);
	if(h<0)
	{
		printf("File not found\n");
		return;
	}

	yaffs_read(h,addr,(int)s.st_size);
	printf("\t[DONE]\n");
	*size = s.st_size;

	yaffs_close(h);
}


void cmd_yaffs_mwrite_file(char *fn, char *addr, int size)
{
	int outh;

	outh = yaffs_open(fn, O_CREAT | O_RDWR | O_TRUNC, S_IREAD | S_IWRITE);
	if (outh < 0)
	{
		printf("Error opening file: %d\n", outh);
	}

	yaffs_write(outh,addr,size);

	yaffs_close(outh);
}


void cmd_yaffs_ls(const char *mountpt, int longlist)
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
}

int cmd_yaffs_df(const char *path, loff_t *space)
{
	loff_t free_space;
	int ret = 0;

	free_space = yaffs_freespace(path);
	if (free_space == -1) {
		ret = yaffsfs_GetError();
	} else
		*space = free_space;

	return ret;
}

void cmd_yaffs_mkdir(const char *dir)
{
	int retval = yaffs_mkdir(dir, 0);

	if ( retval < 0)
		printf("yaffs_mkdir returning error: %d\n", retval);
}

void cmd_yaffs_rmdir(const char *dir)
{
	int retval = yaffs_rmdir(dir);

	if ( retval < 0)
		printf("yaffs_rmdir returning error: %d\n", retval);
}

void cmd_yaffs_rm(const char *path)
{
	int retval = yaffs_unlink(path);

	if ( retval < 0)
		printf("yaffs_unlink returning error: %d\n", retval);
}

void cmd_yaffs_mv(const char *oldPath, const char *newPath)
{
	int retval = yaffs_rename(newPath, oldPath);

	if ( retval < 0)
		printf("yaffs_unlink returning error: %d\n", retval);
}

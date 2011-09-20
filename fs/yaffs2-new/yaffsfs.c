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

/* XXX U-BOOT XXX */
#include <common.h>
#include "malloc.h"

#include <jffs2/load_kernel.h>


#include "mtd_parts.h"
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>

#include "yaffscfg.h"
#include "yaffs_guts.h"
#include "yportenv.h"
#include "yaffs_mtdif.h"
#include "yaffs_mtdif2.h"
#include "yaffs_direct.h"
#include "yaffsfs_errno.h"


// #include "yaffsfs.h"

#if (CONFIG_SYS_MALLOC_LEN < (512 << 10))
#error Malloc Area too small for YAFFS, increas CONFIG_SYS_MALLOC_LEN to >= 512KiB
#endif

/* yaffsfs_FindDevice
 * yaffsfs_FindRoot
 * Scan the parttion list to find the root; split out rest of path from
 *  Initialial partition.  path must be /<partition>/<dir-or-file> */

#define N_MAX_PARTLEN 64

typedef enum {
	YAFFSFS_ACTION_CREATE,
	YAFFSFS_ACTION_FIND,
	YAFFSFS_ACTION_DESTROY
} yaffsfs_action_t;

#include "nand.h"
#include "yaffsfs_errno.h"
#include "yaffs_trace.h"

extern nand_info_t nand_info[];

static struct yaffs_dev *yaffsfs_FindDevice(const char *path, char **restOfPath, yaffsfs_action_t action)
{
	int slash_cnt, i, ret;
	char partname[N_MAX_PARTLEN];
	void *part_priv;
	loff_t part_off, part_size;
	int part_idx;
	struct mtd_device *dev;
	void *cookie;
	struct yaffs_dev *flashDev;
	struct yaffs_param *param;
	struct yaffs_direct_context *context = NULL;
	struct mtd_info *mtd;

	if (!path)
		return NULL;
	if (*path != '/') {
		printf("Path '%s' does not start with '/'!\n", path);
		return NULL;
	}
	path++;	/* Skip over leading '/' */

	for (slash_cnt=i=0; i<N_MAX_PARTLEN && path[i]; ++i) {
		if (path[i] == '/')
			break;
		partname[i] = path[i];
	}
	partname[i] = '\0';

	/* save rest of path(if exists) */
	*restOfPath = (char *)path+i;
	if (**restOfPath == '/')
		(*restOfPath)++;

	/* Find the partition */
	ret = mtd_get_part_priv(partname, &part_idx, &dev, &part_off, &part_size, &cookie, &part_priv);
	if (ret)
		return NULL;

	switch(action) {
	case YAFFSFS_ACTION_FIND:
		return (struct yaffs_dev *)part_priv;

	case YAFFSFS_ACTION_CREATE:
		if (part_priv) {
			printf("Huh? mount of partition '%s' already has device %p\n", partname, part_priv);
			return NULL;
		}

		/* First time we're seeing this device, create it using
		   information inside of the MTD_DEVICE structure */
		flashDev = malloc(sizeof(struct yaffs_dev));
		if (!flashDev) {
			printf("%s:%d out of memory!\n", __FUNCTION__, __LINE__);
			return NULL;
		}
		memset(flashDev, 0, sizeof(*flashDev));

		context = malloc(sizeof(struct yaffs_direct_context));
		if (!context) {
			printf("%s:%d out of memory!\n", __FUNCTION__, __LINE__);
			free(flashDev);
			return NULL;
		}


		

		printf("%s: initialise struct yaffs_dev %p\n", __FUNCTION__, flashDev);

		/* Side effect of mtd_get_part_priv() is to set nand_curr_device */
		mtd = &nand_info[nand_curr_device];

		flashDev->driver_context = mtd;

		memset(context, 0, sizeof(*context));
		flashDev->os_context = context;
		yaffs_dev_to_lc(flashDev)->spare_buffer =
			malloc(mtd->oobsize);

		param = &(flashDev->param);

		param->start_block = part_off / mtd->erasesize;
		param->end_block = (part_off + part_size - 1) / mtd->erasesize;
#if 1
		printf("%s: part_off %x part_size %x startBlock %u endBlock %u\n", __FUNCTION__, (unsigned int)part_off, (unsigned int)part_size, param->start_block, param->end_block);
#endif
		param->n_reserved_blocks = 5;

		param->write_chunk_tags_fn = nandmtd2_write_chunk_tags;
		param->read_chunk_tags_fn = nandmtd2_read_chunk_tags;
		param->bad_block_fn = nandmtd2_mark_block_bad;
		param->query_block_fn = nandmtd2_query_block;
		param->is_yaffs2 = 1;
#if 0
#if 1
		Yaffs_DeviceToContext(dev)->spareBuffer = YMALLOC(mtd->oobsize);
#else
		flashDev->spareBuffer = YMALLOC(mtd->oobsize);
#endif
#endif
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17))
		param->total_bytes_per_chunk = mtd->writesize;
		param->chunks_per_block = mtd->erasesize / mtd->writesize;
#else
		param->total_bytes_per_chunk = mtd->oobblock;
		param->chunks_per_block = mtd->erasesize / mtd->oobblock;
#endif
#if 0
		flashDev->nCheckpointReservedBlocks = 10;
#endif
		/* ... and common functions */
		param->erase_fn = nandmtd_erase_block;
		param->initialise_flash_fn = nandmtd_initialise;
#if 0
		flashDev->removeObjectCallback = yaffsfs_RemoveObjectCallback;
#endif

		mtd_set_part_priv(cookie, flashDev);

		return flashDev;

	case YAFFSFS_ACTION_DESTROY:
		flashDev = (struct yaffs_dev *)part_priv;
		if (yaffs_dev_to_lc(flashDev)->spare_buffer)
			free(yaffs_dev_to_lc(flashDev)->spare_buffer);
		if (flashDev->os_context)
			free(flashDev->os_context);
		free(part_priv);
		mtd_set_part_priv(cookie, NULL);
		return NULL;

	default:
		printf("%s: unknown action %d\n", __FUNCTION__, (int)action);
		return NULL;
	}

}

static struct yaffs_obj *yaffsfs_FindRoot(const char *path, char **restOfPath)
{

	struct yaffs_dev *dev;

	dev= yaffsfs_FindDevice(path,restOfPath, YAFFSFS_ACTION_FIND);
	if(dev && dev->is_mounted)
	{
		return dev->root_dir;
	}
	return NULL;
}

loff_t yaffs_freespace(const char *path)
{
	loff_t retVal=-1;
	struct yaffs_dev *dev=NULL;
	char *dummy;

	yaffsfs_Lock();
	dev = yaffsfs_FindDevice(path,&dummy, YAFFSFS_ACTION_FIND);
	if(dev  && dev->is_mounted)
	{
		retVal = yaffs_get_n_free_chunks(dev);
		retVal *= dev->data_bytes_per_chunk;

	}
	else
	{
		yaffsfs_set_error(-EINVAL);
	}

	yaffsfs_Unlock();
	return retVal;
}

/* Forward references */
static struct yaffs_obj *yaffsfs_find_object(struct yaffs_obj *relative_dir, const char *path, int sym_depth);

static struct yaffs_obj *yaffsfs_follow_link(struct yaffs_obj *obj,int symDepth)
{

	while(obj && obj->variant_type == YAFFS_OBJECT_TYPE_SYMLINK)
	{
		char *alias = obj->variant.symlink_variant.alias;

		if(*alias == '/')
		{
			// Starts with a /, need to scan from root up
			obj = yaffsfs_find_object(NULL,alias,symDepth++);
		}
		else
		{
			// Relative to here, so use the parent of the symlink as a start
			obj = yaffsfs_find_object(obj->parent,alias,symDepth++);
		}
	}
	return obj;
}

#define YAFFSFS_MAX_SYMLINK_DEREFERENCES 5

// yaffsfs_find_directory
// Parse a path to determine the directory and the name within the directory.
//
// eg. "/data/xx/ff" --> puts name="ff" and returns the directory "/data/xx"
static struct yaffs_obj *yaffsfs_find_directory(struct yaffs_obj *startDir,const char *path,char **name,int symDepth)
{
	struct yaffs_obj *dir;
	char *restOfPath;
	char str[YAFFS_MAX_NAME_LENGTH+1];
	int i;

	if(symDepth > YAFFSFS_MAX_SYMLINK_DEREFERENCES)
	{
		return NULL;
	}

	if(startDir)
	{
		dir = startDir;
		restOfPath = (char *)path;
	}
	else
	{
		dir = yaffsfs_FindRoot(path,&restOfPath);
	}

	while(dir)
	{
		// parse off /.
		// curve ball: also throw away surplus '/'
		// eg. "/ram/x////ff" gets treated the same as "/ram/x/ff"
		while(*restOfPath == '/')
		{
			restOfPath++; // get rid of '/'
		}

		*name = restOfPath;
		i = 0;

		while(*restOfPath && *restOfPath != '/')
		{
			if (i < YAFFS_MAX_NAME_LENGTH)
			{
				str[i] = *restOfPath;
				str[i+1] = '\0';
				i++;
			}
			restOfPath++;
		}

		if(!*restOfPath)
		{
			// got to the end of the string
			return dir;
		}
		else
		{
			if(strcmp(str,".") == 0)
			{
				// Do nothing
			}
			else if(strcmp(str,"..") == 0)
			{
				dir = dir->parent;
			}
			else
			{
				dir = yaffs_find_by_name(dir,str);

				while(dir && dir->variant_type == YAFFS_OBJECT_TYPE_SYMLINK)
				{

					dir = yaffsfs_follow_link(dir,symDepth);

				}

				if(dir && dir->variant_type != YAFFS_OBJECT_TYPE_DIRECTORY)
				{
					dir = NULL;
				}
			}
		}
	}
	// directory did not exist.
	return NULL;
}

static struct yaffs_obj *yaffsfs_find_object(struct yaffs_obj *relative_dir, const char *path, int sym_depth)
{
	struct yaffs_obj *dir;
	char *name;

	dir = yaffsfs_find_directory(relative_dir, path, &name, sym_depth);

	if (dir && *name)
		return yaffs_find_by_name(dir, name);
	return dir;
}

int yaffsfs_mount(const char *path)
{
	int retVal=-1;
	int result=YAFFS_FAIL;
	struct yaffs_dev *dev=NULL;
	char *dummy;

	yaffs_trace(YAFFS_TRACE_ALWAYS,("yaffs: Mounting %s\n",path));

	yaffsfs_Lock();
	dev = yaffsfs_FindDevice(path,&dummy, YAFFSFS_ACTION_CREATE);
	if(dev)
	{
		if(!dev->is_mounted)
		{
			result = yaffs_guts_initialise(dev);
			if(result == YAFFS_FAIL)
			{
				// todo error - mount failed
				yaffsfs_set_error(-ENOMEM);
			}
			retVal = result ? 0 : -1;

		}
		else
		{
			//todo error - already mounted.
			yaffsfs_set_error(-EBUSY);
		}
	}
	else
	{
		// todo error - no device
		yaffsfs_set_error(-ENODEV);
	}
	yaffsfs_Unlock();
	return retVal;

}

int yaffsfs_unmount(const char *path)
{
	int retVal=-1;
	struct yaffs_dev *dev=NULL;
	char *dummy;

	yaffsfs_Lock();
	dev = yaffsfs_FindDevice(path,&dummy, YAFFSFS_ACTION_FIND);
	if(dev)
	{
		if(dev->is_mounted)
		{
			yaffs_flush_whole_cache(dev);
			yaffs_checkpoint_save(dev);

			yaffs_deinitialise(dev);

			yaffsfs_FindDevice(path, &dummy, YAFFSFS_ACTION_DESTROY);

			retVal = 0;
		}
		else
		{
			//todo error - not mounted.
			yaffsfs_set_error(-EINVAL);

		}
	}
	else
	{
		// todo error - no device
		yaffsfs_set_error(-ENODEV);
	}
	yaffsfs_Unlock();
	return retVal;

}

int yaffs_DumpDevStruct(const char *path)
{
	char *rest;

	struct yaffs_obj *obj = yaffsfs_FindRoot(path,&rest);

	if(obj)
	{
		struct yaffs_dev *dev = obj->my_dev;

		printf("\n"
			   "nPageWrites.......... %d\n"
			   "nPageReads........... %d\n"
			   "nBlockErasures....... %d\n"
			   "nGCCopies............ %d\n"
			   "garbageCollections... %d\n"
			   "passiveGarbageColl'ns %d\n"
			   "\n",
				dev->n_page_writes,
				dev->n_page_reads,
				dev->n_erasures,
				dev->n_gc_copies,
				dev->all_gcs,
				dev->passive_gc_count
		);

	}
	return 0;
}

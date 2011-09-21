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

#include "yaffsfs.h"

#if (CONFIG_SYS_MALLOC_LEN < (512 << 10))
#error Malloc Area too small for YAFFS, increas CONFIG_SYS_MALLOC_LEN to >= 512KiB
#endif

/* YAFFSFS_RW_SIZE must be a power of 2 */
#define YAFFSFS_RW_SHIFT (13)
#define YAFFSFS_RW_SIZE  (1<<YAFFSFS_RW_SHIFT)

typedef struct {
	int count;	/* Number of handles accessing this inode */
	struct yaffs_obj *iObj;
} yaffsfs_Inode;

typedef struct{
	u8 	reading:1;
	u8 	writing:1;
	u8 	append:1;
	u8	shareRead:1;
	u8	shareWrite:1;
	int	inodeId:12;	/* Index to corresponding yaffsfs_Inode */
	int	handleCount:10;	/* Number of handles for this fd */
	u32 position;		/* current position in file */
}yaffsfs_FileDes;

typedef struct {
	short int fdId;
	short int useCount;
} yaffsfs_Handle;

static yaffsfs_Inode yaffsfs_inode[YAFFSFS_N_HANDLES];
static yaffsfs_FileDes yaffsfs_fd[YAFFSFS_N_HANDLES];
static yaffsfs_Handle yaffsfs_handle[YAFFSFS_N_HANDLES];

static int yaffsfs_handlesInitialised;

/*
 * yaffsfs_InitHandle
 * Inilitalise handle management on start-up.
 */

static void yaffsfs_InitHandles(void)
{
	int i;
	if(yaffsfs_handlesInitialised)
                return;

	memset(yaffsfs_inode,0,sizeof(yaffsfs_inode));
	memset(yaffsfs_fd,0,sizeof(yaffsfs_fd));
	memset(yaffsfs_handle,0,sizeof(yaffsfs_handle));

	for(i = 0; i < YAFFSFS_N_HANDLES; i++)
		yaffsfs_fd[i].inodeId = -1;
	for(i = 0; i < YAFFSFS_N_HANDLES; i++)
		yaffsfs_handle[i].fdId = -1;
}

static yaffsfs_Handle *yaffsfs_HandleToPointer(int h)
{
	if(h >= 0 && h <= YAFFSFS_N_HANDLES)
		return &yaffsfs_handle[h];
	return NULL;
}

static yaffsfs_FileDes *yaffsfs_HandleToFileDes(int handle)
{
	yaffsfs_Handle *h = yaffsfs_HandleToPointer(handle);

	if(h && h->useCount > 0 && h->fdId >= 0 && h->fdId < YAFFSFS_N_HANDLES)
		return  &yaffsfs_fd[h->fdId];

	return NULL;
}

static yaffsfs_Inode *yaffsfs_HandleToInode(int handle)
{
	yaffsfs_FileDes *fd = yaffsfs_HandleToFileDes(handle);

	if(fd && fd->handleCount > 0 &&
		fd->inodeId >= 0 && fd->inodeId < YAFFSFS_N_HANDLES)
		return  &yaffsfs_inode[fd->inodeId];

	return NULL;
}

static struct yaffs_obj *yaffsfs_HandleToObject(int handle)
{
	yaffsfs_Inode *in = yaffsfs_HandleToInode(handle);

	if(in)
		return in->iObj;

	return NULL;
}


/*
 * yaffsfs_FindInodeIdForObject
 * Find the inode entry for an object, if it exists.
 */

static int yaffsfs_FindInodeIdForObject(struct yaffs_obj *obj)
{
	int i;
	int ret = -1;

	if(obj)
		obj = yaffs_get_equivalent_obj(obj);

	/* Look for it in open inode table*/
	for(i = 0; i < YAFFSFS_N_HANDLES && ret < 0; i++){
		if(yaffsfs_inode[i].iObj == obj)
			ret = i;
	}
	return ret;
}

/*
 * yaffsfs_GetInodeIdForObject
 * Grab an inode entry when opening a new inode.
 */
static int yaffsfs_GetInodeIdForObject(struct yaffs_obj *obj)
{
	int i;
	int ret;
	yaffsfs_Inode *in = NULL;

	if(obj)
		obj = yaffs_get_equivalent_obj(obj);

        ret = yaffsfs_FindInodeIdForObject(obj);

	for(i = 0; i < YAFFSFS_N_HANDLES && ret < 0; i++){
		if(!yaffsfs_inode[i].iObj)
			ret = i;
	}

	if(ret>=0){
		in = &yaffsfs_inode[ret];
		if(!in->iObj)
			in->count = 0;
		in->iObj = obj;
		in->count++;
	}


	return ret;
}

/* yaffsfs_FindDevice
 * yaffsfs_FindRoot
 * Scan the parttion list to find the root; split out rest of path from
 *  Initialial partition.  path must be /<partition>/<dir-or-file> */

#define N_MAX_PARTLEN 64

#include "nand.h"
#include "yaffsfs_errno.h"
#include "yaffs_trace.h"

typedef struct
{
	u32 magic;
	yaffs_dirent de;		/* directory entry being used by this dsc */
	YCHAR name[NAME_MAX+1];		/* name of directory being searched */
        struct yaffs_obj *dirObj;           /* ptr to directory being searched */
        struct yaffs_obj *nextReturn;       /* obj to be returned by next readddir */
        int offset;
        struct list_head others;
} yaffsfs_DirectorySearchContext;

static void yaffsfs_RemoveObjectCallback(struct yaffs_obj *obj);
static void yaffsfs_DirAdvance(yaffsfs_DirectorySearchContext *dsc);

typedef enum {
	YAFFSFS_ACTION_CREATE,
	YAFFSFS_ACTION_FIND,
	YAFFSFS_ACTION_DESTROY
} yaffsfs_action_t;

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
#if 0
		printf("%s: part_off %x part_size %x startBlock %u endBlock %u\n", __FUNCTION__, (unsigned int)part_off, (unsigned int)part_size, param->start_block, param->end_block);
#endif
		param->n_reserved_blocks = 5;

		param->write_chunk_tags_fn = nandmtd2_write_chunk_tags;
		param->read_chunk_tags_fn = nandmtd2_read_chunk_tags;
		param->bad_block_fn = nandmtd2_mark_block_bad;
		param->query_block_fn = nandmtd2_query_block;
		param->is_yaffs2 = 1;
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17))
		param->total_bytes_per_chunk = mtd->writesize;
		param->chunks_per_block = mtd->erasesize / mtd->writesize;
#else
		param->total_bytes_per_chunk = mtd->oobblock;
		param->chunks_per_block = mtd->erasesize / mtd->oobblock;
#endif
		/* ... and common functions */
		param->erase_fn = nandmtd_erase_block;
		param->initialise_flash_fn = nandmtd_initialise;
		param->remove_obj_fn = yaffsfs_RemoveObjectCallback;

		/* If the OOB area has only 16 bytes available can't
		   ECC the tag (must assume OOB is ECC'd already) */
		if (mtd->ecclayout && mtd->ecclayout->oobavail <= 16) {
			printf("ECC oobavail <= 16; forcing \"tags-ecc-off\"\n");
			param->no_tags_ecc = 1;
		}

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

#define yaffsfs_IsPathDivider(x) ((x) == '/')

/* Forward references */
static struct yaffs_obj *yaffsfs_find_object(struct yaffs_obj *rel_dir,
			const char *path, int sym_depth, int getEquiv,
			struct yaffs_obj **dir_out, int *not_dir, int *loop);

static struct yaffs_obj *yaffsfs_follow_link(struct yaffs_obj *obj,int symDepth, int *loop)
{
	if (obj)
		obj = yaffs_get_equivalent_obj(obj);

	while(obj && obj->variant_type == YAFFS_OBJECT_TYPE_SYMLINK)
	{
		YCHAR *alias = obj->variant.symlink_variant.alias;

		if(yaffsfs_IsPathDivider(*alias))
			/* Starts with a /, need to scan from root up */
			obj = yaffsfs_find_object(NULL,alias,symDepth++,
						1,NULL,NULL,loop);
		else
			/* Relative to here, so use the parent of the symlink as a start */
			obj = yaffsfs_find_object(obj->parent,alias,symDepth++,
						1,NULL,NULL,loop);
	}
	return obj;
}

#define YAFFSFS_MAX_SYMLINK_DEREFERENCES 5

// yaffsfs_find_directory
// Parse a path to determine the directory and the name within the directory.
//
// eg. "/data/xx/ff" --> puts name="ff" and returns the directory "/data/xx"

static struct yaffs_obj *yaffsfs_do_find_directory(struct yaffs_obj *startDir,
				const YCHAR *path, YCHAR **name, int symDepth,
				int *notDir,int *loop)
{
	struct yaffs_obj *dir;
	YCHAR *restOfPath;
	YCHAR str[YAFFS_MAX_NAME_LENGTH+1];
	int i;

	if(symDepth > YAFFSFS_MAX_SYMLINK_DEREFERENCES){
		if(loop)
			*loop = 1;
		return NULL;
	}

	if(startDir){
		dir = startDir;
		restOfPath = (YCHAR *)path;
	}
	else
		dir = yaffsfs_FindRoot(path,&restOfPath);


	while(dir){
		/*
		 * parse off /.
		 * curve ball: also throw away surplus '/'
		 * eg. "/ram/x////ff" gets treated the same as "/ram/x/ff"
		 */
		while(yaffsfs_IsPathDivider(*restOfPath))
			restOfPath++; /* get rid of '/' */

		*name = restOfPath;
		i = 0;

		while(*restOfPath && !yaffsfs_IsPathDivider(*restOfPath)){
			if (i < YAFFS_MAX_NAME_LENGTH){
				str[i] = *restOfPath;
				str[i+1] = '\0';
				i++;
			}
			restOfPath++;
		}

		if(!*restOfPath)
			/* got to the end of the string */
			return dir;
		else{
			if(strcmp(str,_Y(".")) == 0){
				/* Do nothing */
			} else if(strcmp(str,_Y("..")) == 0) {
				dir = dir->parent;
			} else{
				dir = yaffs_find_by_name(dir,str);

				dir = yaffsfs_follow_link(dir,symDepth,loop);

				if(dir && dir->variant_type !=
					YAFFS_OBJECT_TYPE_DIRECTORY){
					if(notDir)
						*notDir = 1;
					dir = NULL;
				}

			}
		}
	}
	/* directory did not exist. */
	return NULL;
}

static struct yaffs_obj *yaffsfs_find_directory(struct yaffs_obj *relDir,
					const YCHAR *path,
					YCHAR **name,
					int symDepth,
					int *notDir,
					int *loop)
{
	return yaffsfs_do_find_directory(relDir,path,name,symDepth,notDir,loop);
}


static struct yaffs_obj *yaffsfs_find_object(struct yaffs_obj *relative_dir,
			const char *path, int sym_depth, int get_equiv,
			struct yaffs_obj **dir_out, int *not_dir, int *loop)
{
	struct yaffs_obj *dir;
	struct yaffs_obj *obj;
	char *name;

	dir = yaffsfs_find_directory(relative_dir, path, &name, sym_depth, not_dir, loop);

	if (dir_out)
		*dir_out = dir;


	if (dir && *name)
		obj = yaffs_find_by_name(dir, name);
	else
		obj = dir;

	if (get_equiv)
		obj = yaffs_get_equivalent_obj(obj);

	return obj;
}

int yaffsfs_mount(const char *path)
{
	int retVal=-1;
	int result=YAFFS_FAIL;
	struct yaffs_dev *dev=NULL;
	char *dummy;

	yaffs_trace(YAFFS_TRACE_ALWAYS, "yaffs: Mounting %s",path);

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

static struct list_head search_contexts;

static int yaffsfs_CheckPath(const YCHAR *path)
{
	int n=0;
	int divs=0;
	while(*path && n < YAFFS_MAX_NAME_LENGTH && divs < 100){
		if(yaffsfs_IsPathDivider(*path)){
			n=0;
			divs++;
		} else
			n++;
		path++;
	}

	return (*path) ? -1 : 0;
}

static void yaffsfs_SetDirRewound(yaffsfs_DirectorySearchContext *dsc)
{
	if(dsc &&
	   dsc->dirObj &&
	   dsc->dirObj->variant_type == YAFFS_OBJECT_TYPE_DIRECTORY){

           dsc->offset = 0;

           if( list_empty(&dsc->dirObj->variant.dir_variant.children))
                dsc->nextReturn = NULL;
           else
                dsc->nextReturn = list_entry(dsc->dirObj->variant.dir_variant.children.next,
                                                struct yaffs_obj,siblings);
        } else {
		/* Hey someone isn't playing nice! */
	}
}

static void yaffsfs_RemoveObjectCallback(struct yaffs_obj *obj)
{

        struct list_head *i;
        yaffsfs_DirectorySearchContext *dsc;

        /* if search contexts not initilised then skip */
        if(!search_contexts.next)
                return;

        /* Iterate through the directory search contexts.
         * If any are the one being removed, then advance the dsc to
         * the next one to prevent a hanging ptr.
         */
         list_for_each(i, &search_contexts) {
                if (i) {
                        dsc = list_entry(i, yaffsfs_DirectorySearchContext,others);
                        if(dsc->nextReturn == obj)
                                yaffsfs_DirAdvance(dsc);
                }
	}

}

yaffs_DIR *yaffs_opendir(const YCHAR *dirname)
{
	yaffs_DIR *dir = NULL;
 	struct yaffs_obj *obj = NULL;
	yaffsfs_DirectorySearchContext *dsc = NULL;
	int notDir = 0;
	int loop = 0;

	if(!dirname){
		yaffsfs_set_error(-EFAULT);
		return NULL;
	}

	if(yaffsfs_CheckPath(dirname) < 0){
		yaffsfs_set_error(-ENAMETOOLONG);
		return NULL;
	}

	yaffsfs_Lock();

	obj = yaffsfs_find_object(NULL,dirname,0,1,NULL,&notDir,&loop);

	if(!obj && notDir)
		yaffsfs_set_error(-ENOTDIR);
	else if(loop)
		yaffsfs_set_error(-ELOOP);
	else if(!obj)
		yaffsfs_set_error(-ENOENT);
	else if(obj->variant_type != YAFFS_OBJECT_TYPE_DIRECTORY)
		yaffsfs_set_error(-ENOTDIR);
	else {

		dsc = kmalloc(sizeof(yaffsfs_DirectorySearchContext), 0);
		dir = (yaffs_DIR *)dsc;

		if(dsc){
			memset(dsc,0,sizeof(yaffsfs_DirectorySearchContext));
                        dsc->magic = YAFFS_MAGIC;
                        dsc->dirObj = obj;
                        strncpy(dsc->name,dirname,NAME_MAX);
                        INIT_LIST_HEAD(&dsc->others);

                        if(!search_contexts.next)
                                INIT_LIST_HEAD(&search_contexts);

                        list_add(&dsc->others,&search_contexts);
                        yaffsfs_SetDirRewound(dsc);
		}

        }

	yaffsfs_Unlock();

	return dir;
}

static void yaffsfs_DirAdvance(yaffsfs_DirectorySearchContext *dsc)
{
	if(dsc &&
	   dsc->dirObj &&
           dsc->dirObj->variant_type == YAFFS_OBJECT_TYPE_DIRECTORY){

           if( dsc->nextReturn == NULL ||
               list_empty(&dsc->dirObj->variant.dir_variant.children))
                dsc->nextReturn = NULL;
           else {
                   struct list_head *next = dsc->nextReturn->siblings.next;

                   if( next == &dsc->dirObj->variant.dir_variant.children)
                        dsc->nextReturn = NULL; /* end of list */
                   else
                        dsc->nextReturn = list_entry(next,struct yaffs_obj,siblings);
           }
        } else {
                /* Hey someone isn't playing nice! */
	}
}

static int yaffsfs_alt_dir_path(const YCHAR *path, YCHAR **ret_path)
{
	YCHAR *alt_path = NULL;
	int path_length;
	int i;

	/*
	 * We don't have a definition for max path length.
	 * We will use 3 * max name length instead.
	 */
	*ret_path = NULL;
	path_length = strnlen(path,(YAFFS_MAX_NAME_LENGTH+1)*3 +1);

	/* If the last character is a path divider, then we need to
	 * trim it back so that the name look-up works properly.
	 * eg. /foo/new_dir/ -> /foo/newdir
	 * Curveball: Need to handle multiple path dividers:
	 * eg. /foof/sdfse///// -> /foo/sdfse
	 */
	if(path_length > 0 &&
		yaffsfs_IsPathDivider(path[path_length-1])){
		alt_path = kmalloc(path_length + 1, 0);
		if(!alt_path)
			return -1;
		strcpy(alt_path, path);
		for(i = path_length-1;
			i >= 0 && yaffsfs_IsPathDivider(alt_path[i]);
			i--)
			alt_path[i] = (YCHAR) 0;
	}
	*ret_path = alt_path;
	return 0;
}

struct yaffs_dirent *yaffs_readdir(yaffs_DIR *dirp)
{
	yaffsfs_DirectorySearchContext *dsc = (yaffsfs_DirectorySearchContext *)dirp;
	struct yaffs_dirent *retVal = NULL;

	yaffsfs_Lock();

	if(dsc && dsc->magic == YAFFS_MAGIC){
		yaffsfs_set_error(0);
		if(dsc->nextReturn){
			dsc->de.d_ino = yaffs_get_equivalent_obj(dsc->nextReturn)->obj_id;
			dsc->de.d_dont_use = (unsigned)dsc->nextReturn;
			dsc->de.d_off = dsc->offset++;
			yaffs_get_obj_name(dsc->nextReturn,dsc->de.d_name,NAME_MAX);
			if(strnlen(dsc->de.d_name,NAME_MAX+1) == 0)
			{
				/* this should not happen! */
				strcpy(dsc->de.d_name,_Y("zz"));
			}
			dsc->de.d_reclen = sizeof(struct yaffs_dirent);
			retVal = &dsc->de;
			yaffsfs_DirAdvance(dsc);
		} else
			retVal = NULL;
	} else
		yaffsfs_set_error(-EBADF);

	yaffsfs_Unlock();

	return retVal;

}

int yaffsfs_do_unlink(const YCHAR *path,int isDirectory)
{
	struct yaffs_obj *dir = NULL;
	struct yaffs_obj *obj = NULL;
	YCHAR *name;
	int result = YAFFS_FAIL;
	int notDir = 0;
	int loop = 0;

	if(!path){
		yaffsfs_set_error(-EFAULT);
		return -1;
	}

	if(yaffsfs_CheckPath(path) < 0){
		yaffsfs_set_error(-ENAMETOOLONG);
		return -1;
	}

	yaffsfs_Lock();

	obj = yaffsfs_find_object(NULL,path,0,0,NULL,NULL,NULL);
	dir = yaffsfs_find_directory(NULL,path,&name,0,&notDir,&loop);

	if(!dir && notDir)
		yaffsfs_set_error(-ENOTDIR);
	else if(loop)
		yaffsfs_set_error(-ELOOP);
	else if(!dir)
		yaffsfs_set_error(-ENOENT);
	else if(strncmp(name,_Y("."),2) == 0)
		yaffsfs_set_error(-EINVAL);
	else if(!obj)
		yaffsfs_set_error(-ENOENT);
	else if(obj->my_dev->read_only)
		yaffsfs_set_error(-EROFS);
	else if(!isDirectory && obj->variant_type == YAFFS_OBJECT_TYPE_DIRECTORY)
		yaffsfs_set_error(-EISDIR);
	else if(isDirectory && obj->variant_type != YAFFS_OBJECT_TYPE_DIRECTORY)
		yaffsfs_set_error(-ENOTDIR);
	else if(isDirectory && obj == obj->my_dev->root_dir)
		yaffsfs_set_error(-EBUSY); /* Can't rmdir a root */
	else {
		result = yaffs_unlinker(dir,name);

		if(result == YAFFS_FAIL && isDirectory)
			yaffsfs_set_error(-ENOTEMPTY);
	}

	yaffsfs_Unlock();

	return (result == YAFFS_FAIL) ? -1 : 0;
}

int yaffs_unlink(const YCHAR *path)
{
	return yaffsfs_do_unlink(path,0);
}


int yaffs_rename(const YCHAR *oldPath, const YCHAR *newPath)
{
	struct yaffs_obj *olddir = NULL;
	struct yaffs_obj *newdir = NULL;
	struct yaffs_obj *obj = NULL;
	struct yaffs_obj *newobj = NULL;
	YCHAR *oldname;
	YCHAR *newname;
	int result= YAFFS_FAIL;
	int rename_allowed = 1;
	int notOldDir = 0;
	int notNewDir = 0;
	int oldLoop = 0;
	int newLoop = 0;

	YCHAR *alt_newpath=NULL;

	if(!oldPath || !newPath){
		yaffsfs_set_error(-EFAULT);
		return -1;
	}

	if(yaffsfs_CheckPath(oldPath) < 0 ||
		yaffsfs_CheckPath(newPath) < 0){
		yaffsfs_set_error(-ENAMETOOLONG);
		return -1;
	}

	if(yaffsfs_alt_dir_path(newPath, &alt_newpath) < 0){
		yaffsfs_set_error(-ENOMEM);
		return -1;
	}
	if(alt_newpath)
		newPath = alt_newpath;

	yaffsfs_Lock();


	olddir = yaffsfs_find_directory(NULL,oldPath,&oldname,0,&notOldDir,&oldLoop);
	newdir = yaffsfs_find_directory(NULL,newPath,&newname,0,&notNewDir,&newLoop);
	obj = yaffsfs_find_object(NULL,oldPath,0,0,NULL,NULL,NULL);
	newobj = yaffsfs_find_object(NULL,newPath,0,0,NULL,NULL,NULL);

	/* If the object being renamed is a directory and the
	 * path ended with a "/" then the olddir == obj.
	 * We pass through NULL for the old name to tell the lower layers
	 * to use olddir as the object.
	 */

	if(olddir == obj)
		oldname = NULL;

	if((!olddir && notOldDir) || (!newdir && notNewDir)) {
		yaffsfs_set_error(-ENOTDIR);
		rename_allowed = 0;
	} else if(oldLoop || newLoop) {
		yaffsfs_set_error(-ELOOP);
		rename_allowed = 0;
	} else if (olddir && oldname && strncmp(oldname, _Y("."),2) == 0){
		yaffsfs_set_error(-EINVAL);
		rename_allowed = 0;
	}else if(!olddir || !newdir || !obj) {
		yaffsfs_set_error(-ENOENT);
		rename_allowed = 0;
	} else if(obj->my_dev->read_only){
		yaffsfs_set_error(-EROFS);
		rename_allowed = 0;
	} else if(yaffs_is_non_empty_dir(newobj)){
		yaffsfs_set_error(-ENOTEMPTY);
		rename_allowed = 0;
	} else if(olddir->my_dev != newdir->my_dev) {
		/* Rename must be on same device */
		yaffsfs_set_error(-EXDEV);
		rename_allowed = 0;
	} else if(obj && obj->variant_type == YAFFS_OBJECT_TYPE_DIRECTORY) {
		/*
		 * It is a directory, check that it is not being renamed to
		 * being its own decendent.
		 * Do this by tracing from the new directory back to the root,
		 * checking for obj
		 */

		struct yaffs_obj *xx = newdir;

		while( rename_allowed && xx){
			if(xx == obj)
				rename_allowed = 0;
			xx = xx->parent;
		}
		if(!rename_allowed)
			yaffsfs_set_error(-EINVAL);
	}

	if(rename_allowed)
		result = yaffs_rename_obj(olddir,oldname,newdir,newname);

	yaffsfs_Unlock();

	if(alt_newpath)
		kfree(alt_newpath);

	return (result == YAFFS_FAIL) ? -1 : 0;
}

static int yaffsfs_DoStat(struct yaffs_obj *obj,struct yaffs_stat *buf)
{
	int retVal = -1;

	obj = yaffs_get_equivalent_obj(obj);

	if(obj && buf){
	    	buf->st_dev = (int)obj->my_dev->os_context;
	    	buf->st_ino = obj->obj_id;
	    	buf->st_mode = obj->yst_mode & ~S_IFMT; /* clear out file type bits */

	    	if(obj->variant_type == YAFFS_OBJECT_TYPE_DIRECTORY)
			buf->st_mode |= S_IFDIR;
		else if(obj->variant_type == YAFFS_OBJECT_TYPE_SYMLINK)
			buf->st_mode |= S_IFLNK;
		else if(obj->variant_type == YAFFS_OBJECT_TYPE_FILE)
			buf->st_mode |= S_IFREG;

	    	buf->st_nlink = yaffs_get_obj_link_count(obj);
	    	buf->st_uid = 0;
	    	buf->st_gid = 0;;
	    	buf->st_rdev = obj->yst_rdev;
	    	buf->st_size = yaffs_get_obj_length(obj);
	    	buf->st_blksize = obj->my_dev->data_bytes_per_chunk;
	    	buf->st_blocks = (buf->st_size + buf->st_blksize -1)/buf->st_blksize;
#if CONFIG_YAFFS_WINCE
		buf->yst_wince_atime[0] = obj->win_atime[0];
		buf->yst_wince_atime[1] = obj->win_atime[1];
		buf->yst_wince_ctime[0] = obj->win_ctime[0];
		buf->yst_wince_ctime[1] = obj->win_ctime[1];
		buf->yst_wince_mtime[0] = obj->win_mtime[0];
		buf->yst_wince_mtime[1] = obj->win_mtime[1];
#else
    		buf->yst_atime = obj->yst_atime;
	    	buf->yst_ctime = obj->yst_ctime;
	    	buf->yst_mtime = obj->yst_mtime;
#endif
		retVal = 0;
	}
	return retVal;
}

static int yaffsfs_DoStatOrLStat(const char *path, struct yaffs_stat *buf,int doLStat)
{
	struct yaffs_obj *obj;
	struct yaffs_obj *dir = NULL;

	int retVal = -1;
	int notDir = 0;
	int loop = 0;

	yaffsfs_Lock();
	obj = yaffsfs_find_object(NULL,path,0,1,&dir,&notDir,&loop);

	if(!doLStat && obj)
	{
		obj = yaffsfs_follow_link(obj,0,&loop);
	}

	if(obj)
	{
		retVal = yaffsfs_DoStat(obj,buf);
	}
	else
	{
		// todo error not found
		yaffsfs_set_error(-ENOENT);
	}

	yaffsfs_Unlock();

	return retVal;

}

static void yaffsfs_ReleaseInode(yaffsfs_Inode *in)
{
	struct yaffs_obj *obj;

	obj = in->iObj;

	if(obj->unlinked)
		yaffs_del_obj(obj);

	obj->my_inode = NULL;
	in->iObj = NULL;

}

static void yaffsfs_PutInode(int inodeId)
{
	if(inodeId >= 0 && inodeId < YAFFSFS_N_HANDLES){
		yaffsfs_Inode *in = & yaffsfs_inode[inodeId];
		in->count--;
		if(in->count <= 0){
			yaffsfs_ReleaseInode(in);
			in->count = 0;
		}
	}
}

static int yaffsfs_NewHandle(yaffsfs_Handle **hptr)
{
	int i;
	yaffsfs_Handle *h;

	for(i = 0; i < YAFFSFS_N_HANDLES; i++){
		h = &yaffsfs_handle[i];
		if(h->useCount < 1){
			memset(h,0,sizeof(yaffsfs_Handle));
			h->fdId=-1;
			h->useCount=1;
			if(hptr)
				*hptr = h;
			return i;
		}
	}
	return -1;
}

static int yaffsfs_NewHandleAndFileDes(void)
{
	int i;
	yaffsfs_FileDes *fd;
	yaffsfs_Handle  *h = NULL;
	int handle = yaffsfs_NewHandle(&h);

	if(handle < 0)
		return -1;

	for(i = 0; i < YAFFSFS_N_HANDLES; i++){
		fd = &yaffsfs_fd[i];
		if(fd->handleCount < 1){
			memset(fd,0,sizeof(yaffsfs_FileDes));
			fd->inodeId=-1;
			fd->handleCount=1;
			h->fdId = i;
			return handle;
		}
	}

	/* Dump the handle because we could not get a fd */
	h->useCount = 0;
	return -1;
}

/*
 * yaffs_get_handle
 * Increase use of handle when reading/writing a file
 * Also gets the file descriptor.
 */

static int yaffsfs_GetHandle(int handle)
{
	yaffsfs_Handle *h = yaffsfs_HandleToPointer(handle);

	if(h && h->useCount > 0){
		h->useCount++;
		return 0;
	}
	return -1;
}

/*
 * yaffs_put_handle
 * Let go of a handle when closing a file or aborting an open or
 * ending a read or write.
 */

static int yaffsfs_PutFileDes(int fdId)
{
	yaffsfs_FileDes *fd;

	if(fdId >= 0 && fdId < YAFFSFS_N_HANDLES){
		fd = &yaffsfs_fd[fdId];
		fd->handleCount--;
		if(fd->handleCount < 1){
			if(fd->inodeId >= 0){
				yaffsfs_PutInode(fd->inodeId);
				fd->inodeId = -1;
			}
		}
	}
	return 0;
}

static int yaffsfs_PutHandle(int handle)
{
	yaffsfs_Handle *h = yaffsfs_HandleToPointer(handle);

	if(h && h->useCount > 0){
		h->useCount--;
		if(h->useCount < 1){
			yaffsfs_PutFileDes(h->fdId);
			h->fdId = -1;
		}
	}

	return 0;
}


int yaffs_open_sharing(const YCHAR *path, int oflag, int mode, int sharing)
{
	struct yaffs_obj *obj = NULL;
	struct yaffs_obj *dir = NULL;
	YCHAR *name;
	int handle = -1;
	yaffsfs_FileDes *fd = NULL;
	int openDenied = 0;
	int symDepth = 0;
	int errorReported = 0;
	int rwflags = oflag & ( O_RDWR | O_RDONLY | O_WRONLY);
	u8 shareRead = (sharing & YAFFS_SHARE_READ) ?  1 : 0;
	u8 shareWrite = (sharing & YAFFS_SHARE_WRITE) ? 1 : 0;
	u8 sharedReadAllowed;
	u8 sharedWriteAllowed;
	u8 alreadyReading;
	u8 alreadyWriting;
	u8 readRequested;
	u8 writeRequested;
	int notDir = 0;
	int loop = 0;

	if(!path) {
		yaffsfs_set_error(-EFAULT);
		return -1;
	}

	if(yaffsfs_CheckPath(path) < 0){
		yaffsfs_set_error(-ENAMETOOLONG);
		return -1;
	}

	/* O_EXCL only has meaning if O_CREAT is specified */
	if(!(oflag & O_CREAT))
		oflag &= ~(O_EXCL);

	/* O_TRUNC has no meaning if (O_CREAT | O_EXCL) is specified */
	if( (oflag & O_CREAT) & (oflag & O_EXCL))
		oflag &= ~(O_TRUNC);

	/* Todo: Are there any more flag combos to sanitise ? */

	/* Figure out if reading or writing is requested */

	readRequested = (rwflags == O_RDWR || rwflags == O_RDONLY) ? 1 : 0;
	writeRequested = (rwflags == O_RDWR || rwflags == O_WRONLY) ? 1 : 0;

	yaffsfs_Lock();

	handle = yaffsfs_NewHandleAndFileDes();

	if(handle < 0){
		yaffsfs_set_error(-ENFILE);
		errorReported = 1;
	} else {

		fd = yaffsfs_HandleToFileDes(handle);

		/* try to find the exisiting object */
		obj = yaffsfs_find_object(NULL,path,0,1,NULL,NULL,NULL);

		obj = yaffsfs_follow_link(obj,symDepth++,&loop);

		if(obj &&
			obj->variant_type != YAFFS_OBJECT_TYPE_FILE &&
			obj->variant_type != YAFFS_OBJECT_TYPE_DIRECTORY)
			obj = NULL;


		if(obj){

			/* The file already exists or it might be a directory */

			/* If it is a directory then we can't open it as a file */
			if(obj->variant_type == YAFFS_OBJECT_TYPE_DIRECTORY){
				openDenied = 1;
				yaffsfs_set_error(-EISDIR);
				errorReported = 1;
			}

			/* Open should fail if O_CREAT and O_EXCL are specified since
			 * the file exists
			 */
			if(!errorReported && (oflag & O_EXCL) && (oflag & O_CREAT)){
				openDenied = 1;
				yaffsfs_set_error(-EEXIST);
				errorReported = 1;
			}

			/* Check file permissions */
			if( readRequested && !(obj->yst_mode & S_IREAD))
				openDenied = 1;

			if( writeRequested && !(obj->yst_mode & S_IWRITE))
				openDenied = 1;

			if( !errorReported && writeRequested &&
				obj->my_dev->read_only){
				openDenied = 1;
				yaffsfs_set_error(-EROFS);
				errorReported = 1;
			}

			if(openDenied && !errorReported ) {
				/* Error if the file exists but permissions are refused. */
				yaffsfs_set_error(-EACCES);
				errorReported = 1;
			}

			/* Check sharing of an existing object. */
			if(!openDenied){
				yaffsfs_FileDes *fdx;
				int i;

				sharedReadAllowed = 1;
				sharedWriteAllowed = 1;
				alreadyReading = 0;
				alreadyWriting = 0;
				for( i = 0; i < YAFFSFS_N_HANDLES; i++){
					fdx = &yaffsfs_fd[i];
					if(fdx->handleCount > 0 &&
						fdx->inodeId >= 0 &&
						yaffsfs_inode[fdx->inodeId].iObj == obj){
						if(!fdx->shareRead)
							sharedReadAllowed = 0;
						if(!fdx->shareWrite)
							sharedWriteAllowed = 0;
						if(fdx->reading)
							alreadyReading = 1;
						if(fdx->writing)
							alreadyWriting = 1;
					}
				}



				if((!sharedReadAllowed && readRequested)||
					(!shareRead  && alreadyReading) ||
					(!sharedWriteAllowed && writeRequested) ||
					(!shareWrite && alreadyWriting)){
					openDenied = 1;
					yaffsfs_set_error(-EBUSY);
					errorReported=1;
				}
			}

		}

		/* If we could not open an existing object, then let's see if
		 * the directory exists. If not, error.
		 */
		if(!obj && !errorReported){
			dir = yaffsfs_find_directory(NULL,path,&name,0,&notDir,&loop);
			if(!dir && notDir){
				yaffsfs_set_error(-ENOTDIR);
				errorReported = 1;
			} else if(loop){
				yaffsfs_set_error(-ELOOP);
				errorReported = 1;
			} else	if(!dir){
				yaffsfs_set_error(-ENOENT);
				errorReported = 1;
			}
		}

		if(!obj && dir && !errorReported && (oflag & O_CREAT)) {
			/* Let's see if we can create this file if it does not exist. */
			if(dir->my_dev->read_only){
				yaffsfs_set_error(-EROFS);
				errorReported = 1;
			} else
				obj = yaffs_create_file(dir,name,mode,0,0);

			if(!obj && !errorReported){
				yaffsfs_set_error(-ENOSPC);
				errorReported = 1;
			}
		}

		if(!obj && dir && !errorReported && !(oflag & O_CREAT)) {
			/* Error if the file does not exist and CREAT is not set. */
			yaffsfs_set_error(-ENOENT);
			errorReported = 1;
		}

		if(obj && !openDenied) {
			int inodeId = yaffsfs_GetInodeIdForObject(obj);

			if(inodeId<0) {
				/*
				 * Todo: Fix any problem if inodes run out, though that
				 * can't happen if the number of inode items >= number of handles.
				 */
			}

			fd->inodeId = inodeId;
			fd->reading = readRequested;
			fd->writing = writeRequested;
			fd->append =  (oflag & O_APPEND) ? 1 : 0;
			fd->position = 0;
			fd->shareRead = shareRead;
			fd->shareWrite = shareWrite;

			/* Hook inode to object */
                        obj->my_inode = (void*) &yaffsfs_inode[inodeId];

                        if((oflag & O_TRUNC) && fd->writing)
                                yaffs_resize_file(obj,0);
		} else {
			yaffsfs_PutHandle(handle);
			if(!errorReported)
				yaffsfs_set_error(0); /* Problem */
			handle = -1;
		}
	}

	yaffsfs_Unlock();

	return handle;
}

int yaffs_open(const YCHAR *path, int oflag, int mode)
{
	return yaffs_open_sharing(path, oflag, mode,
			YAFFS_SHARE_READ | YAFFS_SHARE_WRITE);
}

int yaffs_stat(const char *path, struct yaffs_stat *buf)
{
	return yaffsfs_DoStatOrLStat(path,buf,0);
}

int yaffs_close(int handle)
{
	yaffsfs_Handle *h = NULL;
	struct yaffs_obj *obj = NULL;
	int retVal = -1;

	yaffsfs_Lock();

	h = yaffsfs_HandleToPointer(handle);
	obj = yaffsfs_HandleToObject(handle);

	if(!h  || !obj)
		yaffsfs_set_error(-EBADF);
	else {
		/* clean up */
		yaffs_flush_file(obj,1,0);
		yaffsfs_PutHandle(handle);
		retVal = 0;
	}

	yaffsfs_Unlock();

	return retVal;
}

int yaffsfs_do_read(int handle, void *vbuf, unsigned int nbyte, int isPread, int offset)
{
	yaffsfs_FileDes *fd = NULL;
	struct yaffs_obj *obj = NULL;
	int pos = 0;
	int startPos = 0;
	int endPos = 0;
	int nRead = 0;
	int nToRead = 0;
	int totalRead = 0;
	unsigned int maxRead;
	u8 *buf = (u8 *)vbuf;

	if(!vbuf){
		yaffsfs_set_error(-EFAULT);
		return -1;
	}

	yaffsfs_Lock();
	fd = yaffsfs_HandleToFileDes(handle);
	obj = yaffsfs_HandleToObject(handle);

	if(!fd || !obj){
		/* bad handle */
		yaffsfs_set_error(-EBADF);
		totalRead = -1;
	} else if(!fd->reading){
		/* Not a reading handle */
		yaffsfs_set_error(-EINVAL);
		totalRead = -1;
	} else if(nbyte > YAFFS_MAX_FILE_SIZE){
		yaffsfs_set_error(-EINVAL);
		totalRead = -1;
	} else {
		if(isPread)
			startPos = offset;
		else
			startPos = fd->position;

		pos = startPos;

		if(yaffs_get_obj_length(obj) > pos)
			maxRead = yaffs_get_obj_length(obj) - pos;
		else
			maxRead = 0;

		if(nbyte > maxRead)
			nbyte = maxRead;


		yaffsfs_GetHandle(handle);

		endPos = pos + nbyte;

		if(pos < 0 || pos > YAFFS_MAX_FILE_SIZE ||
			nbyte > YAFFS_MAX_FILE_SIZE ||
			endPos < 0 || endPos > YAFFS_MAX_FILE_SIZE){
			totalRead = -1;
			nbyte = 0;
		}

		while(nbyte > 0) {
			nToRead = YAFFSFS_RW_SIZE - (pos & (YAFFSFS_RW_SIZE -1));
			if(nToRead > nbyte)
				nToRead = nbyte;

			/* Tricky bit...
			 * Need to reverify object in case the device was
			 * unmounted in another thread.
			 */
			obj = yaffsfs_HandleToObject(handle);
			if(!obj)
				nRead = 0;
			else
				nRead = yaffs_file_rd(obj,buf,pos,nToRead);

			if(nRead > 0){
				totalRead += nRead;
				pos += nRead;
				buf += nRead;
			}

			if(nRead == nToRead)
				nbyte-=nRead;
			else
				nbyte = 0; /* no more to read */


			if(nbyte > 0){
				yaffsfs_Unlock();
				yaffsfs_Lock();
			}

		}

		yaffsfs_PutHandle(handle);

		if(!isPread) {
			if(totalRead >= 0)
				fd->position = startPos + totalRead;
			else
				yaffsfs_set_error(-EINVAL);
		}

	}

	yaffsfs_Unlock();

	return (totalRead >= 0) ? totalRead : -1;

}

int yaffs_read(int handle, void *buf, unsigned int nbyte)
{
	return yaffsfs_do_read(handle, buf, nbyte, 0, 0);
}


int yaffs_mkdir(const YCHAR *path, mode_t mode)
{
	struct yaffs_obj *parent = NULL;
	struct yaffs_obj *dir = NULL;
	YCHAR *name;
	YCHAR *alt_path = NULL;
	int retVal= -1;
	int notDir = 0;
	int loop = 0;

	if(!path){
		yaffsfs_set_error(-EFAULT);
		return -1;
	}

	if(yaffsfs_CheckPath(path) < 0){
		yaffsfs_set_error(-ENAMETOOLONG);
		return -1;
	}

	if(yaffsfs_alt_dir_path(path, &alt_path) < 0){
		yaffsfs_set_error(-ENOMEM);
		return -1;
	}
	if(alt_path)
		path = alt_path;

	yaffsfs_Lock();
	parent = yaffsfs_find_directory(NULL,path,&name,0,&notDir,&loop);
	if(!parent && notDir)
		yaffsfs_set_error(-ENOTDIR);
	else if(loop)
		yaffsfs_set_error(-ELOOP);
	else if(!parent)
		yaffsfs_set_error(-ENOENT);
	else if(strnlen(name,5) == 0){
		/* Trying to make the root itself */
		yaffsfs_set_error(-EEXIST);
	} else if(parent->my_dev->read_only)
		yaffsfs_set_error(-EROFS);
	else {
		dir = yaffs_create_dir(parent,name,mode,0,0);
		if(dir)
			retVal = 0;
		else if (yaffs_find_by_name(parent,name))
			yaffsfs_set_error(-EEXIST); /* the name already exists */
		else
			yaffsfs_set_error(-ENOSPC); /* just assume no space */
	}

	yaffsfs_Unlock();

	if(alt_path)
		kfree(alt_path);

	return retVal;
}

int yaffs_rmdir(const YCHAR *path)
{
	int result;
	YCHAR *alt_path;

	if(!path){
		yaffsfs_set_error(-EFAULT);
		return -1;
	}

	if(yaffsfs_CheckPath(path) < 0){
		yaffsfs_set_error(-ENAMETOOLONG);
		return -1;
	}

	if(yaffsfs_alt_dir_path(path, &alt_path) < 0){
		yaffsfs_set_error(-ENOMEM);
		return -1;
	}
	if(alt_path)
		path = alt_path;
	result =  yaffsfs_do_unlink(path,1);
	if(alt_path)
		kfree(alt_path);
	return result;
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

void yaffsfs_initialise(void)
{
	yaffsfs_InitHandles();
}

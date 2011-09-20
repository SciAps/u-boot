/*
 * YAFFS: Yet another Flash File System . A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2007 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1 as
 * published by the Free Software Foundation.
 *
 * Note: Only YAFFS headers are LGPL, YAFFS C code is covered by GPL.
 */


#ifndef __YPORTENV_H__
#define __YPORTENV_H__

/* XXX U-BOOT XXX */
#ifndef CONFIG_YAFFS_DIRECT
#define CONFIG_YAFFS_DIRECT
#endif

#if defined CONFIG_YAFFS_WINCE

#include "ywinceenv.h"

/* XXX U-BOOT XXX */
#elif  0 /* defined __KERNEL__ */

#include "moduleconfig.h"

/* Linux kernel */
#include <linux/version.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
#include <linux/config.h>
#endif
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>

#define YCHAR char
#define YUCHAR unsigned char
#define _Y(x)     x
#define yaffs_strcpy(a,b)    strcpy(a,b)
#define yaffs_strncpy(a,b,c) strncpy(a,b,c)
#define yaffs_strncmp(a,b,c) strncmp(a,b,c)
#define yaffs_strlen(s)	     strlen(s)
#define yaffs_sprintf	     sprintf
#define yaffs_toupper(a)     toupper(a)

#define Y_INLINE inline

#define YAFFS_LOSTNFOUND_NAME		"lost+found"
#define YAFFS_LOSTNFOUND_PREFIX		"obj"

/* #define YPRINTF(x) printk x */
#define YMALLOC(x) kmalloc(x,GFP_KERNEL)
#define YFREE(x)   kfree(x)
#define YMALLOC_ALT(x) vmalloc(x)
#define YFREE_ALT(x)   vfree(x)
#define YMALLOC_DMA(x) YMALLOC(x)

// KR - added for use in scan so processes aren't blocked indefinitely.
#define YYIELD() schedule()

#define YAFFS_ROOT_MODE			0666
#define YAFFS_LOSTNFOUND_MODE		0666

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,5,0))
#define Y_CURRENT_TIME CURRENT_TIME.tv_sec
#define Y_TIME_CONVERT(x) (x).tv_sec
#else
#define Y_CURRENT_TIME CURRENT_TIME
#define Y_TIME_CONVERT(x) (x)
#endif

#define yaffs_SumCompare(x,y) ((x) == (y))
#define yaffs_strcmp(a,b) strcmp(a,b)

#define TENDSTR "\n"
#define TSTR(x) KERN_WARNING x
#define TOUT(p) printk p

#define yaffs_trace(mask, fmt, args...) \
	do { if ((mask) & (yaffs_traceMask|YAFFS_TRACE_ERROR)) \
		printk(KERN_WARNING "yaffs: " fmt, ## args); \
	} while (0)

#define compile_time_assertion(assertion) \
	({ int x = __builtin_choose_expr(assertion, 0, (void)0); (void) x; })

#elif defined CONFIG_YAFFS_DIRECT

#define YAFFS_DEBUG_MALLOC /* Complain if yaffs_malloc() fails */

/* Direct interface */
#include "ydirectenv.h"

#elif defined CONFIG_YAFFS_UTIL

/* Stuff for YAFFS utilities */

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "devextras.h"

#define YMALLOC(x) malloc(x)
#define YFREE(x)   free(x)
#define YMALLOC_ALT(x) malloc(x)
#define YFREE_ALT(x) free(x)

#define YCHAR char
#define YUCHAR unsigned char
#define _Y(x)     x
#define yaffs_strcpy(a,b)    strcpy(a,b)
#define yaffs_strncpy(a,b,c) strncpy(a,b,c)
#define yaffs_strlen(s)	     strlen(s)
#define yaffs_sprintf	     sprintf
#define yaffs_toupper(a)     toupper(a)

#define Y_INLINE inline

/* #define YINFO(s) YPRINTF(( __FILE__ " %d %s\n",__LINE__,s)) */
/* #define YALERT(s) YINFO(s) */

#define TENDSTR "\n"
#define TSTR(x) x
#define TOUT(p) printf p

#define YAFFS_LOSTNFOUND_NAME		"lost+found"
#define YAFFS_LOSTNFOUND_PREFIX		"obj"
/* #define YPRINTF(x) printf x */

#define YAFFS_ROOT_MODE				0666
#define YAFFS_LOSTNFOUND_MODE		0666

#define yaffs_SumCompare(x,y) ((x) == (y))
#define yaffs_strcmp(a,b) strcmp(a,b)

#else
/* Should have specified a configuration type */
#error Unknown configuration

#endif

/* Mode flags */

#ifndef XATTR_CREATE
#define XATTR_CREATE	1
#endif

#ifndef XATTR_REPLACE
#define XATTR_REPLACE	2
#endif

/* Errno entries */
#ifndef ENOENT
#define ENOENT	2
#endif

#ifndef ENOMEM
#define ENOMEM	12
#endif

#ifndef EEXIST
#define EEXIST	17
#endif

#ifndef ENOSPC
#define ENOSPC	28
#endif

#ifndef ERANGE
#define ERANGE	34
#endif

#ifndef ENODATA
#define ENODATA	61
#endif

/* see yaffs_fs.c */
extern unsigned int yaffs_traceMask;
extern unsigned int yaffs_wr_attempts;


#ifdef CONFIG_YAFFS_DEBUG
#define yaffs_trace(msk, fmt, ...) do {		\
	if (yaffs_trace_mask & (msk)) \
		printf("yaffs: " fmt "\n", ##__VA_ARGS); \
	} while (0)
#else
#define yaffs_trace(msk, fmt, ...) do { \
	} while (0)
#endif

#endif

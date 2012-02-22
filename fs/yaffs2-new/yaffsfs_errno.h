/*
 * (C) Copyright 2011 Logic Product Development, Inc.
 * Author :
 *	Peter Barada <peter.barada@logicpd.com>
 *
 * YAFFS Errno handling
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

extern void yaffsfs_set_error(int x);
extern int yaffsfs_get_error(char **errstr);

#ifndef EPERM
#define EPERM	1
#endif

#ifndef ENOENT
#define ENOENT	2
#endif

#ifndef EBADF
#define EBADF	9
#endif

#ifndef ENOMEM
#define ENOMEM	12
#endif

#ifndef EACCES
#define EACCES	13
#endif

#ifndef EFAULT
#define EFAULT	14
#endif

#ifndef EBUSY
#define EBUSY	16
#endif

#ifndef EEXIST
#define EEXIST	17
#endif

#ifndef EXDEV
#define EXDEV	18
#endif

#ifndef ENODEV
#define ENODEV	19
#endif

#ifndef ENOTDIR
#define ENOTDIR	20
#endif

#ifndef EISDIR
#define EISDIR	21
#endif

#ifndef EINVAL
#define EINVAL	22
#endif

#ifndef ENFILE
#define ENFILE	23
#endif

#ifndef ENOSPC
#define ENOSPC	28
#endif

#ifndef EROFS
#define EROFS	30
#endif

#ifndef ERANGE
#define ERANGE	34
#endif

#ifndef ENAMETOOLONG
#define ENAMETOOLONG	36
#endif

#ifndef ENOTEMPTY
#define ENOTEMPTY	39
#endif

#ifndef ELOOP
#define ELOOP	40
#endif

#ifndef ENODATA
#define ENODATA	61
#endif

#ifndef EBADMSG
#define EBADMSG	74
#endif

#ifndef EUCLEAN
#define EUCLEAN	117
#endif

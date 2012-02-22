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

#include <common.h>
#include "yaffsfs_errno.h"

int yaffsfs_errno;
char *yaffsfs_errstr;
void yaffsfs_set_error(int x)
{
	yaffsfs_errno = x;
	switch(x) {
	case 0:
		yaffsfs_errstr = "Success";
		break;
	case -EPERM:
		yaffsfs_errstr = "Operation not permitted";
		break;
	case -ENOENT:
		yaffsfs_errstr = "No such file or directory";
		break;
	case -EBADF:
		yaffsfs_errstr = "Bad file number";
		break;
	case -ENOMEM:
		yaffsfs_errstr = "Out of memory";
		break;
	case -EACCES:
		yaffsfs_errstr = "Permission denied";
		break;
	case -EFAULT:
		yaffsfs_errstr = "Bad address";
		break;
	case -EBUSY:
		yaffsfs_errstr = "Device or resource busy";
		break;
	case -EEXIST:
		yaffsfs_errstr = "File exists";
		break;
	case -ENODEV:
		yaffsfs_errstr = "No such device";
		break;
	case -ENOTDIR:
		yaffsfs_errstr = "Not a directory";
		break;
	case -EISDIR:
		yaffsfs_errstr = "Is a directory";
		break;
	case -EINVAL:
		yaffsfs_errstr = "Invalid argument";
		break;
	case -ENFILE:
		yaffsfs_errstr = "File table overflow";
		break;
	case -ENOSPC:
		yaffsfs_errstr = "No space left on device";
		break;
	case -EROFS:
		yaffsfs_errstr = "Read-only file system";
		break;
	case -ERANGE:
		yaffsfs_errstr = "Math result not representable";
		break;
	case -ENAMETOOLONG:
		yaffsfs_errstr = "File name too long";
		break;
	case -ENOTEMPTY:
		yaffsfs_errstr = "Directory not empty";
		break;
	case -ELOOP:
		yaffsfs_errstr = "Too many symbolic links encountered";
		break;
	case -ENODATA:
		yaffsfs_errstr = "No data available";
		break;
	case -EBADMSG:
		yaffsfs_errstr = "Not a data message";
		break;
	case -EUCLEAN:
		yaffsfs_errstr = "Structure needs cleaning";
		break;
	default:
		printf("%s: Huh? yaffs_errno called from %p with %d\n",
			__FUNCTION__, __builtin_return_address(0), x);
		yaffsfs_errstr = "Unknown!";
		break;
	}
}

int yaffsfs_get_error(char **errstr)
{
	if (errstr)
		*errstr = yaffsfs_errstr;
	return yaffsfs_errno;
}


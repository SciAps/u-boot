int yaffsfs_errno;

void yaffsfs_set_error(int x)
{
	yaffsfs_errno = x;
}

int yaffsfs_get_error(void)
{
	return yaffsfs_errno;
}


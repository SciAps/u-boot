
extern int cmd_yaffs_mount(const char *path);
extern int cmd_yaffs_unmount(const char *path);
extern int cmd_yaffs_ls(const char *mountpt, int longlist);
extern int cmd_yaffs_df(const char *path, loff_t *size);
extern int cmd_yaffs_mwrite_file(char *fn, char *addr, int size);
extern int cmd_yaffs_mread_file(char *fn, char *addr, long *size);
extern int cmd_yaffs_mkdir(const char *dir);
extern int cmd_yaffs_rmdir(const char *dir);
extern int cmd_yaffs_rm(const char *path);
extern int cmd_yaffs_mv(const char *oldPath, const char *newPath);

extern int yaffsfs_mount(const char *path);
extern int yaffsfs_unmount(const char *path);

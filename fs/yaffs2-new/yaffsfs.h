
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


#define YAFFS_SHARE_READ	1
#define YAFFS_SHARE_WRITE	2

#define YAFFS_MAX_FILE_SIZE (0x7ffffff)

struct yaffs_dirent{
    long d_ino;                 /* inode number */
    off_t d_off;                /* offset to this dirent */
    unsigned short d_reclen;    /* length of this d_name */
    char d_name [NAME_MAX+1];   /* file name (null-terminated) */
    unsigned d_dont_use;	/* debug pointer, not for public consumption */
};

typedef struct yaffs_dirent yaffs_dirent;

	
typedef struct __opaque yaffs_DIR;

struct yaffs_stat {
    int		      st_dev;      /* device */
    int           st_ino;      /* inode */
    mode_t        st_mode;     /* protection */
    int           st_nlink;    /* number of hard links */
    int           st_uid;      /* user ID of owner */
    int           st_gid;      /* group ID of owner */
    unsigned      st_rdev;     /* device type (if inode device) */
    off_t         st_size;     /* total size, in bytes */
    unsigned long st_blksize;  /* blocksize for filesystem I/O */
    unsigned long st_blocks;   /* number of blocks allocated */
    unsigned long yst_atime;    /* time of last access */
    unsigned long yst_mtime;    /* time of last modification */
    unsigned long yst_ctime;    /* time of last change */
};

extern int yaffs_open(const char *path, int oflag, int mode) ;
extern int yaffs_read(int fd, void *buf, unsigned int nbyte) ;
extern int yaffs_close(int fd);
extern yaffs_DIR *yaffs_opendir(const char *dirname);
extern struct yaffs_dirent *yaffs_readdir(yaffs_DIR *dirp);
extern int yaffs_stat(const char *path, struct yaffs_stat *buf) ;
extern int yaffs_mkdir(const char *path, mode_t mode);
extern int yaffs_rmdir(const char *path);
extern int yaffs_rename(const char *new_path, const char *old_path);

extern void yaffsfs_initialise(void);

#include <common.h>

#include <config.h>
#include <command.h>

#ifdef  YAFFS2_DEBUG
#define PRINTF(fmt,args...) printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

extern void cmd_yaffs_mount(char *mp);
extern void cmd_yaffs_umount(char *mp);
extern void cmd_yaffs_ls(const char *mountpt, int longlist);
extern int cmd_yaffs_df(const char *path, loff_t *size);
extern void cmd_yaffs_mwrite_file(char *fn, char *addr, int size);
extern void cmd_yaffs_mread_file(char *fn, char *addr, long *size);
extern void cmd_yaffs_mkdir(const char *dir);
extern void cmd_yaffs_rmdir(const char *dir);
extern void cmd_yaffs_rm(const char *path);
extern void cmd_yaffs_mv(const char *oldPath, const char *newPath);

extern int yaffs_DumpDevStruct(const char *path);


int do_ymount (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    if (argc < 2)
	    return cmd_usage(cmdtp);

    cmd_yaffs_mount(argv[1]);

    return(0);
}

int do_yumount (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    if (argc < 2)
	    return cmd_usage(cmdtp);

    cmd_yaffs_umount(argv[1]);

    return(0);
}

int do_yls (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    if (argc < 2)
	    return cmd_usage(cmdtp);
    cmd_yaffs_ls(argv[argc-1], (argc>2)?1:0);

    return(0);
}

int do_ydf (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    loff_t space;
    int ret;

    if (argc < 2)
	    return cmd_usage(cmdtp);

    ret = cmd_yaffs_df(argv[1], &space);
    if (!ret)
	    printf("Free Space: %lluKiB\n", space / 1024);

    return !!ret;
}


int do_yrdm (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    long size;
    char buf[12];
    char *filename;
    ulong addr;

    if (argc < 3)
	    return cmd_usage(cmdtp);

    filename = argv[2];
    addr = simple_strtoul(argv[1], NULL, 16);

    cmd_yaffs_mread_file(filename, (char *)addr, &size);

    sprintf(buf, "%lX", size);
    setenv("filesize", buf);
    return(0);
}

int do_ywrm (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    char *filename;
    ulong addr;
    ulong size;

    if (argc < 4)
	    return cmd_usage(cmdtp);

    filename = argv[1];
    addr = simple_strtoul(argv[2], NULL, 16);
    size = simple_strtoul(argv[3], NULL, 16);

    cmd_yaffs_mwrite_file(filename, (char *)addr, size);

    return(0);
}

int do_ymkdir (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    if (argc < 2)
	    return cmd_usage(cmdtp);
    
    cmd_yaffs_mkdir(argv[1]);

    return(0);
}

int do_yrmdir (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    if (argc < 2)
	    return cmd_usage(cmdtp);

    cmd_yaffs_rmdir(argv[1]);

    return(0);
}

int do_yrm (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    if (argc < 2)
	    return cmd_usage(cmdtp);

    cmd_yaffs_rm(argv[1]);

    return(0);
}

int do_ymv (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    char *oldPath;
    char *newPath;

    if (argc < 3)
	    return cmd_usage(cmdtp);

    oldPath = argv[1];
    newPath = argv[2];

    cmd_yaffs_mv(newPath, oldPath);

    return(0);
}

int do_ydump (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    char *dirname;

    if (argc < 2)
	    return cmd_usage(cmdtp);

    dirname = argv[1];
    if (yaffs_DumpDevStruct(dirname) != 0)
	printf("yaffs_DumpDevStruct returning error when dumping path: , %s\n", dirname);
    return 0;
}

U_BOOT_CMD(
    ymount, 3,  0,  do_ymount,
    "YAFFS mount",
    "<partition>"
);

U_BOOT_CMD(
    yumount, 3,  0,  do_yumount,
    "YAFFS unmount",
    "<partition>"
);

U_BOOT_CMD(
    yls,    4,  0,  do_yls,
    "YAFFS ls",
    "[-l] <name>"
);

U_BOOT_CMD(
    ydf,    2,  0,  do_ydf,
    "YAFFS disk free",
    "[-l] <dirname>"
);

U_BOOT_CMD(
    yrdm,   3,  0,  do_yrdm,
    "YAFFS read file to memory",
    "<addr> <filename>"
);

U_BOOT_CMD(
    ywrm,   4,  0,  do_ywrm,
    "YAFFS write file from memory",
    "<filename> <addr> <size>"
);

U_BOOT_CMD(
    ymkdir, 2,  0,  do_ymkdir,
    "YAFFS mkdir",
    "<dirname>"
);

U_BOOT_CMD(
    yrmdir, 2,  0,  do_yrmdir,
    "YAFFS rmdir",
    "<dirname>"
);

U_BOOT_CMD(
    yrm,    2,  0,  do_yrm,
    "YAFFS rm",
    "<path>"
);

U_BOOT_CMD(
    ymv,    4,  0,  do_ymv,
    "YAFFS mv",
    "oldPath newPath"
);

U_BOOT_CMD(
    ydump,  2,  0,  do_ydump,
    "YAFFS device struct",
    "dirname"
);

extern void yaffsfs_set_error(int x);
extern int yaffsfs_get_error(void);

#ifndef ENOENT
#define ENOENT	2
#endif

#ifndef ENOMEM
#define ENOMEM	12
#endif

#ifndef EBUSY
#define EBUSY	16
#endif

#ifndef EEXIST
#define EEXIST	17
#endif

#ifndef ENODEV
#define ENODEV	18
#endif

#ifndef EINVAL
#define EINVAL	22
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

#ifndef EBADMSG
#define EBADMSG	74
#endif

#ifndef EUCLEAN
#define EUCLEAN	117
#endif

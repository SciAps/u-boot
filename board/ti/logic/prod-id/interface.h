/*
 * Header file that interfaces to environment to access data
 */

/* Create the enum list of keys, not strings! */
#undef ID_KEY_STRINGS
#define ID_KEY_ENUMS
#include "keys.h"


typedef enum {
	/* Number */
	IDENUM_NEG_NUM = 0,
	IDENUM_POS_NUM,

	/* String/Hex String */
	IDENUM_STR,
	IDENUM_HEXSTR,

	/* Array */
	IDENUM_ARRAY,

	/* Dictionary */
	IDENUM_DICT,

	/* Key */
	IDENUM_KEY,

	/* Any string */
	IDENUM_ANY_STRING,

	/* Any number */
	IDENUM_ANY_NUMBER,

} idenum_t;

/* structure of builtin keys */
struct id_key {
	unsigned char *ptr;
	unsigned int size;
};


/*
 * return a byte from the ID data at offset 'offset' and set *oor to zero
 * if offset is in range of the device.  If offset is out of range then
 * set *oor to non-zero.  If mem_ptr is non-NULL use it as a pointer to
 * a cached copy of the ID data */
extern unsigned char id_fetch_byte(unsigned char *mem_ptr, int offset, int *oor);
extern void id_fetch_bytes(unsigned char *mem_ptr, int offset, int size, int *oor);

struct id_data {
	/* mem_ptr is a pointer to read the initial ID data into, then if not
	 * null used to read a cached copy of the ID data from */
	unsigned char *mem_ptr;
	unsigned int root_size;
	unsigned int root_offset;
};

/* Function to do the intial startup (i.e. figure out how much data, offset of
 * key table, etc */
extern int id_startup(struct id_data *data,
		int (*setup_id_chip)(void),
		int (*shutdown_id_chip)(void));

/*
 * Functions provided back to callers for use in accessing data
 */

/* ID data "cookie" used to access data; ultimately this will be opaque
 * to the callers as they don't need to know whats in it, just pass it around
 */
struct id_cookie {
	unsigned char *mem_ptr;		/* cache pointer to ID data */
	unsigned int start_offset;	/* start offset from beginning of data */
	unsigned int size;		/* size of data in bytes */
	unsigned int offset;		/* current read offset */
};

/* Initialize the cookie to cover the whole root dictionary */
extern int id_init_cookie(struct id_data *data, struct id_cookie *cookie);

/* What is the read pointer cookie is pointing at */
extern int id_whatis(struct id_cookie *cookie, idenum_t *type);

/* Given a string, return the key code (or -1 if not found) */
extern int id_data_get_key(char *key_name);

/* Define to get id_dbg_printf() output... */
#undef DEBUG_NEW_ID_CODE

/* ID error routine to handle malformed data */
extern void id_error(const char *fmt, ...);

/* Ouptut routine */
extern int id_printf(const char *format, ...);

/* Ouptut debug routine; define DEBUG_NEW_ID_CODE to enable it */
#ifdef DEBUG_NEW_ID_CODE
extern int id_dbg_printf(const char *format, ...);
#else
#define id_dbg_printf(fmt, ...)
#endif


/* User interface functions */
extern int id_dict_size(struct id_data *data, struct id_cookie *cookie);
extern int id_array_size(struct id_data *data, struct id_cookie *cookie);

extern int id_dict_find_key(struct id_cookie *cookie, id_keys_t key);
extern int id_find_dict(struct id_cookie *cookie, id_keys_t key, idenum_t type);
extern int id_find_string(struct id_cookie *cookie, id_keys_t key, unsigned char *str_ptr, unsigned int *str_size);
extern int id_find_number(struct id_cookie *cookie, id_keys_t key, int *num);
extern int id_find_numbers(struct id_cookie *cookie, id_keys_t *key, int key_size, int *nums);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(var) sizeof(var)/sizeof((var)[0])
#endif

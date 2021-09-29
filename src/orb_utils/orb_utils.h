#ifndef ORB_UTILS_H
#define ORB_UTILS_H

#include <sys/types.h>
#include <json-c/json.h>
#include "../orb_types/orb_types.h"

#define SHA1_LEN       (20)
#define SHA1_EMPTY_STR "0000000000000000000000000000000000000000"

#define orb_try(PRED) if (!(PRED)) return false

typedef u8 * orb_sha1;

bool orb_dir_exist(const char * path);

bool orb_file_exist(const char * path);

bool orb_mkdir_p(const char * dir);

bool orb_rmrf(const char * path);

const char * orb_cat(const char * root, const char * file);

const char * orb_get_dir(const char * path);

bool orb_copy(const char * from, const char * to);

orb_sha1 orb_file_sha1(const char * path);

const char * orb_sha2str(orb_sha1 sha1);

struct orb_bts * orb_file_read(const char * path);

#endif /* ORB_UTILS_H */

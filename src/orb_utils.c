#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <openssl/sha.h>
#include "../src/orb_utils.h"
#include "../src/orb_log.h"

#define PATH_MAX_STRING_SIZE (B_KB(4))
#define RWBUF_SIZE           (B_KB(1))

#define DIR_USRM (S_IRWXU)
#define DIR_GRPM (S_IRWXG)
#define DIR_OTRM (S_IRWXO)
#define DIR_MODE (DIR_USRM | DIR_GRPM | DIR_OTRM)

bool orb_dir_exist(const char * path) {
    struct stat sb;
    if ( stat(path, &sb) == 0 ) {
        if (S_ISDIR (sb.st_mode))
            return true;
    }
    return false;
}

static inline bool _file_exist(const char * path) {
    struct stat sb;
    if (access(path, F_OK) != -1) {
        if ( stat(path, &sb) == 0 ) {
            if (!S_ISDIR(sb.st_mode))
                return true;
        } return false;
    }
    return false;
}

static char * _next_dir(char ** dir) {
    char c;
    u32 w = 0;
    static __thread char buff[B_KB(1)];

    memset(buff, 0, B_KB(1));

    do {
        c = **dir;
        if (c == '\0') {
            buff[w++] = '/';
            break;
        } else
            buff[w++] = c;

        ++(*dir);
    } while (c != '/');

    return buff;
}

bool orb_mkdir_p(const char * dir) {
    char * ptr = (char *)dir;
    char path[PATH_MAX_STRING_SIZE] = { 0 };

    if(!dir || dir[0] != '/') return false;

    while (*ptr) {
        strcat(path, _next_dir(&ptr));
        if (orb_dir_exist(path))
            continue;
        if (mkdir(path, DIR_MODE) < 0)
            return false;
    }

    return true;
}

const char * orb_cat(const char * root, const char * file) {
    static __thread char path[B_KB(4)];
    snprintf(path, B_KB(4), "%s/%s", root, file);
    return path;
}

const char * orb_get_dir(const char * path) {
    char c;
    char * ptr = (char*)path;
    char * lastsh = ptr;
    static __thread char buff[B_KB(4)];

    if (!path)
        return NULL;

    while ((c = *(ptr++))) {
        if (c == '/')
            lastsh = ptr;
    }

    memset(buff, 0, B_KB(4));
    memcpy(buff, path, lastsh - path);

    return buff;
}

static bool _create_dir_for_file(const char * path) {
    const char * dir = orb_get_dir(path);
    return orb_mkdir_p(dir);
}

inline static void _try_close(i32 fd, const char * path) {
    if (close(fd) == -1)
        orb_err("error while close file %s", path);
}

bool orb_copy(const char * from, const char * to) {
    i32 from_fd;
    i32 to_fd;
    bool res = true;
    ssize_t num_read;
    u8 buff[RWBUF_SIZE];

    if (!from || !to)
        return false;

    if(!_file_exist(from)) {
        orb_wrn("file %s not exist", from);
        return false;
    }

    if (!_create_dir_for_file(to)) {
        orb_wrn("dir %s not create", to);
        return false;
    }

    from_fd = open(from, O_RDONLY);
    if (from_fd == -1) {
        orb_err("error while open file %s", from);
        return false;
    }

    to_fd = open(to, O_CREAT | O_WRONLY | O_TRUNC,
                     S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (to_fd == -1) {
        orb_err("error while open file %s", to);
        _try_close(from_fd, from);
        return false;
    }

    while ((num_read = read(from_fd, buff, RWBUF_SIZE)) > 0)
        if (write(to_fd, buff, num_read) != num_read) {
            res = false;
            break;
        }

    if (num_read == -1)
        orb_err("error while read file %s", from);
    _try_close(from_fd, from);
    _try_close(to_fd, to);

    return res;
}

u8 * orb_file_sha1(const char * path) {
    i32 fd;
    ssize_t nread;
    SHA_CTX ctx;
    static __thread u8 sha[SHA_DIGEST_LENGTH];
    u8 buff[B_KB(4)];

    if(!path || !_file_exist(path)) {
        orb_wrn("file %s not exist", path);
        return NULL;
    }

    fd = open(path, O_RDONLY);
    if (fd == -1) {
        orb_err("error while open file %s", path);
        return NULL;
    }

    if (SHA1_Init(&ctx) != 1) {
        _try_close(fd, path);
        return NULL;
    }

    while ((nread = read(fd, buff, B_KB(4))) > 0) {
        if (SHA1_Update(&ctx, buff, nread) != 1) {
            _try_close(fd, path);
            return NULL;
        }
    }
    if (nread == -1)
        orb_err("error while read file %s", path);

    if (SHA1_Final(sha, &ctx) != 1) {
        _try_close(fd, path);
        return NULL;
    }
    _try_close(fd, path);

    return sha;
}

static void _write_half_byte(u8 half_byte, char * dst) {
    if (half_byte < 0x0a) *dst = '0' + half_byte;
    else *dst = 'a' + half_byte - 0x0a;
}

static void _write_byte(u8 byte, char ** dst) {
    u8 half_byte;

    half_byte = (byte >> 4) & 0x0F;
    _write_half_byte(half_byte, (*dst)++);
    half_byte = byte & 0x0F;
    _write_half_byte(half_byte, (*dst)++);
}

const char * orb_sha2str(u8 * sha1) {
    static __thread char buff[2 * SHA_DIGEST_LENGTH + 1];
    char * wptr = buff;

    if (!sha1)
        return NULL;

    buff[2 * SHA_DIGEST_LENGTH] = '\0';
    for(u32 i = 0; i < SHA_DIGEST_LENGTH; ++i)
        _write_byte(sha1[i], &wptr);

    return buff;
}

static size_t _file_size(FILE * file) {
    size_t size;
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    rewind (file);
    return size;
}

struct orb_bts * orb_file_read(const char * path) {
    size_t nread;
    FILE * file = NULL;
    struct orb_bts * bts;

    file = fopen(path, "r");
    if (!file) {
        orb_err("%s not opened", path);
        return NULL;
    }

    bts = orb_bts_malloc(_file_size(file));
    if (!bts) {
        fclose(file);
        return NULL;
    }

    nread = fread(bts->data, 1, bts->size , file);
    if (nread != bts->size) {
        orb_err("didn't read file completely");
        orb_bts_free(bts);
        bts = NULL;
    }

    fclose(file);
    return bts;
}

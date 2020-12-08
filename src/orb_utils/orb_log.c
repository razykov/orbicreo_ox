#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include "orb_log.h"

#define COL_SET "\033[1;3%dm"
#define COL_CLR "\033[0m"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


inline static void _paint(FILE * file, u8 colour) {
    fprintf(file, COL_SET, colour);
}

inline static void _clear(FILE * file) {
    fprintf(file, COL_CLR);
}

static char * _stime(void) {
    static char time_str[32];
    time_t now = time(NULL);

    struct tm * tm = localtime(&now);

    strftime(time_str, 32, "[%Y-%m-%d %H:%M:%S]", tm);
    return time_str;
}

void _cprint(FILE * stream, u8 colour, u8 flags,
             const char * func, const char * fmt, ...) {
    va_list args;

    pthread_mutex_lock(&mutex);

    if (flags & FLAG_TIME) {
        _paint(stream, ORB_COLOUR_A8_WHITE);
        fprintf(stream, "%s ", _stime());
        _clear(stream);
    }

    if (strlen(func)) {
        _paint(stream, ORB_COLOUR_A8_CYAN);
        fprintf(stream, "%s ", func);
        _clear(stream);
    }

    _paint(stream, colour);
        va_start(args, fmt);
        vfprintf(stream, fmt, args);
        va_end(args);
        if (flags & FLAG_ERROR)
            fprintf(stream, " (\"%s\" (%u))", strerror(errno), errno);
        fprintf(stream, "\n");
    _clear(stream);

    fflush(stream);

    pthread_mutex_unlock(&mutex);
}

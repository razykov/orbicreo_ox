#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include "orb_log.h"

#define COL_SET "\033[%sm"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


inline static void _paint(FILE * file, const char * colour)
{
    fprintf(file, COL_SET, colour);
}

void _cprint(FILE * stream, const char * colour, u8 flags,
             const char * func, const char * fmt, ...)
{
    va_list args;

    pthread_mutex_lock(&mutex);

    if (strlen(func)) {
        _paint(stream, ORB_COLOUR_CYAN);
        fprintf(stream, "%s ", func);
        _paint(stream, ORB_COLOUR_CLEAR);
    }

    _paint(stream, colour);
        va_start(args, fmt);
        vfprintf(stream, fmt, args);
        va_end(args);
        if (flags & FLAG_ERROR)
            fprintf(stream, " (\"%s\" (%u))", strerror(errno), errno);
        fprintf(stream, "\n");
    _paint(stream, ORB_COLOUR_CLEAR);

    fflush(stream);

    pthread_mutex_unlock(&mutex);
}

void _stprint(FILE * stream, const char * colour,
              const char * name, const char * fmt, ...)
{
    va_list args;

    pthread_mutex_lock(&mutex);

    _paint(stream, ORB_COLOUR_WHITE);
        if (name && strlen(name))
            fprintf(stream, "%s: ", name);
    _paint(stream, ORB_COLOUR_CLEAR);

    _paint(stream, colour);
        va_start(args, fmt);
        vfprintf(stream, fmt, args);
        va_end(args);
        fprintf(stream, "\n");
    _paint(stream, ORB_COLOUR_CLEAR);

    fflush(stream);

    pthread_mutex_unlock(&mutex);
}

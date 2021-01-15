#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

extern const char* argv0;

void die(const char* fmt, ...)
{
    fmt = str_concat(2, fmt, "\n");
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    exit(EXIT_FAILURE);
}

void edie(const char* fmt, ...)
{
    fmt = str_concat(2, fmt, strerror(errno));
    va_list ap;
    die(fmt, ap);
}

void usage()
{
    die("usage: %s\n", argv0);
}

char* str_concat(int count, ...)
{
    va_list ap;
    int new_len = 1;

    /* Total length of all strings */
    va_start(ap, count);
    for (int i = 0; i < count; i++)
        new_len += strlen(va_arg(ap, char*));
    va_end(ap);

    char *new_str = calloc(new_len, sizeof(char));
    if (new_str == NULL) edie("calloc: ");

    /* Concat strings into newstr */
    va_start(ap, count);
    for (int i = 0; i < count; i++) {
        char* s = va_arg(ap, char*);
        strcat(new_str, s);
    }
    va_end(ap);

    return new_str;
}

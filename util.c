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

void* ecalloc(size_t nmemb, size_t size)
{
    void* ptr = calloc(nmemb, size);
    if (ptr == NULL) edie("calloc: ");
    return ptr;
}

size_t str_pushc(char* s, char c, size_t s_size, size_t realloc_amount) {
    /* Realloc s if needed */
    int s_len = strlen(s);
    if (s_len + 1 > s_size) {
        s_size += realloc_amount;
        s = realloc(s, s_size);
        if (s == NULL) edie("realloc: ");
    }

    s[s_len] = c;
    s[s_len + 1] = '\0';
    return s_size;
}

int str_trimr(char* s, char c, int max_num)
{
    int removed = 0;
    int i = strlen(s)-1;
    while (s[i] == c && removed <= max_num) {
        s[i] = '\0';
        i--;
        removed++;
    }
    return removed;
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

struct str_list* str_list_add(struct str_list* l, char* s) {
    l->size++;
    l->values = realloc(l->values, l->size);
    if (l->values == NULL) edie("realloc: ");
    l->values[l->size-1] = s;
    return l;
}

struct str_list str_list_new(int count, ...)
{
    /* At least one str is required */
    if (count < 1) return (struct str_list) { .size = 0, .values = NULL };

    va_list ap;
    va_start(ap, count);
    struct str_list l = {
        .size = count,
        .values = ecalloc(count, sizeof(char*))
    };
    for (int i = 0; i < l.size; i++) {
        l.values[i] = va_arg(ap, char*);
    }
    va_end(ap);

    return l;
}

void str_list_free(struct str_list* l) {
    for (int i = 0; i < l->size; i++) {
        free(l->values[i]);
    }
}

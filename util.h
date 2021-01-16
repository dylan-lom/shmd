#include <stddef.h>

/*
 * TODO: It's probably considered bad practice to define macros in lower-case...
 * TODO: str_list_free
 */

#define SHIFT_ARGS() argv++; argc--
#define SET_ARGV0()  argv0 = argv[0]; SHIFT_ARGS()

void  die(const char* fmt, ...);
void  edie(const char* fmt, ...);
void  usage();

void* ecalloc(size_t nmemb, size_t size);

size_t str_pushc(char* s, char c, size_t s_size, size_t realloc_amount);
int    str_trimr(char* s, char c, int max_num);
char*  str_concat(int count, ...);
#define str_ealloc(s) ecalloc(s, sizeof(char))
#define str_malloc(s) calloc(s, sizeof(char))

struct str_list {
    size_t size;
    char** values;
};

struct str_list* str_list_add(struct str_list* l, char* s);
struct str_list  str_list_new(int count, ...);
#define str_list_init() str_list_new(0)


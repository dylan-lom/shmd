#define SHIFT_ARGS() argv++; argc--
#define SET_ARGV0() argv0 = argv[0]; SHIFT_ARGS()

void  die(const char* fmt, ...);
void  edie(const char* fmt, ...);
void  usage();

void* ecalloc(size_t nmemb, size_t size);

char* str_concat(int count, ...);
char* str_trimr(char* s, char c, int max_num);
/* It's probably considered bad practice to define macros in lower-case... */
#define str_malloc(s)  calloc(s, sizeof(char))
#define str_ealloc(s) ecalloc(s, sizeof(char))

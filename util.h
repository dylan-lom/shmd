#define SHIFT_ARGS() argv++; argc--;
#define SET_ARGV0() argv0 = argv[0]; SHIFT_ARGS();

void die(const char* fmt, ...);
void edie(const char* fmt, ...);
void usage();

char* str_concat(int count, ...);
char* str_trimr(char* s, char c, int max_num);

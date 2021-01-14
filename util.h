#define SHIFT_ARGS() argv++; argc--;
#define SET_ARGV0() argv0 = argv[0]; SHIFT_ARGS();

void die(const char* fmt, ...);
void edie(const char* fmt, ...);
void usage();

char* concat(int count, ...);

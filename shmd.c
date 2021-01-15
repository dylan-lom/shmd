#include <stdio.h>
#include <stdlib.h>

#include "util.h"

const char* argv0;

char* execute_command(const char* command) {
    FILE *fp;
    fp = popen(command, "r");
    if (fp == NULL) edie("popen: ");

    char* result = calloc(1, sizeof(char));
    if (result == NULL) edie("calloc: ");
    result[0] = '\0';

    size_t buf_size = 81;
    char* buf = calloc(buf_size, sizeof(char));
    if (buf == NULL) edie("calloc: ");

    /*
     * str_concat allocates a new string on the heap, so free the old memory
     * when we concat the next line of output.
     */
    while (fgets(buf, buf_size, fp) != NULL) {
        char* tmp = result;
        result = str_concat(2, result, buf);
        free(tmp);
    }
    free(buf);
    pclose(fp);

    return result;
}

char* process_shmd(char* input, ...) {
    return execute_command(str_concat(3, "echo -n \"", input, "\""));
}

int main(int argc, char* argv[]) {
    SET_ARGV0();
    size_t buf_size = 1000;
    char* buf = calloc(buf_size, sizeof(char));
    if (buf == NULL) edie("calloc: ");

    while (fgets(buf, buf_size, stdin) != NULL) {
        printf("%s", process_shmd(buf));
    }

    return EXIT_SUCCESS;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

const char* argv0;

char* command_execute(const char* command) {
    FILE *pp;
    pp = popen(command, "r");
    if (pp == NULL) edie("popen: ");

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
    while (fgets(buf, buf_size, pp) != NULL) {
        char* tmp = result;
        result = str_concat(2, result, buf);
        free(tmp);
    }
    free(buf);
    pclose(pp);

    result = str_trimr(result, '\n', 1);
    return result;
}

char* command_substitute(FILE* fp) {
    size_t command_size = 100;
    size_t command_i = 0;
    char* command = calloc(command_size, sizeof(char));
    if (command == NULL) edie("calloc: ");
    memset(command, '\0', command_size);

    int sub_parens = 0; /* track nested parans inside of command */
    char c;
    while ((c = fgetc(fp)) != EOF) {
        if (c == ')' && sub_parens == 0) break;
        else if (c == ')') sub_parens--;
        else if (c == '(') sub_parens++;

        if (command_i >= command_size) {
            command_size += 100;
            command = realloc(command, command_size);
            if (command == NULL) edie("realloc: ");
            memset(command + command_i, '\0', command_size - command_i);
        }

        command[command_i] = c;
        command_i++;
    }

    return command_execute(command);
}

int process_input(FILE* fp) {
    char b, c;

    b = fgetc(fp);
    while (b != EOF) {
        c = fgetc(fp);
        if (b == '$' && c == '(') {
            printf("%s", command_substitute(fp));
            /*
             * command_substitute consumes from fp to after the closing ')' of
             * the substitution, we then need to get the next character from fp
             * for the next iteration of the loop.
             */
            b = fgetc(fp);
        } else {
            putchar(b);
            b = c;
        }
    }

    return EXIT_SUCCESS;
}

int main(int argc, char* argv[]) {
    SET_ARGV0();
    return process_input(stdin);
}

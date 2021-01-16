#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

const char* argv0;

struct str_list header_split(char* s) {
    struct str_list l = str_list_init();
    size_t word_size = 100;
    char* word = str_ealloc(word_size);

    /* Keep track of whether we're inside of a quoted or escaped string */
    int in_dquote = 0, in_squote = 0, in_escape = 0;

    for ( ; *s != '\0'; s++) {
        if (*s == ' ' && !in_squote && !in_dquote) {
            str_list_add(&l, word);
            word = str_ealloc(word_size); /* Allocate memory for next word */
            continue;
        }
        else if (*s == '\\' && !in_escape) { in_escape = 1; continue; }
        else if (*s == '"' && !in_escape) in_dquote ^= 1;
        else if (*s == '\'' && !in_escape) in_squote ^= 1;

        word_size = str_pushc(word, *s, word_size, 100);
        /* It's just simpler to reset this after every iteration */
        in_escape = 0;
    }

    return l;
}

char* command_execute(const char* command) {
    FILE *pp;
    pp = popen(command, "r");
    if (pp == NULL) edie("popen: ");

    char* result = str_ealloc(1);
    result[0] = '\0';

    size_t buf_size = 81;
    char* buf = str_ealloc(buf_size);

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

    str_trimr(result, '\n', 1);
    return result;
}

char* command_substitute(FILE* fp) {
    size_t command_size = 100;
    size_t command_i = 0;
    char* command = str_ealloc(command_size);
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
    struct str_list l = header_split("title \"Hello World\" wowo");
    return process_input(stdin);
}

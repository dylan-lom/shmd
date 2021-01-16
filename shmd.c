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
        else if (*s == '"' && !in_escape) { in_dquote ^= 1; continue; }
        else if (*s == '\'' && !in_escape) { in_squote ^= 1; continue; }

        word_size = str_pushc(word, *s, word_size, 100);
        /* It's just simpler to reset this after every iteration */
        in_escape = 0;
    }

    if (strlen(word) > 0) str_list_add(&l, word);
    return l;
}

char* header_list_to_html(struct str_list l) {
    if (l.size < 1) return "";
    char** vals = l.values;
    /* work out the total length of all strings */
    size_t vals_size = 0;
    for (int i = 0; i < l.size; i++)
        vals_size += strlen(vals[i]);

    char* html;
    /*
     * TODO: This is gross... I did it like this to make the if ... else
     * statement readable... need a better abstraction though
     */
    #define HTML_SPRINTF(l_min, fmt, ...) if (l.size < l_min) return ""; \
        html = str_ealloc(sizeof(fmt) + vals_size + 1); \
        sprintf(html, fmt, __VA_ARGS__)

    /* Known HEAD tags */
    if (strcmp(vals[0], "charset") == 0) {
        HTML_SPRINTF(2, "<meta charset=\"%s\">", vals[1]);
    } else if (strcmp(vals[0], "title") == 0) {
        HTML_SPRINTF(2, "<title>%s</title>", vals[1]);
    } else if (strcmp(vals[0], "link") == 0) {
        HTML_SPRINTF(3, "<link rel=\"%s\" vals=\"%s\">", vals[1], vals[2]);
    } else {
        HTML_SPRINTF(2, "<meta name=\"%s\" content=\"%s\">", vals[0], vals[1]);
    }

    return html;
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
    // struct str_list l = header_split("title \"Hello World\" wowo");
    struct str_list l = header_split("author \"Dylan Lom\"");
    puts(header_list_to_html(l));
    return process_input(stdin);
}

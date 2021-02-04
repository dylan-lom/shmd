#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shmd.h"

const char* argv0;
/*
 * Not a great name, but this is used for variables that we want avaiable to the
 * shell when we start, ie. the header stuff.
 */
char* sh_prefix;

struct str_list header_split(char* s) {
    struct str_list l = STR_LIST_INIT();
    size_t word_size = 100;
    char* word = STR_EALLOC(word_size);

    /* Keep track of whether we're inside of a quoted or escaped string */
    int in_dquote = 0, in_squote = 0, in_escape = 0;

    for ( ; *s != '\0'; s++) {
        if (*s == ' ' && !in_squote && !in_dquote) {
            str_list_add(&l, word);
            word = STR_EALLOC(word_size); /* Allocate memory for next word */
            continue;
        }
        else if (*s == '\\' && !in_escape) { in_escape = 1; continue; }
        else if (*s == '"' && !in_escape && !in_squote) { in_dquote ^= 1; continue; }
        else if (*s == '\'' && !in_escape && !in_dquote) { in_squote ^= 1; continue; }

        word_size = str_pushc(word, *s, word_size, 100);
        /* It's just simpler to reset this after every iteration */
        in_escape = 0;
    }

    if (strlen(word) > 0) str_list_add(&l, word);
    return l;
}

#define HTML_SPRINTF(l_min, fmt, ...) if (l.size < l_min) return ""; \
    html = STR_EALLOC(sizeof(fmt) + vals_size + 1); \
    sprintf(html, fmt, __VA_ARGS__)
char* header_list_process(struct str_list l) {
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
    /* Known HEAD tags */
    if (strcmp(vals[0], "charset") == 0) {
        HTML_SPRINTF(2, "<meta charset=\"%s\">", vals[1]);
    } else if (strcmp(vals[0], "title") == 0) {
        HTML_SPRINTF(2, "<title>%s</title>", vals[1]);
    } else if (strcmp(vals[0], "link") == 0) {
        HTML_SPRINTF(3, "<link rel=\"%s\" href=\"%s\">", vals[1], vals[2]);
    } else {
        HTML_SPRINTF(2, "<meta name=\"%s\" content=\"%s\">", vals[0], vals[1]);
    }

    /* Add to sh_prefix so that values are available in the shell environment */
    char* tmp = sh_prefix;
    size_t val0_length = strlen(vals[0]) - 1;
    if (vals[0][val0_length-1] == '(' && vals[0][val0_length] == ')') {
        /* Function */
        sh_prefix = str_concat(5, sh_prefix, vals[0], " { ", vals[l.size-1], "; }; ");
    } else {
        /* Variable */
        sh_prefix = str_concat(5, sh_prefix, vals[0], "=\"", vals[l.size-1], "\"; ");
    }
    free(tmp);

    return html;
}
#undef HTML_SPRINTF

#define HEADER_ISEND(s) (strlen(s) >= 2 && s[0] == '*' && s[1] == '/')
char* header_substitute(FILE* fp) {
    char* result = str_concat(1, "<head>");

    size_t line_size = 250;
    char* line = STR_EALLOC(line_size);
    char* lp;
    char* tmp;
    while (fgets(line, line_size, fp) != NULL) {
        str_trimr(line, '\n', 1);
        lp = line;
        /*
         * Non-alphabetical characters (e.g. whitespace, '*') in the header are
         * ignored, advance lp to the start of input, while also making sure we
         * don't reach the end of the line ('\0') or the end of the header
         * section ('* /' without the space).
         */
        while (!isalpha(*lp) && *lp != '\0' && !HEADER_ISEND(lp)) { lp++; }
        if (HEADER_ISEND(lp)) break;
        if (*lp == '\0') continue;

        struct str_list list = header_split(lp);
        char* line_html = header_list_process(list);
        str_list_free(&list);

        /* concat html to result */
        tmp = result;
        result = str_concat(3, result, "\n\t", line_html);
        free(tmp);
    }
    tmp = result;
    result = str_concat(3, result, "\n", "</head>");
    free(tmp);
    return result;
}
#undef HEADER_ISEND

char* command_execute(const char* command) {
    FILE *pp;
    char* c = str_concat(2, sh_prefix, command);
    pp = popen(c, "r");
    if (pp == NULL) edie("popen: ");

    char* result = STR_EALLOC(1);
    result[0] = '\0';

    size_t buf_size = 81;
    char* buf = STR_EALLOC(buf_size);

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
    char* command = STR_EALLOC(command_size);
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
    c = fgetc(fp);

    if (b == '/' && c == '*') {
        puts(header_substitute(fp));
        b = fgetc(fp);
        c = fgetc(fp);
    }

    while (b != EOF) {
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
        c = fgetc(fp);
    }

    return EXIT_SUCCESS;
}

int main(int argc, char* argv[]) {
    SET_ARGV0();
    sh_prefix = STR_EALLOC(100);
    return process_input(stdin);
}

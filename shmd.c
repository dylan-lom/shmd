#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "shmd.h"

const char *argv0;
char *sh_prefix;

char *header_process_extract_field(FILE *fp)
{
    char c;
    size_t dest_size = 100;
    char *dest = STR_EALLOC(dest_size);

    bool is_escaped = false,
         is_dquoted = false,
         is_squoted = false;
    while (
        (c = fgetc(fp)) != '\0' && !(
            !is_dquoted
            && !is_squoted
            && !is_escaped
            && (c == ' ' || c == '\n')
        )
    ) {
        if (!is_escaped) {
            if (c == '"' && !is_squoted) {
                is_dquoted = !is_dquoted;
                continue;
            } else if (c == '\'' && !is_dquoted) {
                is_squoted = !is_squoted;
                continue;
            } else if (c == '\\') {
                is_escaped = true;
                continue;
            }
        }
        dest_size = str_pushc(dest, c, dest_size, 50);
    }
    return dest;
}

size_t header_field_count(enum header_field_type type)
{
    size_t count = 0;
    switch (type) {
    case E_HEADER_FIELD_CHARSET:
        count = 1;
        break;
    case E_HEADER_FIELD_TITLE:
        count = 1;
        break;
    case E_HEADER_FIELD_LINK:
        count = 2;
        break;
    case E_HEADER_FIELD_META:
        count = 1;
        break;
    }
    return count;
}

char **header_process_fields(FILE *fp, enum header_field_type type)
{
    size_t values_count = header_field_count(type);
    char **values = ecalloc(values_count, sizeof(char*));

    for (int i = 0; i < values_count; i++) {
        values[i] = header_process_extract_field(fp);
    }

    return values;
}

/* Process a header record */
char *header_process(FILE *fp)
{
    size_t name_size = 100;
    char* name = STR_EALLOC(name_size);
    char c;
    while ((c = fgetc(fp)) != '\0' && c != ' ') {
        name_size = str_pushc(name, c, name_size, 100);
    }

    enum header_field_type type;
    if (strcmp(name, "charset") == 0)
        type = E_HEADER_FIELD_CHARSET;
    else if (strcmp(name, "title") == 0)
        type = E_HEADER_FIELD_TITLE;
    else if (strcmp(name, "link") == 0)
        type = E_HEADER_FIELD_LINK;
    else
        type = E_HEADER_FIELD_META;

    char **values = header_process_fields(fp, type);
    size_t values_count = header_field_count(type);

    return name;
}

#define HEADER_ISEND(b, c) (b == '*' && c == '/')
char* header_substitute(FILE* fp) {
    char* result = str_concat(1, "<head>");

    char b = '\0';
    char c;
    while ((c = fgetc(fp)) != '\0') {
        /*
         * Non-alphabetical characters (e.g. whitespace, '*') in the header are
         * ignored, advance lp to the start of input, while also making sure we
         * don't reach the end of the line ('\0') or the end of the header
         * section ('* /' without the space).
         */
        while (
            !isalpha(c)
            && c != '"'
            && c != '\''
            && c != '\0'
            && !HEADER_ISEND(b, c)
        ) {
            b = c;
            c = fgetc(fp);
        }
        if (c == '\0') continue;
        if (HEADER_ISEND(b, c)) break;

        ungetc(c, fp);
        header_process(fp);
    }
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

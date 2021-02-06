#include "util.h"

/**
 * #argv0
 * value of argv[0] at the start of execution (ie. the program's name).
 */
const char *argv0;
char *sh_prefix;

enum header_field_type
{
    E_HEADER_FIELD_CHARSET,
    E_HEADER_FIELD_TITLE,
    E_HEADER_FIELD_LINK,
    E_HEADER_FIELD_META
};
/* Read the next field from @fp */
char *header_process_extract_field(FILE *fp);
/* Get the number of properties for header type @type */
size_t header_field_count(enum header_field_type type);
/* Read space-seperated fields from @fp */
char **header_process_fields(FILE *fp, enum header_field_type type);
/* Get the html-ized version of a header */
char *header_to_html(enum header_field_type type, char *name, char **values);
/*
 * Get the shell-executable assignment of a header, using the last value as
 * the value.
 * In the special case @type is E_HEADER_FIELD_META and @name ends in '()',
 * will return a posix-style function declaration.
 */
char *header_to_sh(enum header_field_type type, char *name, char **values);
void header_process(FILE *fp);
char *header_substitute(FILE *fp);


/**
 * #command_execute
 * execute @command with `/bin/sh -c @command`.
 * @command: command to run.
 * @return:  stdout returned when running @command, with at most one trailing
 *           newline stripped.
 */
char *command_execute(const char *command);
/**
 * #command_substitute
 * Read @fp until the next unmatched `)` character and execute. This process is
 * destructive to fp (any characters read will not be re-added) and also
 * consumes the closing `)` character.
 * @fp:     file pointer to read from.
 * @return: the output of running #execute_command on the input read.
 */
char *command_substitute  (FILE *fp);

int process_input(FILE *fp);
int main(int argc, char *argv[]);

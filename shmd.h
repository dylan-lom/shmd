#include "util.h"

/**
 * #argv0
 * value of argv[0] at the start of execution (ie. the program's name).
 */
const char* argv0;

struct str_list header_split        (char* s);
char*           header_list_to_html (struct str_list l);
char*           header_substitute   (FILE* fp);

/**
 * #command_execute
 * execute @command with `/bin/sh -c @command`.
 * @command: command to run.
 * @return:  stdout returned when running @command, with at most one trailing
 *           newline stripped.
 */
char*           command_execute     (const char* command);
/**
 * #command_substitute
 * Read @fp until the next unmatched `)` character and execute. This process is
 * destructive to fp (any characters read will not be re-added) and also
 * consumes the closing `)` character.
 * @fp:     file pointer to read from.
 * @return: the output of running #execute_command on the input read.
 */
char*           command_substitute  (FILE* fp);

int             process_input       (FILE* fp);
int             main                (int argc, char* argv[]);

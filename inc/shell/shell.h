/**
 * shell.h
 *
 * The shell core: the command table types, the prompt loop and the
 * dispatcher.
 *
 * A command is one row of the table in cmd_table.c:
 *     { name, summary, handler }
 * The dispatcher, the banner and (from Phase 3) `help` all read that one
 * table, so adding a command never touches the dispatcher.
 */

#ifndef SHELL_SHELL_H
#define SHELL_SHELL_H

#include <stddef.h>

/* The prompt printed before every input line (requirement N1).
 * Change it here and nowhere else. */
#define SHELL_PROMPT     "> "

/* Size of the input line buffer, including the terminating '\0'.
 * A line longer than SHELL_LINE_MAX - 1 characters is cut by console_readline. */
#define SHELL_LINE_MAX   64U

/* What a command handler returns. */
typedef enum {
    CMD_OK   = 0,       /* the command did what was asked        */
    CMD_FAIL = 1        /* the command could not do its job      */
} cmd_result_t;

/* A command handler: takes nothing, prints its own output, returns a result. */
typedef cmd_result_t (*cmd_fn_t)(void);

typedef struct {
    const char *name;       /* what the user types; lower case, no leading/trailing space */
    const char *summary;    /* one-line description (used by `help` from Phase 3) */
    cmd_fn_t    run;        /* the handler */
} shell_cmd_t;

/* The command table. Defined in cmd_table.c, the only file that knows the
 * full list of commands. */
extern const shell_cmd_t shell_commands[];
extern const size_t      shell_command_count;

/* Prints every command name on one line, separated by " | ", then a newline. */
void shell_print_commands(void);

/* Runs ONE input line: trims it, finds the command in the table and calls it.
 * Works IN PLACE on `line` (str_trim modifies the buffer).
 * - blank or whitespace-only line: prints nothing, returns CMD_OK
 * - known command: returns whatever the handler returns
 * - unknown command: prints an error and the command list, returns CMD_FAIL */
cmd_result_t shell_execute(char *line);

/* The main loop: print prompt, read a line, execute it, repeat. Never returns. */
void shell_run(void);

#endif /* SHELL_SHELL_H */
/**
 * menu.h
 *
 * A reusable single-key menu. A menu is a constant array of rows:
 *     { key, label, action }
 * menu_run() prints the rows, waits for ONE key (no Enter needed), runs the
 * matching action and asks again, until the user presses q.
 *
 * Used by the `led` command (Phase 5); the `irq` (Phase 6) and `selftest`
 * (Phase 7) commands will use it too.
 */

#ifndef SHELL_MENU_H
#define SHELL_MENU_H

#include <stddef.h>
#include "shell/shell.h"       /* cmd_result_t */

/* The key that always leaves the menu (upper case Q works too).
 * No menu row may use it. */
#define MENU_QUIT_KEY  'q'

typedef struct {
    char          key;         /* ONE letter or digit, e.g. '1' (not 'q')       */
    const char   *label;       /* shown to the user, e.g. "On"                  */
    cmd_result_t (*action)(void);  /* runs when the key is pressed              */
} menu_item_t;

/* Runs the menu loop.
 *   title   heading, printed as "=== <title> ==="
 *   prompt  printed before every key, e.g. "led> "
 *   items   the rows (may be empty: then only q works)
 *   count   number of rows
 *
 * - A matching key runs its action. The action prints its own output; its
 *   return value does not stop the menu.
 * - Enter just prints a new prompt; Backspace is ignored.
 * - Any other key prints "Invalid choice...".
 * - q or Q leaves the menu. Keys still waiting in the UART are thrown away
 *   so they cannot start the next command.
 * Returns CMD_OK when the user quits. */
cmd_result_t menu_run(const char *title, const char *prompt,
                      const menu_item_t *items, size_t count);

#endif /* SHELL_MENU_H */
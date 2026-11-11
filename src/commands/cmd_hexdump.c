/**
 * cmd_hexdump.c
 *
 * `hexdump`: asks for an address and a length (Enter = defaults), checks that
 * the range is safe to read, then prints a hex + ASCII dump.
 *
 * The formatting and the range check live in hexdump.c; this file only
 * handles the dialogue with the user.
 */

#include <stdint.h>
#include "commands/commands.h"
#include "shell/console.h"
#include "shell/strutil.h"
#include "hexdump.h"

#define HEXDUMP_DEFAULT_ADDR       0x08000000U   /* start of flash: the vector table */
#define HEXDUMP_DEFAULT_ADDR_TEXT  "08000000"
#define HEXDUMP_DEFAULT_LEN        128U
#define HEXDUMP_DEFAULT_LEN_TEXT   "128"
#define HEXDUMP_MAX_LEN            1024U         /* one typo must not flood the terminal */
#define HEXDUMP_INPUT_SIZE         16U           /* "0x" + 8 digits + NUL = 11, with spare */

/* Prints "<label> [<default>]: ", reads one line and returns it trimmed.
 * An empty result means "use the default". */
static char *ask(const char *label, const char *def, char *buf, uint32_t size)
{
    console_puts(label);
    console_puts(" [");
    console_puts(def);
    console_puts("]: ");
    (void)console_readline(buf, size);
    console_newline();
    return str_trim(buf);
}

cmd_result_t cmd_hexdump(void)
{
    char     buf[HEXDUMP_INPUT_SIZE];
    char    *text;
    uint32_t addr = HEXDUMP_DEFAULT_ADDR;
    uint32_t len  = HEXDUMP_DEFAULT_LEN;

    /* 1. Address (hex, optional 0x prefix). */
    text = ask("Address in hex", HEXDUMP_DEFAULT_ADDR_TEXT, buf, (uint32_t)sizeof(buf));
    if (*text != '\0' && parse_hex(text, &addr) != 0) {
        console_puts("Invalid address (use hex, e.g. 20000000 or 0x20000000).\r\n");
        return CMD_FAIL;
    }

    /* 2. Length (decimal, 1..HEXDUMP_MAX_LEN). */
    text = ask("Length in bytes", HEXDUMP_DEFAULT_LEN_TEXT, buf, (uint32_t)sizeof(buf));
    if (*text != '\0' && parse_uint(text, &len) != 0) {
        console_puts("Invalid length (1-1024 bytes, decimal).\r\n");
        return CMD_FAIL;
    }
    if (len == 0U || len > HEXDUMP_MAX_LEN) {
        console_puts("Invalid length (1-1024 bytes, decimal).\r\n");
        return CMD_FAIL;
    }

    /* 3. Safety: flash and SRAM only. A read anywhere else can fault. */
    if (!hexdump_range_valid(addr, len)) {
        console_puts("Invalid range. Allowed: flash 0x08000000-0x0807FFFF, "
                     "SRAM 0x20000000-0x20017FFF.\r\n");
        return CMD_FAIL;
    }

    /* 4. Dump. console_puts has the signature hexdump() wants for its callback. */
    return (hexdump(addr, len, console_puts) == 0) ? CMD_OK : CMD_FAIL;
}
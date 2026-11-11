/**
 * cmd_uptime.c
 *
* `uptime`: time since boot, e.g. "Uptime: 0h 03m 12s (192 s)".
 *
 * uptime_ms() is a 32-bit millisecond counter, so it wraps after about
 * 49.7 days; `uptime` then starts again from 0.
 */

#include <stdint.h>
#include "commands/commands.h"
#include "shell/console.h"
#include "systick.h"

#define MS_PER_SECOND      1000U
#define SECONDS_PER_HOUR   3600U
#define SECONDS_PER_MINUTE 60U

/* Prints v as at least two digits: 3 -> "03", 12 -> "12". */
static void put_two_digits(uint32_t v)
{
    if (v < 10U) {
        console_putc('0');
    }
    console_put_uint(v);
}

cmd_result_t cmd_uptime(void)
{
    uint32_t total   = uptime_ms() / MS_PER_SECOND;
    uint32_t hours   = total / SECONDS_PER_HOUR;
    uint32_t minutes = (total % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE;
    uint32_t seconds = total % SECONDS_PER_MINUTE;

    console_puts("Uptime: ");
    console_put_uint(hours);
    console_puts("h ");
    put_two_digits(minutes);
    console_puts("m ");
    put_two_digits(seconds);
    console_puts("s (");
    console_put_uint(total);
    console_puts(" s)\r\n");
    return CMD_OK;
}
/**
 * cmd_led.c
 *
 * `led`: opens the LED menu (requirement F6).
 *
 *     1) On   2) Off   3) Toggle   4) Blinky   q) Quit
 *
 * The status LED (Phase 4) owns PA5. This file never touches the pin: it
 * takes manual control with status_led_acquire(), changes the LED through
 * status_led_manual_*(), and gives control back with status_led_release().
 */

#include <stdint.h>
#include "commands/commands.h"
#include "shell/console.h"
#include "shell/menu.h"
#include "shell/strutil.h"
#include "status_led.h"
#include "systick.h"

/* Blinky: 1 blink = one full on + off cycle, so the LED changes state every
 * 500 / rate milliseconds. The upper limit keeps every phase >= 25 ms,
 * which the eye can still follow. */
#define LED_BLINKY_MIN_RATE      1U
#define LED_BLINKY_MAX_RATE      20U
#define LED_BLINKY_DEFAULT_RATE  1U
#define LED_BLINKY_HALF_BASE_MS  500U
#define LED_INPUT_SIZE           8U      /* "20" + NUL, with plenty of spare */

/* Prints the state the LED is in right now. */
static void led_print_state(void)
{
    console_puts(status_led_manual_get() ? "LED is ON\r\n" : "LED is OFF\r\n");
}

static cmd_result_t led_on(void)
{
    status_led_manual_set(1);
    led_print_state();
    return CMD_OK;
}

static cmd_result_t led_off(void)
{
    status_led_manual_set(0);
    led_print_state();
    return CMD_OK;
}

static cmd_result_t led_toggle(void)
{
    status_led_manual_toggle();
    led_print_state();              /* print the NEW state */
    return CMD_OK;
}

/* Asks for a rate, then blinks until the user presses a key. */
static cmd_result_t led_blinky(void)
{
    char     buf[LED_INPUT_SIZE];
    char    *text;
    uint32_t rate = LED_BLINKY_DEFAULT_RATE;
    uint32_t half;
    uint32_t next;

    /* 1. Ask. Enter alone keeps the default. */
    console_puts("Blinks per second (1-20) [1]: ");
    (void)console_readline(buf, (uint32_t)sizeof(buf));
    console_newline();
    text = str_trim(buf);

    if (*text != '\0') {
        if (parse_uint(text, &rate) != 0 ||
            rate < LED_BLINKY_MIN_RATE || rate > LED_BLINKY_MAX_RATE) {
            console_puts("Invalid rate (1-20 blinks per second).\r\n");
            return CMD_FAIL;
        }
    }

    /* 2. Blink. Toggle every `half` ms, on a fixed cadence, until a key. */
    half = LED_BLINKY_HALF_BASE_MS / rate;

    console_puts("Blinking at ");
    console_put_uint(rate);
    console_puts(" blink(s) per second. Press any key to stop.\r\n");

    status_led_manual_set(1);                   /* start with a full on-phase */
    next = uptime_ms() + half;

    while (console_try_getkey() == KEY_NONE) {
        if ((int32_t)(uptime_ms() - next) >= 0) {   /* wrap-safe "is it due?" */
            status_led_manual_toggle();
            next += half;                           /* fixed cadence, no drift */
        }
    }

    /* 3. Stop: LED off, throw away any further keys. */
    status_led_manual_set(0);
    while (console_try_getkey() != KEY_NONE) {
        /* drain */
    }
    console_puts("Stopped.\r\n");
    led_print_state();
    return CMD_OK;
}

cmd_result_t cmd_led(void)
{
    static const menu_item_t items[] = {
        { '1', "On",     led_on     },
        { '2', "Off",    led_off    },
        { '3', "Toggle", led_toggle },
        { '4', "Blinky", led_blinky }
    };

    status_led_acquire();               /* status lights pause; LED is ours */
    (void)menu_run("LED menu", "led> ", items, sizeof(items) / sizeof(items[0]));
    status_led_release();               /* status lights resume (IDLE) */
    return CMD_OK;
}
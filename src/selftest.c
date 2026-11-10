/**
 * selftest.c
 *
 * Shell self-tests for the P3 drivers: `gpio test` and `uart test`.
 * Owner: P3 - Hardware Drivers
 */

#include "selftest.h"
#include "gpio.h"
#include "uart.h"
#include "systick.h"
#include "shell/console.h"

#define ST_PORT  GPIOB
#define ST_PIN   0U      /* PB0 = Arduino A3, left unconnected for the test */



/* ------------------------------------------------------------------ GPIO */

typedef struct {
    const char   *label;
    gpio_mode_t   mode;
    gpio_otype_t  otype;
    gpio_pull_t   pull;
    gpio_state_t  drive;    /* level written for output modes, ignored for input */
    gpio_state_t  expect;   /* level the pin must read back */
} gpio_case_t;

static const gpio_case_t gpio_cases[] = {
    { "input, pull-up",                      GPIO_MODE_INPUT,  GPIO_OTYPE_PUSH_PULL,  GPIO_PULL_UP,   GPIO_PIN_LOW,  GPIO_PIN_HIGH },
    { "input, pull-down",                    GPIO_MODE_INPUT,  GPIO_OTYPE_PUSH_PULL,  GPIO_PULL_DOWN, GPIO_PIN_LOW,  GPIO_PIN_LOW  },
    { "push-pull, drive high",               GPIO_MODE_OUTPUT, GPIO_OTYPE_PUSH_PULL,  GPIO_PULL_NONE, GPIO_PIN_HIGH, GPIO_PIN_HIGH },
    { "push-pull, drive low",                GPIO_MODE_OUTPUT, GPIO_OTYPE_PUSH_PULL,  GPIO_PULL_NONE, GPIO_PIN_LOW,  GPIO_PIN_LOW  },
    { "push-pull + pull-down, drive high",   GPIO_MODE_OUTPUT, GPIO_OTYPE_PUSH_PULL,  GPIO_PULL_DOWN, GPIO_PIN_HIGH, GPIO_PIN_HIGH },
    { "open-drain + pull-down, drive high",  GPIO_MODE_OUTPUT, GPIO_OTYPE_OPEN_DRAIN, GPIO_PULL_DOWN, GPIO_PIN_HIGH, GPIO_PIN_LOW  },
    { "open-drain + pull-up, drive high",    GPIO_MODE_OUTPUT, GPIO_OTYPE_OPEN_DRAIN, GPIO_PULL_UP,   GPIO_PIN_HIGH, GPIO_PIN_HIGH },
    { "open-drain + pull-up, drive low",     GPIO_MODE_OUTPUT, GPIO_OTYPE_OPEN_DRAIN, GPIO_PULL_UP,   GPIO_PIN_LOW,  GPIO_PIN_LOW  }
};

/* Configure PB0, drive it if it is an output, wait 1 ms for the level to
 * settle, then read it back. */
static gpio_state_t gpio_probe(const gpio_case_t *c)
{
    gpio_config_t cfg = {
        .mode  = c->mode,
        .otype = c->otype,
        .speed = GPIO_SPEED_LOW,
        .pull  = c->pull,
        .af    = 0
    };

    gpio_init(ST_PORT, ST_PIN, &cfg);
    if (c->mode == GPIO_MODE_OUTPUT) {
        gpio_write(ST_PORT, ST_PIN, c->drive);
    }
    delay_ms(1);
    return gpio_read(ST_PORT, ST_PIN);
}

/* Print one result line and return 1 for a pass, 0 for a fail. */
static uint32_t gpio_report(const char *label, gpio_state_t got, gpio_state_t want)
{
    console_puts("  "); console_puts(label); console_puts(": ");
    console_puts(got == GPIO_PIN_HIGH ? "HIGH" : "LOW");
    console_puts(got == want ? "  PASS\r\n" : "  FAIL\r\n");
    return (got == want) ? 1U : 0U;
}

void selftest_gpio(void)
{
    gpio_case_t restore = { "", GPIO_MODE_INPUT, GPIO_OTYPE_PUSH_PULL,
                            GPIO_PULL_NONE, GPIO_PIN_LOW, GPIO_PIN_LOW };
    gpio_case_t low = { "", GPIO_MODE_OUTPUT, GPIO_OTYPE_PUSH_PULL,
                        GPIO_PULL_NONE, GPIO_PIN_LOW, GPIO_PIN_LOW };
    uint32_t pass = 0U;
    uint32_t total = 0U;
    uint32_t i;

    console_puts("GPIO self-test on PB0 (Arduino A3). Nothing may be plugged into A3.\r\n");

    for (i = 0U; i < (sizeof(gpio_cases) / sizeof(gpio_cases[0])); i++) {
        pass += gpio_report(gpio_cases[i].label, gpio_probe(&gpio_cases[i]), gpio_cases[i].expect);
        total++;
    }

    /* Toggle: start from a driven low, then flip twice. */
    (void)gpio_probe(&low);
    gpio_toggle(ST_PORT, ST_PIN);
    delay_ms(1);
    pass += gpio_report("toggle once (low to high)", gpio_read(ST_PORT, ST_PIN), GPIO_PIN_HIGH);
    total++;
    gpio_toggle(ST_PORT, ST_PIN);
    delay_ms(1);
    pass += gpio_report("toggle twice (back to low)", gpio_read(ST_PORT, ST_PIN), GPIO_PIN_LOW);
    total++;

    (void)gpio_probe(&restore);     /* leave PB0 as a floating input */

    console_puts(pass == total ? "Result: ALL PASS (" : "Result: FAILED (");
    console_put_uint(pass); console_puts(" of "); console_put_uint(total); console_puts(" checks passed)\r\n");
}

/* ------------------------------------------------------------------ UART */

void selftest_uart(void)
{
    static const uint8_t msg[] = "MiniOS-IGNORED";
    uart_errors_t e;
    uint32_t start;

    console_puts("UART self-test (3 stages)\r\n");

    /* Stage 1: uart_write must send exactly the number of bytes asked for. */
    console_puts("1) uart_write. The next line should show exactly MiniOS and nothing after it:\r\n   ");
    uart_write(USART2, msg, 6U);
    console_puts("\r\n");

    /* Stage 2: uart_data_available must not block, and uart_getc must return the key. */
    console_puts("2) uart_data_available. Press any key within 5 seconds...\r\n");
    delay_ms(50);
    while (uart_data_available(USART2)) {
        (void)uart_getc(USART2);        /* drop anything left over from typing the command */
    }
    start = uptime_ms();
    while (!uart_data_available(USART2) && (uptime_ms() - start) < 5000U) {
        delay_ms(10);
    }
    if (uart_data_available(USART2)) {
        console_puts("   key received, code ");
        console_put_uint((uint32_t)(uint8_t)uart_getc(USART2));
        console_puts(": PASS\r\n");
    } else {
        console_puts("   no key within 5 seconds: SKIPPED\r\n");
    }

    /* Stage 3: bytes that arrive while nothing reads them must be counted as overruns. */
    console_puts("3) receive errors. In 2 seconds I will say GO. Then type 6 or more keys quickly.\r\n");
    uart_clear_errors(USART2);
    delay_ms(2000);
    console_puts("   GO (nothing is echoed during this stage)\r\n");
    delay_ms(3000);
    while (uart_data_available(USART2)) {
        (void)uart_getc(USART2);        /* this read is where the overrun gets counted */
    }
    uart_get_errors(USART2, &e);
    console_puts("   overrun "); console_put_uint(e.overrun);
    console_puts(", noise "); console_put_uint(e.noise);
    console_puts(", framing "); console_put_uint(e.framing);
    console_puts(e.overrun > 0U ? ": PASS, lost bytes were detected\r\n"
                       : ": no overrun seen (type more keys, faster, or ignore this stage)\r\n");
}
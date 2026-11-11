/**
 * status_led.c
 *
 * Status LED engine for LD2 (PA5). See status_led.h for the patterns.
 *
 * How it works: the engine keeps ONE deadline (in ms). status_led_update()
 * compares it with uptime_ms(); when the deadline has passed it flips the
 * LED and sets the next deadline. No delay_ms() is used anywhere, so the
 * shell stays responsive.
 *
 * Only this file writes to PA5. It uses gpio_write() (BSRR, atomic) and
 * never gpio_toggle() (read-modify-write).
 */

#include "status_led.h"
#include "gpio.h"
#include "systick.h"

/* LD2 on the Nucleo-F401RE is wired to PA5. */
#define STATUS_LED_PORT  GPIOA
#define STATUS_LED_PIN   5U

/* A pattern: LED on for on_ms, off for off_ms, `flashes` times (one-shots). */
typedef struct {
    uint32_t on_ms;
    uint32_t off_ms;
    uint32_t flashes;       /* 0 for the repeating heartbeat */
} pattern_t;

static const pattern_t PATTERN_IDLE  = { 100U, 1900U, 0U };
static const pattern_t PATTERN_OK    = { 100U,  100U, 2U };
static const pattern_t PATTERN_ERROR = {  50U,   50U, 6U };

/* HardFault blink: iterations of an empty loop per half period.
 * Roughly 100 ms at 16 MHz with -Os (about 6 cycles per iteration), but NOT
 * calibrated: change it if the blink looks too slow or too fast. */
#define FAULT_HALF_PERIOD_LOOPS  250000U

static led_status_t s_status       = STATUS_IDLE;
static int          s_acquired     = 0;     /* 1 = manual mode            */
static int          s_led_on       = 0;     /* what we last wrote to PA5  */
static uint32_t     s_deadline     = 0U;    /* when the next change is due */
static uint32_t     s_flashes_left = 0U;    /* one-shot: flashes still to do */

/* The ONLY place that writes the pin. Remembers what it wrote. */
static void led_write(int on)
{
    s_led_on = (on != 0) ? 1 : 0;
    gpio_write(STATUS_LED_PORT, STATUS_LED_PIN, s_led_on ? GPIO_PIN_HIGH : GPIO_PIN_LOW);
}

void status_led_init(void)
{
    gpio_init_output(STATUS_LED_PORT, STATUS_LED_PIN);
    s_acquired     = 0;
    s_status       = STATUS_IDLE;
    s_flashes_left = 0U;
    led_write(0);
    s_deadline = uptime_ms();           /* first heartbeat blink starts at once */
}

void status_led_set(led_status_t s)
{
    if (s_acquired) {
        return;
    }
    if (s == STATUS_BUSY) {
        s_status = STATUS_BUSY;
        led_write(1);                   /* solid on: nothing to animate */
    } else if (s == STATUS_IDLE) {
        s_status = STATUS_IDLE;
        led_write(0);
        s_deadline = uptime_ms() + PATTERN_IDLE.off_ms;
    } else {
        /* OK / ERROR are one-shots: use status_led_signal(). Ignored here. */
    }
}

void status_led_signal(led_status_t s)
{
    const pattern_t *p;

    if (s_acquired) {
        return;
    }
    if (s == STATUS_OK) {
        p = &PATTERN_OK;
    } else if (s == STATUS_ERROR) {
        p = &PATTERN_ERROR;
    } else {
        return;                         /* IDLE / BUSY are sticky: use set() */
    }

    s_status       = s;
    s_flashes_left = p->flashes;
    led_write(0);                       /* start with a short gap so the first flash is visible */
    s_deadline = uptime_ms() + p->off_ms;
}

led_status_t status_led_get(void)
{
    return s_status;
}

void status_led_update(void)
{
    const pattern_t *p;
    uint32_t         now;

    if (s_acquired || s_status == STATUS_BUSY) {
        return;                         /* frozen, or solid on */
    }

    now = uptime_ms();
    if ((int32_t)(now - s_deadline) < 0) {
        return;                         /* not due yet (wrap-safe compare) */
    }

    /* ---- IDLE: repeating heartbeat ---- */
    if (s_status == STATUS_IDLE) {
        if (s_led_on != 0) {
            led_write(0);
            s_deadline = now + PATTERN_IDLE.off_ms;
        } else {
            led_write(1);
            s_deadline = now + PATTERN_IDLE.on_ms;
        }
        return;
    }

    /* ---- OK / ERROR: finite number of flashes, then IDLE ---- */
    p = (s_status == STATUS_OK) ? &PATTERN_OK : &PATTERN_ERROR;

    if (s_led_on != 0) {
        led_write(0);                   /* end of a flash */
        s_flashes_left--;
        if (s_flashes_left == 0U) {
            s_status = STATUS_IDLE;     /* heartbeat resumes after one off period */
        }
        s_deadline = now + p->off_ms;
    } else {
        led_write(1);                   /* start of a flash */
        s_deadline = now + p->on_ms;
    }
}

/* ------------------------------------------------------------ manual mode */

void status_led_acquire(void)
{
    if (!s_acquired) {
        s_acquired = 1;
        led_write(0);                   /* deterministic starting point */
    }
}

void status_led_release(void)
{
    if (s_acquired) {
        s_acquired     = 0;
        s_status       = STATUS_IDLE;
        s_flashes_left = 0U;
        led_write(0);
        s_deadline = uptime_ms();       /* heartbeat blink starts at once */
    }
}

void status_led_manual_set(int on)
{
    if (s_acquired) {
        led_write(on);
    }
}

void status_led_manual_toggle(void)
{
    if (s_acquired) {
        led_write(s_led_on == 0);
    }
}

int status_led_manual_get(void)
{
    return s_led_on;
}

/* ------------------------------------------------------------- HardFault */

void status_led_fault_forever(void)
{
    volatile uint32_t i;

    for (;;) {
        gpio_write(STATUS_LED_PORT, STATUS_LED_PIN, GPIO_PIN_HIGH);
        for (i = 0U; i < FAULT_HALF_PERIOD_LOOPS; i++) {
            /* wait */
        }
        gpio_write(STATUS_LED_PORT, STATUS_LED_PIN, GPIO_PIN_LOW);
        for (i = 0U; i < FAULT_HALF_PERIOD_LOOPS; i++) {
            /* wait */
        }
    }
}
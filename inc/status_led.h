/**
 * status_led.h
 *
 * Owner of LD2 (PA5), the only user LED on the Nucleo-F401RE.
 * No other file may write to PA5.
 *
 * Status patterns (all non-blocking, driven by status_led_update()):
 *   IDLE   heartbeat: 100 ms on, 1900 ms off, repeating   (sticky)
 *   BUSY   solid ON                                       (sticky)
 *   OK     2 flashes of 100 ms on / 100 ms off, then IDLE (one-shot)
 *   ERROR  6 flashes of  50 ms on /  50 ms off, then IDLE (one-shot)
 *
 * Manual mode (acquire / manual_* / release) freezes the patterns so the
 * user can control the LED directly. While acquired, set() and signal()
 * are ignored.
 *
 * Depends only on gpio.h and systick.h.
 */

#ifndef STATUS_LED_H
#define STATUS_LED_H

#include <stdint.h>

typedef enum {
    STATUS_IDLE = 0,    /* waiting at the prompt: heartbeat           */
    STATUS_BUSY,        /* a command is running: solid on             */
    STATUS_OK,          /* last command succeeded: 2 flashes          */
    STATUS_ERROR        /* last command failed: 6 fast flashes        */
} led_status_t;

/* Configures PA5 as an output, LED off, status IDLE, manual mode off.
 * Call once, AFTER systick_init() (it reads uptime_ms()). */
void status_led_init(void);

/* Sticky status: STATUS_IDLE or STATUS_BUSY. Any other value is ignored.
 * Ignored while the LED is acquired. */
void status_led_set(led_status_t s);

/* One-shot pattern: STATUS_OK or STATUS_ERROR. It runs, then the engine
 * returns to STATUS_IDLE by itself. Any other value is ignored.
 * Ignored while the LED is acquired. */
void status_led_signal(led_status_t s);

/* Returns the current status (what the engine is showing). */
led_status_t status_led_get(void);

/* Advances the patterns. Non-blocking: call it as often as you like.
 * Does nothing while BUSY (solid) or while the LED is acquired. */
void status_led_update(void);

/* ---- manual control (used by the led commands, later the led menu) ---- */

/* Takes manual control: patterns freeze. The first call turns the LED off;
 * calling it again while already acquired changes nothing. */
void status_led_acquire(void);

/* Gives control back: status becomes IDLE and the heartbeat resumes. */
void status_led_release(void);

/* Only while acquired (otherwise ignored). */
void status_led_manual_set(int on);
void status_led_manual_toggle(void);

/* Current physical state of the LED: 1 = on, 0 = off. */
int  status_led_manual_get(void);

/* HardFault pattern: blinks the LED fast, forever. Never returns.
 * Uses a counted busy loop, NOT SysTick, because the fault handler runs
 * with interrupts disabled. Only the fault handler should call it. */
void status_led_fault_forever(void) __attribute__((noreturn));

#endif /* STATUS_LED_H */
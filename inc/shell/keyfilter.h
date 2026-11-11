/**
 * keyfilter.h
 *
 * Turns the raw bytes received from the terminal into clean "key" events.
 *
 * Only these keys are let through:
 *     letters, digits, space, Backspace, Enter
 * Everything else is silently dropped, in particular the multi-byte escape
 * sequences sent by arrow keys, Home/End/Delete and the F-keys.
 *
 * This file has NO hardware dependency, so it can be unit-tested on a PC.
 */

#ifndef SHELL_KEYFILTER_H
#define SHELL_KEYFILTER_H

#include <stdint.h>

#define KEY_NONE       (-1)     /* no key event (byte swallowed or ignored) */
#define KEY_ENTER      ('\n')   /* CR, LF, or CR+LF all give ONE Enter      */
#define KEY_BACKSPACE  ('\b')   /* both 0x08 and 0x7F                       */

typedef enum {
    KF_NORMAL = 0,              /* ordinary typing                          */
    KF_ESC,                     /* just saw ESC (0x1B)                      */
    KF_SEQ                      /* inside an ESC [ ... or ESC O ... sequence */
} kf_state_t;

typedef struct {
    kf_state_t state;
    uint8_t    prev;            /* previous byte, used to merge CR+LF       */
} key_filter_t;

void key_filter_init(key_filter_t *f);

/* Feed ONE received byte. Returns KEY_NONE, KEY_ENTER, KEY_BACKSPACE, or the
 * ASCII code of an accepted letter/digit/space. */
int key_filter_feed(key_filter_t *f, uint8_t byte);

#endif /* SHELL_KEYFILTER_H */
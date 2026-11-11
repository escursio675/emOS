/**
 * keyfilter.c
 *
 * Input key filter
 *
 * An arrow key is sent by the terminal as three bytes, e.g. Up = ESC [ A
 * (0x1B 0x5B 0x41). This small state machine recognises the start of such a
 * sequence and throws away all of its bytes, so none of them is echoed and
 * none ends up in the command line.
 *
 *   KF_NORMAL --ESC--> KF_ESC --'[' or 'O'--> KF_SEQ --final byte--> KF_NORMAL
 *                         |  any other byte
 *                         +--------------------------> KF_NORMAL (byte dropped)
 */

#include "shell/keyfilter.h"

#define ASCII_ESC  0x1BU
#define ASCII_DEL  0x7FU

static int is_alnum(uint8_t c)
{
    return ((c >= '0' && c <= '9') ||
            (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z'));
}

void key_filter_init(key_filter_t *f)
{
    f->state = KF_NORMAL;
    f->prev  = 0U;
}

int key_filter_feed(key_filter_t *f, uint8_t c)
{
    uint8_t prev = f->prev;     /* byte before this one */
    f->prev = c;

    /* --- Inside an escape sequence: swallow bytes until it ends. --- */
    if (f->state == KF_ESC) {
        /* ESC [ ... (CSI) and ESC O ... (SS3) are the two sequence types
         * terminals use for cursor and function keys. Anything else after
         * ESC (e.g. Alt+key) is a 2-byte sequence: drop it and finish. */
        f->state = (c == '[' || c == 'O') ? KF_SEQ : KF_NORMAL;
        return KEY_NONE;
    }
    if (f->state == KF_SEQ) {
        /* Parameter bytes (digits, ';') keep the sequence going. A "final
         * byte" in 0x40..0x7E (A, B, C, D, ~ ...) ends it. A second '['
         * is allowed through because the Linux console sends ESC [ [ A
         * for F1. */
        if (c >= 0x40U && c <= 0x7EU && c != '[') {
            f->state = KF_NORMAL;
        }
        return KEY_NONE;
    }

    /* --- Normal typing. --- */
    if (c == ASCII_ESC) {
        f->state = KF_ESC;
        return KEY_NONE;
    }
    if (c == '\r') {
        return KEY_ENTER;
    }
    if (c == '\n') {
        /* A terminal that sends CR+LF must count as ONE Enter. */
        return (prev == '\r') ? KEY_NONE : KEY_ENTER;
    }
    if (c == '\b' || c == ASCII_DEL) {
        return KEY_BACKSPACE;
    }
    if (is_alnum(c) || c == ' ') {
        return (int)c;
    }

    return KEY_NONE;            /* Tab, Ctrl-keys, punctuation, 8-bit bytes... */
}
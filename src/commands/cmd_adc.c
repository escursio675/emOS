/**
 * cmd_adc.c
 *
 * ADC commands: `adc` and `vref`.
 * (`vref` was the two-word command `adc vref` until Phase 7.)
 */

#include "commands/commands.h"
#include "shell/console.h"
#include "adc.h"

/* adc: one conversion on A0 (PA0 = ADC1 channel 0), shown as raw count and millivolts. */
cmd_result_t cmd_adc(void)
{
    uint16_t raw = adc_read(ADC_CHANNEL_0);

    if (raw == ADC_READ_ERROR) {
        console_puts("ADC read failed: no end-of-conversion\r\n");
        return CMD_FAIL;
    }

    console_puts("ADC A0 (PA0): raw "); console_put_uint(raw);
    console_puts(" of 4095, ");         console_put_uint(adc_to_millivolts(raw));
    console_puts(" mV\r\n");
    return CMD_OK;
}

/* vref: reads the chip's internal 1.21 V reference and estimates VDDA from it.
 * Needs no wiring, so it checks the whole conversion path on its own. */
cmd_result_t cmd_vref(void)
{
    uint16_t raw = adc_read(ADC_CHANNEL_VREFINT);

    if (raw == ADC_READ_ERROR) {
        console_puts("ADC read failed: no end-of-conversion\r\n");
        return CMD_FAIL;
    }

    console_puts("VREFINT raw "); console_put_uint(raw);
    console_puts(" of 4095, estimated VDDA "); console_put_uint(adc_estimate_vdda_mv(raw));
    console_puts(" mV\r\n");
    console_puts("(expected: raw about 1450-1550, VDDA about 3200-3400 mV)\r\n");
    return CMD_OK;
}
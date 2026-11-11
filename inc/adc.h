/**
 * adc.h
 *
 * Single-conversion, polling-mode ADC driver for STM32F401RE (ADC1).
 * Owner: P3 - Hardware Drivers
 *
 * Software-triggered, one channel at a time, 12-bit, right-aligned.
 * DMA and continuous sampling are stretch goals and are not part of this API.
 *
 * Nucleo-F401RE Arduino analog pins (CN8) and the ADC1 channel each uses:
 *   A0 = PA0 = channel 0     A1 = PA1 = channel 1     A2 = PA4 = channel 4
 *   A3 = PB0 = channel 8     A4 = PC1 = channel 11    A5 = PC0 = channel 10
 *
 * Results are measured against VDDA, assumed to be 3.3 V on this board.
 * No floating point is used, because the FPU is not enabled at startup.
 */

#ifndef ADC_H
#define ADC_H

#include <stdint.h>
#include "stm32f401re.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Returned by adc_read() when no end-of-conversion arrives. A real 12-bit result is at most 4095. */
#define ADC_READ_ERROR  0xFFFFU

/**
 * ADC1 input channels. 0 to 15 are the external pins (ADC1_INx).
 * ADC_CHANNEL_VREFINT is the chip's internal 1.21 V reference, which needs no
 * wiring and is useful as a self-test and for estimating VDDA.
 */
typedef enum {
    ADC_CHANNEL_0 = 0,
    ADC_CHANNEL_1,
    ADC_CHANNEL_2,
    ADC_CHANNEL_3,
    ADC_CHANNEL_4,
    ADC_CHANNEL_5,
    ADC_CHANNEL_6,
    ADC_CHANNEL_7,
    ADC_CHANNEL_8,
    ADC_CHANNEL_9,
    ADC_CHANNEL_10,
    ADC_CHANNEL_11,
    ADC_CHANNEL_12,
    ADC_CHANNEL_13,
    ADC_CHANNEL_14,
    ADC_CHANNEL_15,
    ADC_CHANNEL_VREFINT = 17
} adc_channel_t;

/**
 * Enable ADC1's clock, switch on the internal reference, select 12-bit
 * single-conversion mode and power the ADC on. Call once at startup, before
 * any adc_read().
 *
 * Does NOT configure GPIO pins: call gpio_init_analog() for the pin first.
 */
void adc_init(void);

/**
 * Select `channel` as the one conversion to run. External channels use a
 * 56-cycle sample time (about 7 us at the default 8 MHz ADC clock), which
 * suits a wire or a potentiometer. The internal reference needs at least
 * 10 us, so it uses 480 cycles (about 60 us). Any other channel number is
 * ignored. adc_read() calls this itself; it is exposed so a caller can set
 * the channel up once. Never call it while a conversion is running.
 */
void adc_channel_config(adc_channel_t channel);

/**
 * Run one conversion on `channel` and wait for it to finish.
 * Returns the raw 12-bit result (0 to 4095), or ADC_READ_ERROR if the
 * conversion never completed (for example if adc_init() was not called).
 */
uint16_t adc_read(adc_channel_t channel);

/**
 * Convert a raw 12-bit reading to millivolts for a 3.3 V reference,
 * rounded to the nearest millivolt. Values above 4095 are treated as 4095.
 */
uint32_t adc_to_millivolts(uint16_t raw);

/**
 * Estimate the supply voltage VDDA in millivolts from a raw reading of
 * ADC_CHANNEL_VREFINT, assuming the reference is exactly 1.21 V. The datasheet
 * allows 1.18 to 1.24 V, so the estimate is good to about 2.5%.
 * Returns 0 if `vrefint_raw` is 0.
 */
uint32_t adc_estimate_vdda_mv(uint16_t vrefint_raw);

#ifdef __cplusplus
}
#endif

#endif /* ADC_H */
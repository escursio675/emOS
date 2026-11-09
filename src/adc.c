/**
 * adc.c
 *
 * Single-conversion, polling-mode ADC driver implementation for
 * STM32F401RE (ADC1).
 * Owner: P3 - Hardware Drivers
 */

#include "adc.h"

/* The shared register header has no ADC common register block, so the one
 * register needed from it (CCR, offset 0x04) is defined here. */
#ifndef ADC_CCR
#define ADC_CCR             (*(volatile uint32_t *)(ADC_COMMON_BASE + 0x04UL))
#endif
#define ADC_CCR_TSVREFE     (1UL << 23)   /* switches on the internal reference */

#define ADC_VREF_MV         3300U      /* VDDA on the Nucleo board, in millivolts */
#define ADC_VREFINT_MV      1210U      /* nominal internal reference, in millivolts */
#define ADC_MAX_RAW         4095U      /* full scale of a 12-bit conversion */
#define ADC_SMP_56_CYCLES   0x3UL      /* SMPx code 011 = 56 ADC clock cycles */
#define ADC_SMP_480_CYCLES  0x7UL      /* SMPx code 111 = 480 ADC clock cycles */
#define ADC_STAB_LOOPS      100U       /* busy-wait passes after power-on (needs >= 3 us) */
#define ADC_TIMEOUT_LOOPS   100000U    /* passes to wait for end of conversion */

/* Short busy-wait: 100 passes is far longer than the ~3 us the ADC needs
 * to stabilise after ADON is set. */
static void adc_stabilise_delay(void)
{
    volatile uint32_t i;

    for (i = 0U; i < ADC_STAB_LOOPS; i++) {
    }
}

void adc_init(void)
{
    /* 1. Clock for ADC1, then read the register back so the clock is
     *    really running before the first access to the ADC registers. */
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    (void)RCC->APB2ENR;

    /* 2. Switch on the internal reference (channel 17). */
    ADC_CCR |= ADC_CCR_TSVREFE;

    /* 3. CR1 = 0: 12-bit resolution, no scan mode, no interrupts. */
    ADC1->CR1 = 0U;

    /* 4. CR2 = ADON only: power on, single conversion, right-aligned data,
     *    software trigger. The ADC clock is left at its default of PCLK2 / 2. */
    ADC1->CR2 = ADC_CR2_ADON;

    /* 5. Give the ADC time to stabilise before the first conversion. */
    adc_stabilise_delay();
}

void adc_channel_config(adc_channel_t channel)
{
    uint32_t ch = (uint32_t)channel;
    uint32_t smp;

    /* Only the 16 external channels and the internal reference are valid. */
    if (ch > 15U && ch != (uint32_t)ADC_CHANNEL_VREFINT) {
        return;
    }

    /* The internal reference needs at least 10 us of sampling. */
    smp = (ch == (uint32_t)ADC_CHANNEL_VREFINT) ? ADC_SMP_480_CYCLES
                                                : ADC_SMP_56_CYCLES;

    /* Sample time: 3 bits per channel. Channels 0-9 are in SMPR2,
     * channels 10-17 are in SMPR1. */
    if (ch < 10U) {
        ADC1->SMPR2 &= ~(0x7UL << (ch * 3U));
        ADC1->SMPR2 |=  (smp << (ch * 3U));
    } else {
        ADC1->SMPR1 &= ~(0x7UL << ((ch - 10U) * 3U));
        ADC1->SMPR1 |=  (smp << ((ch - 10U) * 3U));
    }

    /* Sequence length L = 0000 (bits 23:20) means one conversion. */
    ADC1->SQR1 &= ~(0xFUL << 20);

    /* First and only conversion in the sequence: SQ1 (bits 4:0) = channel. */
    ADC1->SQR3 = (ADC1->SQR3 & ~0x1FUL) | ch;
}

uint16_t adc_read(adc_channel_t channel)
{
    uint32_t timeout = ADC_TIMEOUT_LOOPS;

    adc_channel_config(channel);

    ADC1->SR = 0U;                     /* clear any stale status flags */
    ADC1->CR2 |= ADC_CR2_SWSTART;      /* start one conversion */

    /* Wait for end of conversion, but never forever. */
    while (!(ADC1->SR & ADC_SR_EOC)) {
        if (timeout == 0U) {
            return ADC_READ_ERROR;
        }
        timeout--;
    }

    /* Reading DR returns the 12-bit result and clears EOC. */
    return (uint16_t)(ADC1->DR & 0x0FFFU);
}

uint32_t adc_to_millivolts(uint16_t raw)
{
    uint32_t r = raw;

    if (r > ADC_MAX_RAW) {
        r = ADC_MAX_RAW;
    }

    /* raw * 3300 / 4095, rounded to nearest. The largest product,
     * 4095 * 3300, is about 13.5 million and fits easily in 32 bits. */
    return (r * ADC_VREF_MV + (ADC_MAX_RAW / 2U)) / ADC_MAX_RAW;
}

uint32_t adc_estimate_vdda_mv(uint16_t vrefint_raw)
{
    uint32_t r = vrefint_raw;

    if (r == 0U) {
        return 0U;
    }

    /* The reading is VREFINT / VDDA * 4095, so VDDA = 1210 mV * 4095 / raw,
     * rounded to nearest. 1210 * 4095 is about 5 million and fits in 32 bits. */
    return (ADC_VREFINT_MV * ADC_MAX_RAW + (r / 2U)) / r;
}
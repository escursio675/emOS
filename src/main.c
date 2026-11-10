#include <stdint.h>
#include "systick.h"
#include "gpio.h"
#include "uart.h"
#include "adc.h"
#include "memory.h"
#include "interrupts.h"
#include "shell/console.h"
#include "shell/shell.h"
#include "commands/commands.h"     /* only for cmd_greet() */

int main(void)
{
    /* LD2 on the Nucleo-F401RE is wired to PA5 */
    gpio_init_output(GPIOA, 5);

    /* B1 user button is wired to PC13 (idle HIGH via external pull-up, LOW when pressed) */
    gpio_init_input(GPIOC, 13);

    /* USART2 TX/RX pins: PA2 = TX, PA3 = RX, both use AF7 */
    gpio_init_af(GPIOA, 2, 7);
    gpio_init_af(GPIOA, 3, 7);

    /* PA0 (Arduino A0) as an analog input for ADC1 channel 0 */
    gpio_init_analog(GPIOA, 0);

    uart_init(USART2, 115200, 16000000);
    console_init();           /* after uart_init(): console sits on top of USART2 */
    systick_init(16000 - 1);  /* 1ms tick @ 16MHz HSI */
    kmalloc_init();
    adc_init();

    /* Initialize EXTI for Blue Button (NVIC Demo) */
    button_exti_init();
    irq_enable_global(); /* Ensure interrupts are unmasked */

    /* Boot greeting (N2): banner + "Type 'help' ..." hint. */
    (void)cmd_greet();

    shell_run();              /* never returns */

    return 0;
}
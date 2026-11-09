#projects 

# P3: Hardware Drivers (GPIO, UART, ADC)

P3 owns the hardware drivers of MiniOS. GPIO, UART and ADC are all written and verified on a Nucleo-F401RE board.

## Status

| Driver | Files | State | Verified on the board |
| --- | --- | --- | --- |
| GPIO | `inc/gpio.h`, `src/gpio.c` | Done, merged into `main` | LED blink, button read, shell commands |
| UART (USART2) | `inc/uart.h`, `src/uart.c` | Done, merged into `main` | Transmit, receive, line read, shell prompt |
| ADC (ADC1) | `inc/adc.h`, `src/adc.c` | Done, on branch `p3-adc-driver` | Internal 1.21 V reference, and A0 wired to GND and to 3V3 (see Testing) |

All three drivers use the register map in `inc/stm32f401re.h`. There is no HAL and no floating point (the FPU is not enabled at startup).

## Hardware and settings

| Item | Detail |
| --- | --- |
| Board | Nucleo-F401RE (STM32F401RE, Cortex-M4), clock 16 MHz HSI |
| LED | LD2 on PA5 |
| Button | B1 on PC13. Active-low: high when released, low when pressed. Pressing it also raises the EXTI interrupt, which toggles LD2 (see the interrupts report) |
| UART | USART2, PA2 (TX) and PA3 (RX), alternate function 7, 115200 baud, 8 data bits, no parity, 1 stop bit |
| ADC | ADC1, 12-bit. A0 is PA0 = channel 0. Readings assume VDDA = 3.3 V |
| Flash tool | `st-flash` from the stlink package, run by `make flash`, writing at address `0x8000000` |
| Serial terminal | `picocom` at 115200 baud, 8N1 |

## Functions

**GPIO** (`gpio.h`)

| Function | What it does |
| --- | --- |
| `gpio_clock_enable(port)` | Turns on the clock of a GPIO port |
| `gpio_init(port, pin, config)` | Sets mode, output type, speed, pull and alternate function of one pin |
| `gpio_set_af(port, pin, af)` | Sets the alternate function number of a pin |
| `gpio_write(port, pin, state)` | Drives a pin high or low through `BSRR` |
| `gpio_toggle(port, pin)` | Flips an output pin |
| `gpio_read(port, pin)` | Returns the level of a pin |
| `gpio_init_output`, `gpio_init_input`, `gpio_init_af`, `gpio_init_analog` | Shortcuts for the common pin set-ups |

**UART** (`uart.h`)

| Function | What it does |
| --- | --- |
| `uart_init(usart, baud, pclk)` | Sets up a USART for 8N1 at the given baud rate |
| `uart_putc`, `uart_puts`, `uart_write` | Send one byte, a string, or a block of bytes (blocking) |
| `uart_getc` | Waits for one byte and returns it |
| `uart_data_available` | Returns 1 if a byte is waiting, without blocking |
| `uart_read_line(usart, buf, maxlen)` | Reads a typed line with echo and backspace. Stops at Enter or after `maxlen - 1` characters |

**ADC** (`adc.h`)

| Function | What it does |
| --- | --- |
| `adc_init()` | Enables the ADC1 clock, powers it on and switches on the internal reference |
| `adc_channel_config(channel)` | Selects the channel for the next conversion |
| `adc_read(channel)` | Runs one conversion. Returns 0 to 4095, or `ADC_READ_ERROR` (0xFFFF) if it never finishes |
| `adc_to_millivolts(raw)` | Converts a reading to millivolts (`raw * 3300 / 4095`, integer math) |
| `adc_estimate_vdda_mv(raw)` | Estimates the supply voltage from a reading of `ADC_CHANNEL_VREFINT` |

## Shell commands for the drivers

| Command | What it does |
| --- | --- |
| `led on`, `led off`, `led toggle` | Control LD2 with `gpio_write` and `gpio_toggle` |
| `button` | Prints whether B1 is pressed, using `gpio_read` |
| `adc` | One conversion on A0, printed as a raw value and in millivolts |
| `adc vref` | Reads the internal 1.21 V reference and estimates VDDA. Needs no wiring |

With nothing connected, `adc` reads a floating pin, so its value is arbitrary and changes between runs. That is normal.

## Building and flashing

1. Build with `make clean` and then `make`. A successful build prints nothing.
2. Plug in the board and flash with `make flash`. Success ends with `Flash written and verified! jolly good!`.
3. For UART tests, find the port with `ls /dev/tty.usb*` on a Mac or `/dev/ttyACM0` on Linux. Open it with `picocom -b 115200 /dev/tty.usbmodemXXXX` or `picocom -b 115200 /dev/ttyACM0`.

## Testing

After flashing, open the terminal and press the black RESET button. The banner and a `>` prompt appear. Then run:

1. `led on`, `led off`, `led toggle`: LD2 follows each command.
2. `button` with B1 untouched prints `Button is NOT pressed`. Hold B1 and run it again to get `Button is PRESSED`.
3. `adc vref` should print a raw value of about 1500 and an estimated VDDA of about 3300 mV. On our board it reads raw 1497 to 1499 and VDDA 3306 to 3310 mV. Anything from roughly 1450 to 1550 raw is normal, because the reference may be anywhere from 1.18 V to 1.24 V.

**ADC check with a jumper wire (done).** Run `adc` several times while A0 is wired to GND, then wired to the 3V3 socket. Readings from our board on 9 Nov 2026, five each:

| A0 connected to | Expected `adc` output | Measured on our board |
| --- | --- | --- |
| GND | raw near 0, near 0 mV | raw 0 to 2 (0 to 2 mV) |
| 3V3 | raw near 4095, near 3300 mV | raw 4091 to 4095 (3297 to 3300 mV) |

Repeat it after any change to the ADC code. Unplug the USB cable before moving the wire, and never connect A0 to the 5V socket. The ADC pin tolerates 3.3 V only.

## Design notes

- `gpio_write` uses the `BSRR` register, so one write sets or clears a single pin without reading the port first.
- The GPIO clock bit is worked out from the port address, so every port uses the same code.
- The UART baud divisor is `pclk / baud`, rounded. For 16 MHz and 115200 baud that is 139, an error of about 0.08%.
- The ADC uses integer millivolts because the FPU is off. External channels sample for 56 cycles. The internal reference samples for 480 cycles, because it needs at least 10 us.
- `adc_read` waits for the end of conversion with a timeout, so a missing or unpowered ADC returns `ADC_READ_ERROR` instead of hanging.

## Limitations

- All driver calls are polling and blocking. Interrupt-driven UART receive with a ring buffer and DMA are not implemented.
- Only USART2 and GPIO ports A and C have been run on the board.
- `uart_write` has not been run on the board yet.
- The ADC has been checked on external channel 0 (A0) and on the internal reference (channel 17). The other external channels use the same code but have not been run, and no mid-scale external voltage has been tested.
- The UART driver does not handle receive overrun or framing errors, and the data register holds one byte, so text pasted quickly can lose characters.
- The ADC reads one channel at a time, and results depend on VDDA being close to 3.3 V.

## Useful details

1. Exit picocom with Ctrl+A, then Ctrl+X.
2. The black RESET button restarts the program. Use it when a message prints only once at startup, so you can open the terminal first and then catch it.
3. The port number can change after unplugging or reflashing, so run `ls /dev/tty.usb*` again each time. On Linux the port usually appears as `/dev/ttyACM0`, and on Windows as a COM port in Device Manager.
4. Close every other serial program before opening picocom. Two programs on one port can split the incoming data.
5. Run `make clean` after switching branches or machines. Stale `.d` files can cause a `No rule to make target ... stdint.h` error.
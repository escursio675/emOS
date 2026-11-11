#projects 

# P3: Hardware Drivers (GPIO, UART, ADC)

P3 owns the hardware drivers of MiniOS. GPIO, UART and ADC are written and verified on a Nucleo-F401RE board, together with shell self-tests for GPIO and UART.

## Status

| Part | Files | State | Verified on the board |
| --- | --- | --- | --- |
| GPIO | `inc/gpio.h`, `src/gpio.c` | Done | LED blink, button read, shell commands, `gpio test` (10 of 10 checks) |
| UART (USART2) | `inc/uart.h`, `src/uart.c` | Done | Transmit, receive, line read, shell prompt, `uart test`, error counters |
| ADC (ADC1) | `inc/adc.h`, `src/adc.c` | Done | Internal 1.21 V reference, and A0 wired to GND and to 3V3 |
| Self-tests | `inc/selftest.h`, `src/selftest.c` | Done | `gpio test` and `uart test` pass |

All drivers use the register map in `inc/stm32f401re.h`. There is no HAL and no floating point (the FPU is not enabled at startup).

## Hardware and settings

| Item | Detail |
| --- | --- |
| Board | Nucleo-F401RE (STM32F401RE, Cortex-M4), clock 16 MHz HSI |
| LED | LD2 on PA5 |
| Button | B1 on PC13. Active-low: high when released, low when pressed. Pressing it also raises the EXTI interrupt, which toggles LD2 (see the interrupts report) |
| UART | USART2, PA2 (TX) and PA3 (RX), alternate function 7, 115200 baud, 8 data bits, no parity, 1 stop bit |
| ADC | ADC1, 12-bit. A0 is PA0 = channel 0. Readings assume VDDA = 3.3 V |
| GPIO test pin | PB0 (Arduino A3). Leave it unconnected when running `gpio test` |
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
| `uart_getc` | Waits for one byte and returns it. Counts any receive error flagged with that byte |
| `uart_data_available` | Returns 1 if a byte is waiting, without blocking |
| `uart_read_line(usart, buf, maxlen)` | Reads a typed line with echo and backspace. Stops at Enter or after `maxlen - 1` characters |
| `uart_get_errors(usart, out)` | Copies the overrun, noise, framing and parity counters of a USART |
| `uart_clear_errors(usart)` | Resets those counters to zero |

**ADC** (`adc.h`)

| Function | What it does |
| --- | --- |
| `adc_init()` | Enables the ADC1 clock, powers it on and switches on the internal reference |
| `adc_channel_config(channel)` | Selects the channel for the next conversion |
| `adc_read(channel)` | Runs one conversion. Returns 0 to 4095, or `ADC_READ_ERROR` (0xFFFF) if it never finishes |
| `adc_to_millivolts(raw)` | Converts a reading to millivolts (`raw * 3300 / 4095`, integer math) |
| `adc_estimate_vdda_mv(raw)` | Estimates the supply voltage from a reading of `ADC_CHANNEL_VREFINT` |

**Self-tests** (`selftest.h`)

| Function | What it does |
| --- | --- |
| `selftest_gpio()` | Runs the wire-free GPIO test on PB0 and prints the results over USART2 |
| `selftest_uart()` | Runs the three-stage UART test and prints the results over USART2 |

## Shell commands for the drivers

| Command | What it does |
| --- | --- |
| `led on`, `led off`, `led toggle` | Control LD2 with `gpio_write` and `gpio_toggle` |
| `button` | Prints whether B1 is pressed, using `gpio_read` |
| `adc` | One conversion on A0, printed as a raw value and in millivolts |
| `adc vref` | Reads the internal 1.21 V reference and estimates VDDA. Needs no wiring |
| `gpio test` | Wire-free GPIO self-test on PB0 |
| `uart test` | Three-stage UART self-test. Needs a person at the keyboard and takes about 12 seconds |

With nothing connected, `adc` reads a floating pin, so its value is arbitrary and changes between runs. That is normal.

## Building and flashing

1. Build with `make clean` and then `make`. A successful build prints nothing.
2. Plug in the board and flash with `make flash`. Success ends with `Flash written and verified! jolly good!`.
3. For UART tests, find the port with `ls /dev/tty.usb*` on a Mac or `/dev/ttyACM0` on Linux. Open it with `picocom -b 115200 /dev/tty.usbmodemXXXX` or `picocom -b 115200 /dev/ttyACM0`.

## Testing

After flashing, open the terminal and press the black RESET button. The banner and a `>` prompt appear. Then run the commands below.

### Basic commands

| Command | Expected |
| --- | --- |
| `led on`, `led off` | LD2 turns on and off, with `LED turned ON` and `LED turned OFF` |
| `led toggle` | LD2 flips each time, with `LED toggled` |
| `button` with B1 untouched | `Button is NOT pressed` |
| `button` while holding B1 | `Button is PRESSED` |

Typing and editing also work: Backspace erases the last character on screen and in the buffer, an empty line just shows a new prompt, and a line stops at 63 characters.

### gpio test

Make sure nothing is plugged into the A3 socket, then run `gpio test`. It configures PB0 in different ways and reads the pin back each time:

```
GPIO self-test on PB0 (Arduino A3). Nothing may be plugged into A3.
  input, pull-up: HIGH  PASS
  input, pull-down: LOW  PASS
  push-pull, drive high: HIGH  PASS
  push-pull, drive low: LOW  PASS
  push-pull + pull-down, drive high: HIGH  PASS
  open-drain + pull-down, drive high: LOW  PASS
  open-drain + pull-up, drive high: HIGH  PASS
  open-drain + pull-up, drive low: LOW  PASS
  toggle once (low to high): HIGH  PASS
  toggle twice (back to low): LOW  PASS
Result: ALL PASS (10 of 10 checks passed)
```

What the checks show:

- **Pull-up and pull-down:** an unconnected input follows the internal pull resistor, so the pull settings in `PUPDR` work, and so does the clock of port B.
- **Push-pull:** `gpio_write` through `BSRR` drives the pin both ways, even against a pull-down.
- **Open-drain:** a pin driven high is only released, so it follows the pull resistor instead (low with the pull-down, high with the pull-up), while a low is always driven.
- **Toggle:** `gpio_toggle` flips the pin each time.

### uart test

Run `uart test` and follow the prompts:

```
UART self-test (3 stages)
1) uart_write. The next line should show exactly MiniOS and nothing after it:
   MiniOS
2) uart_data_available. Press any key within 5 seconds...
   key received, code 99: PASS
3) receive errors. In 2 seconds I will say GO. Then type 6 or more keys quickly.
   GO (nothing is echoed during this stage)
   overrun 1, noise 0, framing 0: PASS, lost bytes were detected
```

1. **Stage 1:** `uart_write` sends exactly the 6 bytes asked for, even though the text it reads from is longer.
2. **Stage 2:** `uart_data_available` returns without blocking, and `uart_getc` returns the key you pressed. The code depends on the key (99 is `c`).
3. **Stage 3:** after `GO`, type 6 or more keys quickly. The receive register holds one byte, so the extra bytes arrive while nothing reads them and are counted as an overrun. The count is 1 or more.

### adc vref and adc

`adc vref` should print a raw value of about 1500 and an estimated VDDA of about 3300 mV. On our board it reads raw 1497 to 1499 and VDDA 3306 to 3310 mV. Anything from roughly 1450 to 1550 raw is normal, because the reference may be anywhere from 1.18 V to 1.24 V.

**ADC check with a jumper wire.** Run `adc` several times while A0 is wired to GND, then wired to the 3V3 socket. Readings from our board on 9 and 10 Nov 2026:

| A0 connected to | Expected `adc` output | Measured on our board |
| --- | --- | --- |
| GND | raw near 0, near 0 mV | raw 0 to 2 (0 to 2 mV) |
| 3V3 | raw near 4095, near 3300 mV | raw 4091 to 4095 (3297 to 3300 mV) |

Unplug the USB cable before moving the wire, and never connect A0 to the 5V socket. The ADC pin tolerates 3.3 V only.

## Design notes

- `gpio_write` uses the `BSRR` register, so one write sets or clears a single pin without reading the port first.
- The GPIO clock bit is worked out from the port address, so every port uses the same code.
- The UART baud divisor is `pclk / baud`, rounded. For 16 MHz and 115200 baud that is 139, an error of about 0.08%.
- `uart_getc` reads the status register once, counts any error flags in it, then reads the data register. That read order also clears the flags in hardware.
- The ADC uses integer millivolts because the FPU is off. External channels sample for 56 cycles. The internal reference samples for 480 cycles, because it needs at least 10 us.
- `adc_read` waits for the end of conversion with a timeout, so a missing or unpowered ADC returns `ADC_READ_ERROR` instead of hanging.
- The self-tests live in `selftest.c`, so the shell in `main.c` only needs one line per command.

## Limitations

- All driver calls are polling and blocking, so `uart_read_line` waits until a line is typed.
- The UART receive register holds one byte. Bytes that arrive while nothing is reading are lost, and the driver counts them as overruns.
- ADC results depend on VDDA being close to 3.3 V, and one channel is converted at a time.

## Future scope

- Interrupt-driven UART receive with a ring buffer, so no bytes are lost and the shell can do other work while waiting.
- DMA for UART transmit and ADC sampling.
- Shell commands with arguments, such as `gpio write PA5 1`, `gpio read PC13` and `adc 4`.
- ADC extras: converting several channels in one sequence, and the temperature and battery channels.
- Ignore non-printable bytes (for example arrow-key escape sequences) in `uart_read_line`, so a stray key press cannot spoil a command.
- Further drivers such as SPI and I2C.

## Useful details

1. Exit picocom with Ctrl+A, then Ctrl+X.
2. The black RESET button restarts the program. Use it when a message prints only once at startup, so you can open the terminal first and then catch it.
3. The port number can change after unplugging or reflashing, so run `ls /dev/tty.usb*` again each time. On Linux the port usually appears as `/dev/ttyACM0`, and on Windows as a COM port in Device Manager.
4. Close every other serial program before opening picocom. Two programs on one port can split the incoming data.
5. Run `make clean` after switching branches or machines. Stale `.d` files can cause a `No rule to make target ... stdint.h` error.
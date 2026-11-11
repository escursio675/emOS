# emOS Shell: Work Done

**Target:** Nucleo-F401RE (STM32F401RE, Cortex-M4, 16 MHz). Bare-metal C, no HAL, no RTOS.
**Console:** USART2, 115200 8N1, over the board's USB cable.
**Not done:** the optional blinking cursor, skipped on purpose.

---

## 1. Before and after

**Before:** one `if / else if` chain in `main.c`. Arrow keys were echoed and corrupted the line, commands had spaces (`led on`, `adc vref`, `gpio test`), and nothing ran while waiting for a key.

**After:** a small layered shell with a command table, 16 single-word commands, single-key menus, and a status LED.

| Piece | Job |
| --- | --- |
| `keyfilter` | Drops unwanted keys (arrows, Tab, punctuation). Turns CR / LF / CR+LF into one Enter. |
| `console` | All text and key I/O. Idle hook runs while waiting. |
| `strutil` | Trim and number parsing, no `<string.h>`. |
| `shell` + `cmd_table` | Reads a line, looks it up in one table, runs it. |
| `commands/` | One small file per command. |
| `menu` | Reusable press-one-key menu (`led`, `irq`, `selftest`). |
| `status_led` | Sole owner of LD2. Shows idle / busy / OK / error. |

Flow: `UART -> keyfilter -> console -> shell_execute -> cmd_table -> cmd_xxx -> status_led`

---

## 2. What was built

**Input layer.** `keyfilter` swallows whole escape sequences, so arrow keys do nothing. Only letters, digits, space, Backspace and Enter get through. Lines max out at 63 chars.

**Shell core.** `shell_run()` prints `> `, reads a line, trims it, and dispatches via `cmd_table.c` (`{name, summary, handler}`). Handlers return `CMD_OK` or `CMD_FAIL`. Unknown commands print an error and point to `help`. Blank lines print nothing. `main.c` shrank to init, greet, `shell_run()`.

**Basic commands.** `help` is generated from the table. `clear` sends ANSI codes. `greet` runs at boot. `uptime` uses SysTick. `hexdump` prompts for address and length, and only reads flash (`0x08000000-0x0807FFFF`) and SRAM (`0x20000000-0x20017FFF`), so a bad address cannot HardFault.

**Status LED.** `status_led_update()` keeps one deadline and never blocks. `shell_execute()` sets BUSY, then OK or ERROR from the handler result. The button ISR now only counts presses; the idle hook shows one OK flash. This removes the old ISR-vs-shell race on the LED. HardFault ends in `status_led_fault_forever()`.

**`led` menu.** Keys: 1 On, 2 Off, 3 Toggle, 4 Blinky, q Quit. Takes manual LED control on entry and releases it on quit. Blinky asks for 1-20 blinks/s, uses a drift-free cadence from `uptime_ms()`, and stops on any key.

**`irq` menu.** Path: `PC13 -> EXTI13 -> NVIC IRQ 40 -> ISR -> button_press_count`. Keys: 1 Status, 2 Software trigger (`EXTI->SWIER`), 3 Wait for button (10 s, `wfi`), 4 Mask/Unmask, 5 Reset counter. Presses while masked collapse into one (single pending bit). Quitting while masked re-enables the IRQ. `fault` is a separate command (undefined instruction, escalates to HardFault).

**Single-word rule and extras.**
- `adc vref` became `vref`.
- `gpio test` and `uart test` became the `selftest` menu (GPIO: 10 checks on PB0/A3, leave A3 unplugged; UART: 3 stages, needs a person typing).
- New `version` and `reboot` (`SCB->AIRCR` SYSRESETREQ).

---

## 3. Commands

One lowercase word each, case-sensitive, no arguments.

| Command | Does | Kind |
| --- | --- | --- |
| `help` | Lists all commands | plain |
| `clear` | Clears screen | plain |
| `greet` | Banner | plain |
| `uptime` | Time since boot | plain |
| `hexdump` | Hex + ASCII dump (prompts) | prompts |
| `led` | 1 On, 2 Off, 3 Toggle, 4 Blinky | menu |
| `irq` | Interrupt demo | menu |
| `button` | Blue button state | plain |
| `adc` | A0 raw + mV | plain |
| `vref` | Internal reference, estimated VDDA | plain |
| `selftest` | 1 GPIO, 2 UART | menu |
| `fault` | Deliberate HardFault | plain |
| `mem` | Heap stats | plain |
| `memtest` | `kmalloc` / `kfree` demo | plain |
| `version` | Firmware, build time, compiler | plain |
| `reboot` | Software reset | plain |

Removed or renamed: `led on/off/toggle/auto` -> `led`; `adc vref` -> `vref`; `gpio test`, `uart test` -> `selftest`; `stats` -> `irq` status.

---

## 4. Status LED (LD2, PA5)

| Situation | LD2 |
| --- | --- |
| Idle at prompt | Heartbeat: 100 ms on, 1.9 s off |
| Command running | Solid on |
| OK | 2 quick flashes (100/100 ms), then heartbeat |
| Error / unknown command | 6 fast flashes (50/50 ms), then heartbeat |
| Button press at prompt | One OK flash |
| Button press while busy | No flash, still counted |
| Inside `led` menu | Manual control, no heartbeat |
| HardFault | Fast blink forever, RESET to recover |

Blank lines give no flash. `irq` and `selftest` count as busy, so LD2 is solid inside them.

---

## 5. Files

**New:** `inc/status_led.h`, `inc/shell/{strutil,keyfilter,console,shell,menu}.h`, `inc/commands/commands.h`, `src/status_led.c`, `src/shell/{strutil,keyfilter,console,shell,cmd_table,menu}.c`, `src/commands/cmd_{help,clear,greet,uptime,hexdump,led,irq,selftest,version,reboot}.c`.

**Moved out of `main.c`:** `src/commands/cmd_{mem,adc,system}.c`.

**Changed:**

| File | Change |
| --- | --- |
| `src/main.c` | Was the whole shell. Now: init, console, status LED, idle hook, greet, `shell_run()`. |
| `src/interrupts.c` | ISR only clears the flag and counts. HardFault handler ends in `status_led_fault_forever()`. |
| `src/selftest.c` | Uses `console_*`. Started from the `selftest` menu. |
| `Makefile` | One `OBJS +=` line per new `.c` file. |

Drivers (`gpio`, `uart`, `adc`, `systick`, `memory`, `hexdump`) are used, not rewritten.

---

## 6. Board test

```bash
make clean && make && make flash
picocom -b 115200 /dev/ttyACM0      # Mac: /dev/tty.usbmodemXXXX; exit with Ctrl+A, Ctrl+X
```

Press the black RESET button. Expect the `emOS` banner, `Welcome! Type 'help' to see the available commands.`, then `>`. Close other serial programs first.

**Input rules**

| Test | Steps | Expected |
| --- | --- | --- |
| Unknown command | `foo` | `Unknown command: foo` + help hint |
| Spaces / blank | `  mem  `; empty Enter | `mem` runs; blank prints only a new prompt |
| Case | `MEM` | `Unknown command: MEM` |
| Punctuation | `me!m-_` | Only `mem` appears |
| Backspace | `memx`, Backspace, Enter | `mem` runs |
| Arrows | Up/Down/Left/Right at empty prompt | Nothing appears, cursor still |
| Arrow mid-line | `m`, Left, `e`, `m`, Enter | Runs `mem`, no stray characters |
| Other keys | Home, End, Delete, Tab, F1-F4, Ctrl-A/E/C | Nothing appears, typing still works |
| Long line | Paste 80 letters, Enter | Only 63 kept, `Unknown command` |
| CR+LF | `picocom --omap crcrlf`, Enter on empty line | One new prompt, not two |

**Commands**

| Test | Steps | Expected |
| --- | --- | --- |
| `help` | `help` | 16 rows, one-word names |
| `clear` | `mem`, `clear` | Screen wiped |
| `uptime` | Wait 5 s, `uptime` | About `0h 00m 05s (5 s)` |
| `hexdump` default | `hexdump`, Enter, Enter | 8 lines from `08000000`, starts `00 80 01 20` |
| `hexdump` RAM | `20000000`, `32` | 2 lines |
| `hexdump` safety | `40020000`, `16`; then `0807FFF0`, `32` | `Invalid range`, no HardFault |
| `hexdump` bad input | `zzz`; then length `0`, `2000` | Invalid address / length messages |
| `mem`, `memtest` | Run each | Stats shown; memtest ends OK |
| `adc` | Wire A0 to GND, then to 3V3 (never 5V) | Raw 0-2, then about 4091-4095 |
| `vref` | `vref` | Raw about 1450-1550, VDDA about 3200-3400 mV |
| `button` | Run, then run holding blue button | NOT pressed / PRESSED |
| `version` | `touch src/commands/cmd_version.c`, rebuild, flash, `version` | `Built:` shows new time |
| `reboot` | `reboot` x3 | `Rebooting...` prints fully, banner returns each time |
| Old names | `adc vref`, `gpio test`, `led on`, `stats` | Each `Unknown command`, nothing runs |

**Status LED**

| Test | Steps | Expected |
| --- | --- | --- |
| Heartbeat | Idle 10 s | About 5 short blinks |
| OK | `mem` | 2 quick flashes, then heartbeat |
| Error | `foo` | 6 fast flashes, then heartbeat |
| Busy | `hexdump`, wait at prompt | Solid on until done |
| Button | Press blue button at prompt | One OK flash |
| No lost keys | Type `help` fast; paste `mem` + Enter | Every character once, heartbeat continues |

**`led` menu**

| Test | Steps | Expected |
| --- | --- | --- |
| Open | `led` | Menu shown, LED off, no heartbeat |
| Keys | `1`, `2`, `3`, `3`, `1`, `2` | Works with no Enter, stays in menu |
| Invalid | `z`, `5`, Space | `Invalid choice...` |
| Blinky | `4`, Enter (stopwatch 10 s) | About 10 blinks (1/s) |
| Blinky fast | `4`, `5`, Enter | About 5 blinks/s |
| Stop | Any key while blinking | `Stopped.`, LED off |
| Arrow | Up while blinking | Keeps blinking |
| Bad rate | `4` then `0`, `21`, `abc` | `Invalid rate (1-20 ...)`, menu stays |
| Quit | `q` | Prompt returns, OK flashes, heartbeat |

**`irq` menu**

| Test | Steps | Expected |
| --- | --- | --- |
| Status | `irq`, `1` | After RESET: count 0, IMR yes, PR no, ISER yes, ISPR no |
| Software trigger | `2`, `1` | `Count: 0 -> 1` without touching the button |
| Real press | Press blue button, `1` | Count +1, LD2 stays solid |
| Wait | `3`, press button within 10 s | `Button interrupt received` |
| Wait timeout / cancel | `3` alone; `3` then `x` | Timeout after about 10 s; `Cancelled.` at once |
| Mask | `4`, press button, `1` | Count unchanged, PR yes, ISPR yes |
| Unmask | `4` again | Count +1, "press made while masked ran just now" |
| Collapse | Mask, press 5 times, unmask | Count +1, not +5 |
| Reset counter | `5`, `1` | Count 0 |
| Quit masked | Mask, `q` | IRQ re-enabled, message printed |

**`selftest` menu**

| Test | Steps | Expected |
| --- | --- | --- |
| GPIO | A3 unplugged, `1` | `ALL PASS (10 of 10 checks passed)` |
| UART | `2`, press a key within 5 s, after `GO` type 6+ keys fast | Key received, overrun count > 0, `PASS` |
| Invalid / quit | `z`, `3`, `q` | `Invalid choice...`; back to `>` |

**Fault and recovery**

| Test | Steps | Expected |
| --- | --- | --- |
| HardFault | `fault` | `[KERNEL PANIC] HardFault Triggered`, `System Halted.`, LD2 fast blink, no input response |
| Recovery | Press RESET | Banner, heartbeat, shell works |

---

## 7. Renode guide

Renode runs the firmware with no board.

**Files:** `stm32f401re.repl` (simulated board: flash, SRAM, USART2, GPIO, EXTI/NVIC, blue button on PC13, LD2 on PA5, ADC1) and `emOS.resc` (start script).

**Steps**
1. `make clean && make` (Renode loads `firmware.elf`).
2. In `emOS.resc`, change the two paths to your project folder:
   ```
   machine LoadPlatformDescription @/your/path/to/emOS/stm32f401re.repl
   sysbus LoadELF @/your/path/to/emOS/firmware.elf
   ```
3. `renode emOS.resc`
4. Type shell commands in the **`sysbus.usart2` window**. Type simulator commands in the **Monitor**.

**What `emOS.resc` does:** creates the machine, loads the board file and ELF, opens the UART window, logs every LD2 change (`logLevel -1 sysbus.gpioPortA.ld2`), feeds fake ADC values, and starts.

```
sysbus.adc1 FeedSample 2048 0 -1      # A0 always 2048 (about 1650 mV)
sysbus.adc1 FeedSample 1501 17 -1     # VREFINT always 1501 (VDDA about 3301 mV)
```

Without the `FeedSample` lines, `adc` and `vref` read 0.

**Monitor commands**

| Command | Does |
| --- | --- |
| `sysbus.gpioPortC.userButton Press` / `Release` | One press for the interrupt |
| `pause` / `start` | Pause / resume |
| `machine Reset` | Reset the simulated CPU |
| `sysbus LoadELF @<path>/firmware.elf` | Reload after a rebuild |
| `sysbus.adc1 FeedSample <value> <channel> -1` | Set an ADC channel value |

**Reset recipe (after `fault` or a rebuild)**
```
pause
machine Reset
sysbus LoadELF @<path>/firmware.elf
sysbus.adc1 FeedSample 2048 0 -1
sysbus.adc1 FeedSample 1501 17 -1
start
```

**Reading the LED log:** heartbeat is ON, OFF 100 ms later, ON again about 2 s after the first. OK is 2 pairs at 100 ms. ERROR is 6 pairs at 50 ms. BUSY is one ON that stays on.

**Works in Renode:** typing, all commands, `irq` (use `Press`/`Release` for the button), `fault`, LED patterns, `adc`/`vref` (with the fake values above).

**Test on the board instead:** arrow/special keys and `clear` (the UART window may not send or render ANSI), Blinky stopwatch timing, `reboot`, GPIO self-test (pull-up/down may not be modelled), UART self-test stage 3 (needs fast real typing).

> Renode is a helper. Final sign-off belongs on the real Nucleo.

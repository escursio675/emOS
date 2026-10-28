
| **Test Platform:** [Renode](https://antmicro.com/platforms/renode/) 1.17.0
## Integration

Interrupts have been **INTEGRATED** into emOS:

- **EXTI + NVIC:** The onboard Blue Button (PC13) raises a hardware interrupt (EXTI line 13, falling edge, IRQ 40), replacing polling. The ISR toggles the LED and increments a press counter.
- **HardFault Handler:** Catches CPU faults, prints a panic message over USART2 and halts the system.

## Test Setup
- **Monitor Window:** Stands in for the physical board (simulates button press and release).
- **UART Terminal Window:** Stands in for the serial terminal (type commands, read output).

## TEST 1: Blue Button Hardware Interrupt 

**Goal:** Confirm the button triggers an interrupt and the counter increments **only on press**.

| Step | Window | Command | Output |
|---|---|---|---|
| 1 | Terminal | `stats` | `Button interrupt trigger count: 0` |
| 2 | Monitor | `sysbus.gpioPortC.userButton Press` | |
| 3 | Terminal | `stats` | `Button interrupt trigger count: 1` |
| 4 | Monitor | `sysbus.gpioPortC.userButton Release` | |
| 5 | Terminal | `stats` | `Button interrupt trigger count: 1` |
| 6 | Monitor | `sysbus.gpioPortC.userButton Press` | |
| 7 | Terminal | `stats` | `Button interrupt trigger count: 2` |
| 8 | Monitor | `sysbus.gpioPortC.userButton Release` | |
| 9 | Terminal | `stats` | `Button interrupt trigger count: 2` |

**Result: PASS.** The count rises on each press and stays unchanged on release.

## TEST 2: Hardfault Handling

**Goal:** Confirm a CPU fault is caught and reported.

**Command (Terminal):** `fault` (executes an undefined instruction, which escalates to a HardFault)

**Output:**
```
Simulating HardFault via undefined instruction...
===================================
 [KERNEL PANIC] HardFault Triggered
===================================
System Halted.
```

**Result: PASS.**

## Conclusion

Both tests passed, confirming that the interrupt integration is working.

## Notes

- Writing to an invalid address (e.g. `0xFFFFFFF0`) faults on real hardware but **NOT** in Renode, so an undefined instruction is used instead. It behaves the same on both.
- After a fault the system stays halted. Reset with the board's RESET button, or in Renode: `pause`, `machine Reset`, `sysbus LoadELF $CWD/firmware.elf`, `start`.

 
## Building and Flashing
The build and flash process remains the same as specified in "P3-Drivers-GPIO-and-UART-Report.md". Two new verifications may be perfomed:

1. `arm-none-eabi-size firmware.elf` to primarily check the size of the bss section which should be around 4096 bytes.
2. `arm-none-eabi-nm -n firmware.elf | grep heap_area` to check the heap address. This address should start with `0x2000`.

## Commands
**mem**
This command shows the total, used, free, largest free block and number of blocks. The heap size, as of now, is restricted to 4096 byte static array.

**memtest**
This command runs a scripted simulation of variour heap allocations.

Sample output:
```
> memtest
a = kmalloc(100): 0x20000010
b = kmalloc(200): 0x20000088
Heap:
  Total:        4096
  Used:         336
  Free:         3760
  Largest free: 3744
  Blocks:       3
kfree(a); c = kmalloc(50): 0x20000010 (reused a's block)
kmalloc(KHEAP_SIZE): NULL (out of memory, as expected)
freed everything
Heap:
  Total:        4096
  Used:         0
  Free:         4096
  Largest free: 3744
  Blocks:       4
```

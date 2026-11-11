/**
 * cmd_mem.c
 *
 * Heap commands: `mem` (statistics) and `memtest` (scripted allocator demo).
 */

#include <stdint.h>
#include <stddef.h>
#include "commands/commands.h"
#include "shell/console.h"
#include "memory.h"

/* mem: allocator statistics. */
cmd_result_t cmd_mem(void)
{
    heap_stats_t s;

    kmalloc_stats(&s);

    console_puts("Heap:\r\n");
    console_puts("  Total:        "); console_put_uint((uint32_t)s.total_bytes);  console_puts("\r\n");
    console_puts("  Used:         "); console_put_uint((uint32_t)s.used_bytes);   console_puts("\r\n");
    console_puts("  Free:         "); console_put_uint((uint32_t)s.free_bytes);   console_puts("\r\n");
    console_puts("  Largest free: "); console_put_uint((uint32_t)s.largest_free); console_puts("\r\n");
    console_puts("  Blocks:       "); console_put_uint((uint32_t)s.block_count);  console_puts("\r\n");
    return CMD_OK;
}

/* memtest: exercises kmalloc/kfree on the board.
 * Returns CMD_FAIL only if the "must fail" allocation unexpectedly succeeds. */
cmd_result_t cmd_memtest(void)
{
    void *a = kmalloc(100);
    void *b = kmalloc(200);
    void *c;
    void *big;
    int   oom_ok;

    console_puts("a = kmalloc(100): "); console_put_hex32((uint32_t)(uintptr_t)a); console_puts("\r\n");
    console_puts("b = kmalloc(200): "); console_put_hex32((uint32_t)(uintptr_t)b); console_puts("\r\n");
    (void)cmd_mem();

    kfree(a);
    c = kmalloc(50);
    console_puts("kfree(a); c = kmalloc(50): "); console_put_hex32((uint32_t)(uintptr_t)c);
    console_puts(c == a ? " (reused a's block)\r\n" : " (different block)\r\n");

    big = kmalloc(KHEAP_SIZE);
    oom_ok = (big == NULL);
    console_puts("kmalloc(KHEAP_SIZE): ");
    console_puts(oom_ok ? "NULL (out of memory, as expected)\r\n" : "UNEXPECTED: succeeded\r\n");

    kfree(b);
    kfree(c);
    kfree(big);              /* NULL is ignored */
    console_puts("freed everything\r\n");
    (void)cmd_mem();

    return oom_ok ? CMD_OK : CMD_FAIL;
}
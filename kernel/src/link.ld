/* link.ld — kernel8.img loads at 0x80000 on Pi 4 (BCM2711). */
ENTRY(_start)

SECTIONS
{
    . = 0x80000;
    .text.boot : { *(.text.boot) }
    .text      : { *(.text) }
    .rodata    : { *(.rodata) }
    .data      : { *(.data) }
    .bss       : {
        . = ALIGN(8);
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        . = ALIGN(8);
        __bss_end = .;
    }
    /DISCARD/ : { *(.comment) *(.note*) *(.eh_frame*) }
}

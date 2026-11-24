/* link.ld: the image is loaded at 0x80000 (kernel_address in config.txt).
 * 0x0..0x80000 belongs to the firmware on the Pi 5 and is not used. */
ENTRY(_start)

SECTIONS
{
    . = 0x80000;
    __image_start = .;

    .text : {
        KEEP(*(.text.boot))
        *(.text .text.*)
    }
    .rodata : { *(.rodata .rodata.*) }
    .data : { *(.data .data.*) }

    . = ALIGN(16);
    __bss_start = .;
    .bss (NOLOAD) : {
        *(.bss .bss.*)
        *(COMMON)
    }
    . = ALIGN(16);
    __bss_end = .;

    . += 0x20000;               /* stack, 128 KiB, grows down from here */
    . = ALIGN(16);
    __stack_top = .;
    __image_end = .;

    /DISCARD/ : { *(.comment) *(.note*) *(.eh_frame*) }
}

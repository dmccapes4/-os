/* start.S: entry from the firmware on lirvol (Raspberry Pi 5, BCM2712).
 *
 * The bootloader in the EEPROM loads the image at 0x80000 and enters at
 * _start with the address of the device tree in x0. Entry is expected at
 * EL2; EL3 and EL1 are handled too. This file drops to EL1, sets the
 * stack, clears the bss and calls kernel_main(dtb).
 *
 * Two things differ from the Pi 4 code in ../../kernel/src/boot.S:
 *   - 0x0..0x80000 is reserved for the firmware's EL3 code on this board,
 *     so the stack is placed after the image, not below it.
 *   - The core number is in Aff1 of MPIDR on this processor, not Aff0.
 */

.section ".text.boot", "ax"
.globl _start
_start:
    mov     x20, x0                     /* device tree address */

    /* Only the first core runs. On this board the firmware keeps the
     * others off until asked, so this branch is a precaution. */
    mrs     x1, mpidr_el1
    tst     x1, #0xffff
    b.ne    park

    mrs     x0, CurrentEL
    lsr     x0, x0, #2
    cmp     x0, #3
    b.eq    from_el3
    cmp     x0, #2
    b.eq    from_el2
    b       at_el1

from_el3:
    mov     x0, #0x5b1                  /* SCR_EL3: NS, HCE, RW (EL2 is 64-bit) */
    msr     scr_el3, x0
    mov     x0, #0x3c9                  /* SPSR_EL3: EL2h, D A I F masked */
    msr     spsr_el3, x0
    adr     x0, from_el2
    msr     elr_el3, x0
    eret

from_el2:
    ldr     x0, =vectors                /* a fault before the drop is reported too */
    msr     vbar_el2, x0

    mrs     x0, cnthctl_el2             /* EL1 may read the counter and timer */
    orr     x0, x0, #3
    msr     cnthctl_el2, x0
    msr     cntvoff_el2, xzr

    mrs     x0, midr_el1
    msr     vpidr_el2, x0
    mrs     x0, mpidr_el1
    msr     vmpidr_el2, x0

    mov     x0, #0x33ff                 /* no traps to EL2 */
    msr     cptr_el2, x0
    msr     hstr_el2, xzr
    mov     x0, #(3 << 20)
    msr     cpacr_el1, x0

    mov     x0, #(1 << 31)              /* HCR_EL2.RW: EL1 is 64-bit */
    msr     hcr_el2, x0

    mov     x0, #0x0800                 /* SCTLR_EL1: reserved-one bits only. */
    movk    x0, #0x30d0, lsl #16        /* MMU off, caches off for now. */
    msr     sctlr_el1, x0

    mov     x0, #0x3c5                  /* SPSR_EL2: EL1h, D A I F masked */
    msr     spsr_el2, x0
    adr     x0, at_el1
    msr     elr_el2, x0
    eret

at_el1:
    ldr     x0, =vectors
    msr     vbar_el1, x0

    /* Instruction cache on, after discarding whatever it holds. With the
     * MMU off this is allowed and affects instruction fetches only. Data
     * accesses stay uncached. */
    ic      iallu
    isb
    mrs     x0, sctlr_el1
    orr     x0, x0, #(1 << 12)
    msr     sctlr_el1, x0
    isb

    ldr     x0, =__stack_top
    mov     sp, x0

    ldr     x0, =__bss_start
    ldr     x1, =__bss_end
1:  cmp     x0, x1
    b.hs    2f
    str     xzr, [x0], #8
    b       1b
2:
    mov     x0, x20
    bl      kernel_main

park:
    wfe
    b       park

// boot.S — kyrios B0/B1: the assembly only assembly can do (Pi 4 / BCM2711).
// Firmware loads kernel8.img at 0x80000 and enters at _start, possibly at
// EL3 or EL2 depending on firmware config. We: park secondary cores, drop
// to EL1 if needed, set the stack, zero BSS, and jump to kernel_main (C).
// It never comes back; kernel_main halting is this file's last service.

.section ".text.boot"
.globl _start
_start:
    // Only core 0 proceeds; 1-3 wait forever (B2 wakes them on purpose).
    mrs     x0, mpidr_el1
    and     x0, x0, #0xFF
    cbz     x0, primary
secondary:
    wfe
    b       secondary

primary:
    // If at EL2, drop to EL1 (firmware normally enters at EL2 on Pi 4
    // unless arm_64bit/config says otherwise; EL3 is handled too).
    mrs     x0, CurrentEL
    lsr     x0, x0, #2
    cmp     x0, #3
    b.eq    from_el3
    cmp     x0, #2
    b.eq    from_el2
    b       at_el1

from_el3:
    // EL3 -> EL2: non-secure, AArch64, then fall into the EL2 drop.
    mov     x0, #0x5b1              // SCR_EL3: NS | IRQ/FIQ routed | HCE | RW
    msr     scr_el3, x0
    mov     x0, #0x3c9              // SPSR_EL3: EL2h, interrupts masked
    msr     spsr_el3, x0
    adr     x0, from_el2
    msr     elr_el3, x0
    eret

from_el2:
    // Enable AArch64 at EL1 and drop.
    mov     x0, #(1 << 31)          // HCR_EL2.RW
    msr     hcr_el2, x0
    mov     x0, #0x3c5              // SPSR_EL2: EL1h, interrupts masked
    msr     spsr_el2, x0
    adr     x0, at_el1
    msr     elr_el2, x0
    // Do not touch sctlr_el1 here beyond the reset value; MMU is B3's rung.
    eret

at_el1:
    // Stack grows down from just below the kernel image.
    adr     x0, _start
    mov     sp, x0

    // Zero BSS (linker provides __bss_start/__bss_end).
    ldr     x0, =__bss_start
    ldr     x1, =__bss_end
bss_loop:
    cmp     x0, x1
    b.ge    bss_done
    str     xzr, [x0], #8
    b       bss_loop
bss_done:
    bl      kernel_main
halt:
    wfe
    b       halt

// vectors.S — kyrios B2: EL1 exception vector table (AArch64).
// Layout: 16 entries, 0x80 bytes apart, table 0x800-aligned (VBAR_EL1).
// We handle two entries for real:
//   Synchronous, current EL with SPx (offset 0x200) -> exception_report
//   IRQ,         current EL with SPx (offset 0x280) -> irq_handler
// Everything else reports as an unexpected exception and halts.

.macro ventry target
    .balign 0x80
    b \target
.endm

.section ".text.vectors"
.balign 0x800
.globl vectors
vectors:
    ventry sync_sp0        // 0x000 Synchronous, EL1 with SP0
    ventry unexpected      // 0x080 IRQ, EL1 with SP0
    ventry unexpected      // 0x100 FIQ
    ventry unexpected      // 0x180 SError
    ventry sync_spx        // 0x200 Synchronous, EL1 with SPx  <- brk lands here
    ventry irq_spx         // 0x280 IRQ, EL1 with SPx          <- timer lands here
    ventry unexpected      // 0x300 FIQ
    ventry unexpected      // 0x380 SError
    ventry unexpected      // 0x400 Synchronous, lower EL AArch64
    ventry unexpected      // 0x480 IRQ, lower EL
    ventry unexpected      // 0x500 FIQ
    ventry unexpected      // 0x580 SError
    ventry unexpected      // 0x600 Synchronous, lower EL AArch32
    ventry unexpected      // 0x680 IRQ
    ventry unexpected      // 0x700 FIQ
    ventry unexpected      // 0x780 SError

sync_sp0:
sync_spx:
    // Report ESR_EL1/ELR_EL1 and never return (the C side halts).
    mrs x0, esr_el1
    mrs x1, elr_el1
    bl  exception_report
1:  wfe
    b   1b

irq_spx:
    // Preserve the caller-saved set the C handler may clobber.
    sub sp, sp, #256
    stp x0,  x1,  [sp, #0]
    stp x2,  x3,  [sp, #16]
    stp x4,  x5,  [sp, #32]
    stp x6,  x7,  [sp, #48]
    stp x8,  x9,  [sp, #64]
    stp x10, x11, [sp, #80]
    stp x12, x13, [sp, #96]
    stp x14, x15, [sp, #112]
    stp x16, x17, [sp, #128]
    stp x18, x19, [sp, #144]
    stp x20, x21, [sp, #160]
    stp x22, x23, [sp, #176]
    stp x24, x25, [sp, #192]
    stp x26, x27, [sp, #208]
    stp x28, x29, [sp, #224]
    str x30, [sp, #240]
    bl  irq_handler
    ldp x0,  x1,  [sp, #0]
    ldp x2,  x3,  [sp, #16]
    ldp x4,  x5,  [sp, #32]
    ldp x6,  x7,  [sp, #48]
    ldp x8,  x9,  [sp, #64]
    ldp x10, x11, [sp, #80]
    ldp x12, x13, [sp, #96]
    ldp x14, x15, [sp, #112]
    ldp x16, x17, [sp, #128]
    ldp x18, x19, [sp, #144]
    ldp x20, x21, [sp, #160]
    ldp x22, x23, [sp, #176]
    ldp x24, x25, [sp, #192]
    ldp x26, x27, [sp, #208]
    ldp x28, x29, [sp, #224]
    ldr x30, [sp, #240]
    add sp, sp, #256
    eret

unexpected:
    mov x0, #0
    mrs x0, esr_el1
    mrs x1, elr_el1
    bl  exception_report
1:  wfe
    b   1b

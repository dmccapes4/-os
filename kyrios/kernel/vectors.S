/* vectors.S: exception vector table for EL1 (and for EL2 before the drop).
 *
 * Every one of the 16 entries saves the caller-saved registers and calls
 *     u64 exception_c(u64 slot, u64 esr, u64 elr, u64 far)
 * with slot counting the entries from 1. exception_c normally reports and
 * stops. During a guarded access (see probe_read32 in bcm2712.c) it
 * returns instead, and its return value is added to the return address so
 * that a faulting instruction is stepped over.
 */

.macro ventry slot
    .balign 0x80
    sub     sp, sp, #192
    stp     x0,  x1,  [sp, #0]
    stp     x2,  x3,  [sp, #16]
    stp     x4,  x5,  [sp, #32]
    stp     x6,  x7,  [sp, #48]
    stp     x8,  x9,  [sp, #64]
    stp     x10, x11, [sp, #80]
    stp     x12, x13, [sp, #96]
    stp     x14, x15, [sp, #112]
    stp     x16, x17, [sp, #128]
    stp     x18, x29, [sp, #144]
    str     x30,      [sp, #160]
    mov     x0, #\slot
    b       exception_common
.endm

.section ".text.vectors", "ax"
.balign 0x800
.globl vectors
vectors:
    ventry  1       /* current EL, SP0: synchronous */
    ventry  2       /*                  IRQ         */
    ventry  3       /*                  FIQ         */
    ventry  4       /*                  SError      */
    ventry  5       /* current EL, SPx: synchronous */
    ventry  6       /*                  IRQ         */
    ventry  7       /*                  FIQ         */
    ventry  8       /*                  SError      */
    ventry  9       /* lower EL, 64-bit: synchronous */
    ventry  10
    ventry  11
    ventry  12
    ventry  13      /* lower EL, 32-bit: synchronous */
    ventry  14
    ventry  15
    ventry  16

exception_common:
    mrs     x4, CurrentEL
    cmp     x4, #8                      /* EL2 */
    b.eq    1f
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    mrs     x3, far_el1
    b       2f
1:  mrs     x1, esr_el2
    mrs     x2, elr_el2
    mrs     x3, far_el2
2:  bl      exception_c
    cbz     x0, 4f
    mrs     x4, CurrentEL
    cmp     x4, #8
    b.eq    3f
    mrs     x1, elr_el1
    add     x1, x1, x0
    msr     elr_el1, x1
    b       4f
3:  mrs     x1, elr_el2
    add     x1, x1, x0
    msr     elr_el2, x1
4:  ldp     x0,  x1,  [sp, #0]
    ldp     x2,  x3,  [sp, #16]
    ldp     x4,  x5,  [sp, #32]
    ldp     x6,  x7,  [sp, #48]
    ldp     x8,  x9,  [sp, #64]
    ldp     x10, x11, [sp, #80]
    ldp     x12, x13, [sp, #96]
    ldp     x14, x15, [sp, #112]
    ldp     x16, x17, [sp, #128]
    ldp     x18, x29, [sp, #144]
    ldr     x30,      [sp, #160]
    add     sp, sp, #192
    eret

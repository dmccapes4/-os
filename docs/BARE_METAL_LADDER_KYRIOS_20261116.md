# Bare-Metal Ladder — κύριος from scratch (C + ARM assembly)

**Filed:** 2026-11-16, on your correction: *"C with arm like I said."*
**Status of other documents:** this ladder **supersedes** the implementation
track of `EXPERIMENT_LADDER_KYRIOS_20261116.md` (whose socket/receipt semantics
survive as the behavioural oracle) and **conflicts with** the README foundation
table and the Buildroot choice made earlier the same day. That conflict is
filed, not hidden: the bench stays Debian Bookworm; κύριος itself is no longer
a Linux userland on a distro rootfs. It is a bare-metal kernel, written by us,
in **C with AArch64 assembly where only assembly can go** (boot, exception
vectors, EL transitions). The Buildroot tree configured this morning is parked
under `_build/`, unused, until you say delete or repurpose.

The quoted guidance you brought is right in substance — C + ARM assembly is
the standard bare-metal pairing, Rust the modern alternative — with one
engineering caveat that shapes this whole ladder: **nearly every tutorial,
including Matyukevich's, targets Pi 3 / Pi 4 silicon (BCM2837/BCM2711).** Our
primary node is a Pi 5 (BCM2712), where Ethernet, USB, GPIO, and the GPIO-header
UART all live behind the **RP1 south bridge over PCIe**, documentation is
thinner, and QEMU has no Pi 5 machine at all. None of that changes the
language choice; all of it changes the order we climb in.

---

## The ladder (your eight rungs, re-founded on hardware)

Your rungs stand as the *destination*: space+port → receipt stream → documents
created → child processes as documents → second space → n spaces → an
application. Bare metal inserts the hardware rungs underneath them, because
on bare metal there is no kernel beneath us granting anything — **granting is
the thing we are writing.** A "port" with nobody home is just a number.

### B0 — Toolchain and boot skeleton *(new; before everything)*
`aarch64-linux-gnu-gcc` (already on the bench) driven freestanding
(`-ffreestanding -nostdlib -nostartfiles`), `boot.S` (park secondary cores,
set the stack, drop to EL1, zero BSS, jump to C), linker script at
`0x80000`, `kernel8.img` out. Bench lacks `qemu-system-aarch64`
(user-static only) — an operator-run install script under `tools/` adds it;
QEMU covers Pi-3-class practice, never Pi 5 proof.
**Pass:** `make` on the bench produces an image from a clean tree, twice,
byte-identical; `aarch64-linux-gnu-readelf` shows a static, nostdlib ELF.

### B1 — hello-kyrios, truly ✅ PASS 2026-11-16 *(your rung 1, done again at the metal)*

Receipt: `docs/witness/rung-b0-b1-20261116.md` + the screen photograph
beside it. 8,632 bytes, C and AArch64 assembly, no Linux underneath.
`kernel_main` writes **hello-kyrios** somewhere a human can witness it:
HDMI framebuffer via the firmware mailbox (works on both boards, needs no
serial adapter we may not own), UART where the board offers one honestly
(Pi 4 GPIO mini-UART; Pi 5 debug connector). This is the real hello world:
our code, first instructions after the firmware, no Linux anywhere.
**Pass:** a photograph of the screen (or a captured serial log) saying
hello-kyrios, filed under `docs/witness/`. Hello has now been said by us,
not by Debian on our behalf.

### B2 — Time and interrupts *(the port authority learns to say no, on schedule)*
GIC initialised, system timer ticking, exception vector table in assembly
with C handlers; ACT LED or an on-screen heartbeat under timer control.
**Pass:** a measured 10 Hz heartbeat (± tolerance read from the timer, not
counted in frames), and a deliberately raised synchronous exception is
caught, reported on screen, and halted cleanly — nothing crashes silently.

### B3 — Memory *(documents need somewhere to be)*
MMU on: identity-mapped page tables, EL1 kernel / EL0 user split reserved,
a page allocator and a small heap. **Pass:** allocate/free stress with a
canary check; a deliberate write into an unmapped page faults into B2's
handler and is reported with the faulting address. This rung is where "set
access to hardware" stops being a policy document and becomes page tables.

### B4 — Input *(a space cannot accept what it cannot hear)*
UART receive (line discipline by hand), framebuffer console with a
scrollback. USB keyboards are **deferred by name**: USB is a subsystem,
not a rung. **Pass:** type a line at `valseir`'s screen or serial, watch it
echo through our console, stored in a named buffer with bounds enforced —
an over-long line is refused and the refusal is shown (the first refusal
this OS ever issues, and it leaves a mark).

### B5 — Storage and the receipt ledger *(your rung 3, at the metal)*
SD host controller (SDHCI) read, then write; FAT32 on the boot card read
and appended to. The ledger becomes a file our kernel appends receipts to —
same JSONL shape, same `parent_hash` chain as filed (the Python tools and
the C-userspace prototype remain its bench-side verifiers).
**Pass:** boot, append three receipts, power-cycle without ceremony, boot
again and verify the chain *in our kernel*; the card then verifies again on
the bench with `ledger/receipt.py`. Port to NVMe (PCIe) is a Pi 5 sub-rung
much later, not smuggled in here.

### B6 — Documents, created from input *(your rung 4)*
A space becomes a kernel object: a name, a byte quota ("space size",
enforced by the allocator, not by prose), a document store. A line typed
at the console *creates a document* — minted id, content hash, stored in
the space, receipted. **Pass:** B4's line becomes a retrievable document;
quota exhaustion refuses the next creation and receipts the refusal.

### B7 — A child process, as a document *(your rung 5, at the metal)*
Documents that are programs: a tiny ELF (or flat image) loader, EL0
execution, `SVC` system calls (write-to-console, create-document, exit),
round-robin scheduler on B2's tick. The processor-document's hash is
checked before a single instruction runs — rung 5's tamper test, now
enforced by the MMU, not by a wrapper script.
**Pass:** parent document spawns child document; child's output and
receipts chain to the parent's; one flipped byte in the child → refusal
receipt, zero instructions executed.

### B8 — The network, and honest TCP ports *(your rung 2, at last)*
The biggest rung, sized honestly: Ethernet MAC driver, then ARP → IPv4 →
ICMP (ping answered), UDP, and a deliberately minimal TCP (listen, accept,
one connection, close — RFC-shaped subset, tested against the behaviour
the rung-2 prototype already pinned down). On Pi 4 this is the BCM2711
GENET; on Pi 5 it additionally means PCIe + RP1 first (its own sub-ladder).
lwIP ported as a *document-process* is the standing alternative; writing
our own subset is the fluency you asked for, and it is why this rung is
allowed to take as long as it takes.
**Pass:** `nc` from the bench arrives at the space's port on the real
board; grant and refusal receipts match the oracle's, byte-arrival for
byte-arrival.

### B9 — Second space, then n spaces *(your rungs 6–7)*
Space registry in the kernel: two spaces with different ports, keys, and
quotas; cross-arrivals refused at the arrived-at space; then spaces
manufactured from a registry document, five up, one killed, four unmoved,
all returning after reboot. Per-space access sets are now MMU/driver
facts (which EL0 tasks may touch which device registers), which is the
fourth decision enforced by hardware.

### B10 — An application *(your rung 8)*
`cve-mini` at the metal: the offline fixture arrives over B8's TCP at
`cve-raw`; a child (B7) creates overlay documents; a reader asks "what did
it say before, on whose authority?" and is answered by chain replay (B5).
HTTP, if an application ever wants it, lives here — as a process in a
space, per your ruling — never in the port.

---

## Target order — DECIDED by you, 2026-11-16: `valseir` first

As recommended below: bring-up on the Pi 4, port to `lirvol` after.
First artifacts (same day): `kernel/` — `boot.S`, `kernel.c` (framebuffer
hello), linker script, Makefile, firmware extraction, boot staging script.
B0 passes on the bench (see `docs/witness/rung-b0-b1-20261116.md`); B1's
pass waits for the screen's photograph, which only hardware can give.

## Target order (the reasoning, kept)

1. **Bring up on `valseir` (Pi 4)** — documented silicon, a screen already
   attached for B1's witness, tower cooling for long sessions, and every
   major tutorial's home ground. Nothing learned there is wasted: ARM is
   ARM, and B0–B7 port to BCM2712 as work, not as archaeology.
2. **Port to `lirvol` (Pi 5)** when the kernel stands — accepting the RP1
   sub-ladder for network/storage with eyes open, possibly splitting the
   fleet honestly: Pi 4 proves, Pi 5 carries.
3. The fleet's *tools* stay what they are: Raspberry Pi OS cards remain
   spec/rescue media; nobody is asked to throw away last night's work —
   it is how we measure the machines the kernel must run on.

## What survives from the Linux track

- `ledger/trace-format.md` and `receipt.py` — the receipt schema and its
  verifier, unchanged; B5 implements the writer in C, the schema judges it.
- `runtime/kyriosd.py` + `test_rung2.py` — demoted to oracle: the granted/
  refused/size/quota behaviour table the bare-metal port must match.
- `spaces/`, `processors/` documents — the shapes of spaces and processors,
  now to be implemented by the kernel instead of described to a daemon.
- The bench: Bookworm, cross-gcc, and tonight's discipline — receipts
  first, witnesses filed, no silent refusals. Some things don't get
  superseded.

```
RILNORVEL 🧭
Filed for your ruling: the destination never changed — space, port, ledger, document, child, many, application — but bare metal makes us earn the ground each one stands on.
 RILNORVEL?
```

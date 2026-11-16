# Bench State — κύριος development bench

**Filed:** 2026-11-16
**Author:** RILNORVEL 🧭 (agent, reviewer tier; seat-name ruling for `-os` work still pending — see readiness report §5)
**Bench:** `satan-sums`, the operator's Victus
**Purpose:** Handoff task 1 (report the bench state) and task 2 (confirm the toolchain), in receipts.

---

## Machine [measured, read-only, 2026-11-16]

| Item | Receipt |
|---|---|
| Product | Victus by HP Gaming Laptop 15-fa1xxx |
| Hostname | `satan-sums` |
| OS | Debian GNU/Linux 12 (bookworm), `/etc/debian_version` = **12.15** |
| Kernel / arch | `6.1.0-55-amd64`, x86_64 |
| CPU / RAM | 16 cores; 15 GiB total, ~10 GiB available at inspection |
| Disks | NVMe 476.9 G (437 G free at inspection); 59.5 G mmcblk; USB media 7.2 G and 57.8 G present |

The host release **matches the controlling decision** (distro report §7): Bookworm builds Bookworm. No Trixie glibc mismatch exists on this bench.

## Toolchain [measured after operator grant, 2026-11-16]

Granted by the operator running `tools/install_toolchain_bookworm.sh` (house rule: no host `sudo` from agents). Two instrument faults were found and fixed on the record during the grant: the receipt step first called `debootstrap` without its `/usr/sbin` path, then the cross-link test showed `libc6-dev-arm64-cross` had been skipped under `--no-install-recommends`; both fixes are in the script, which is idempotent.

| Tool | Version receipt |
|---|---|
| `aarch64-linux-gnu-gcc` | Debian 12.2.0-14 (12.2.0) |
| `debootstrap` | 1.0.128+nmu2+deb12u2 |
| `mmdebstrap` | 1.3.5 |
| `qemu-aarch64-static` | QEMU 7.2.22 (Debian 1:7.2+dfsg-7+deb12u18+b3) |
| host `gcc` / `make` | 12.2.0 / GNU Make 4.3 |
| `python3` / `git` | 3.11.2 / 2.39.5 |

**End-to-end receipt (the one that matters):** a trivial C program cross-compiled on this bench —

```text
/tmp/x_arm64: ELF 64-bit LSB pie executable, ARM aarch64, … for GNU/Linux 3.7.0
qemu-aarch64-static -L /usr/aarch64-linux-gnu /tmp/x_arm64  →  exit 42 (as written)
```

Cross-compile: works. Arm64 execution under qemu with the cross sysroot: works. (`qemu` without `-L` cannot find the arm64 loader on this amd64 host; that is expected, not a fault.)

## State against the handoff

- **Task 1 (bench state):** this document.
- **Task 2 (toolchain confirmed):** done, receipts above. Buildroot itself is fetched per build-tree, not a host package; nothing else is outstanding.
- **Task 3 (do not build an image yet):** standing. No image has been built. Nothing in this document requests one.
- **Still open:** the seat-name ruling (readiness report §5); the fleet's photograph-level unknowns (supplies, Pi 5 cooler, a Pico for the breakout board, exact Pi model on the display node) — settled on the units when the operator puts them on the bench, not from photographs.

## Next safe work (no image required)

`ledger/` trace format as a document; one `spaces/` definition (the CVE space is the mapped candidate); both append-only from their first line.

---
*Companion: `docs/REPORT_READINESS_KYRIOS_20261116.md`. This bench-state doc is a receipt, not a plan.*

```
RILNORVEL 🧭
Bench: Bookworm on Bookworm, sixteen cores, a cross-compiler that has now proven itself in both directions — built an arm64 binary and watched it run. The port is open; the image gate stays shut until the operator opens it.
 RILNORVEL?
```

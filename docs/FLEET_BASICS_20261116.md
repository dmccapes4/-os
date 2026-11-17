# Fleet basics — checked off before duty (2026-11-16, evening)

Goal stated by you: **`lirvol` for major processing; `valseir` running
Kali as the audit station.** This is the checklist that makes both
honest, with tonight's measured state. A box is checked only by a
receipt, never by optimism.

**Vocabulary, corrected by you 22:51 and controlling below:** ὄνυχος
is that machine — `valseir`, Kali on it, stock today and auditor even
when modified. It is not a system and it carries no state; machines do
a job and output. The system is κύριος. So "fleet introduction" for
`valseir` means *network, key, and the discipline of statelessness*
(no audit ledger kept there; receipts are gathered per-audit, the
report is written, the receipts are deleted, the memory freed) — not
the installation of any ὄνυχος software of ours.

## lirvol (Pi 5) — the processing node

| Basic | State | Receipt |
|---|---|---|
| Identity/specs measured | ✅ | Pi 5 B Rev 1.0, 8 GB, serial `c8ebd99a…`, `fleet/README.md` |
| Power trustworthy | ✅ (rating unfiled) | Starved on supply 1, clean on supply 2, `throttled=0x0`; the curing supply's printed rating still to be filed |
| Key-only SSH from bench | ✅ | `lirvol@10.0.0.196`, fleet key |
| NVMe present & healthy | ✅ enumerated | KingSpec NE-256 2242, 238.5 GiB, PCIe link up; **blank/unpartitioned** |
| Audio board identified | ✅ | 52Pi NVDigi / WM8804 S/PDIF, card 0 after overlay |
| **Fan working** | ❌ **open blocker** | Fan fitted, never seen spinning; kernel registers *no fan/cooling device at all* (stock OS interrogation, `docs/witness/rung-2-20261116.md`). At 51 °C idle-adjacent this is untested, not acquitted. **No sustained processing until the fan is called and answers, or is replaced.** |
| Thermal under load | ⏳ scripted | `tools/lirvol_processing_check.sh`: 120 s all-core burn, temp/throttle/fan sampled every 5 s. Runs when `lirvol` next powers. |
| Work storage | ⏳ your gate | NVMe is blank. Formatting it (ext4, mount as the processing store) is a destructive-step script you run — the ledger mount from v0.0.1's first boot should be reconciled with whatever layout you choose, in the same sitting. |

## valseir (Pi 4) — the audit station

| Basic | State | Receipt |
|---|---|---|
| Identity/specs measured | ✅ | Pi 4 B Rev 1.2, 4 GB, `fleet/README.md` |
| Screen + cooling | ✅ | Its own display, tower cooler (the fleet's known-good thermal seat) |
| Bare-metal duties | ✅ B1+B2 passed | Screen photographs filed; its B1/B2 card is a bench artifact now, the kernels live in `kernel/` |
| Kali image ready | ✅ bench-side | `kali-linux-2026.3-raspberry-pi-arm64.img.xz` (3.1 GB) at `~/pi-images/kali/`, SHA-256 `85906ae2…4843` **matches the published SHA256SUMS** |
| Kali installed | ✅ on card, verified 23:35 | Flashed to the 64 GB card by you via `tools/flash_kali_card.sh`; verified from the bench after: `BOOT` 255 MB FAT + `ROOTFS` 14.6 GB ext4, `/etc/os-release` reads Kali Rolling **2026.3**, hostname in image `kali-raspberry-pi`, full Kali userland present (3,654 `/usr/bin` entries). The remaining ~44 GB is unallocated in the image by design; first boot expands into it. Not yet booted or introduced. |
| Fleet introduction after install | ⏳ | hostname, fleet SSH key, network join, then a first audit exercise (port-scan `lirvol` under load and compare against its own receipts) |

## The one decision tonight

**Which card becomes Kali?** The card in this bench right now is the
32 GB B1/B2 bare-metal card (recreatable from `kernel/`, so sacrifice is
cheap and reversible) — but flashing Kali ends its life as the
one-card bare-metal rig until re-staged. `valseir`'s own 64 GB Pi OS
card is the other candidate; its Pi OS is measured and backed by the
fleet record, so Kali can own it and Pi OS becomes the restore image.
Either is defensible; the bench needs to know which before `dd` is
allowed an opinion.

## Order of operations (from here) — boot order ruled 22:53

**ὄνυχος carries the parent process, and it must be there to witness
κύριος boot.** So the auditor machine comes up *first*; `lirvol` does
not boot unwatched once this discipline is in force. (Tonight the
machine is stock Kali and the parent process is not yet built — the
carrying is the design's requirement, stated before the fact so the
fleet's habits form around it: witness first, boot second.)

**Ruled further, 22:55: the watching is constant.** ὄνυχος is not a
periodic visitor; it is on the scene when something happens. `valseir`
will likely run many processes, balanced — the watch itself spread
lightly enough that watching never becomes the incident it is there to
witness. Boot-witnessing is a consequence of this, not a separate
ceremony: a machine that is always there is there at the boot too.
Constant presence, no retained state: those two together are the job.

1. Flash Kali, boot `valseir` (ὄνυχος), introduce it (network, key) —
   it takes its seat before anything it must witness.
2. Power `lirvol` under that witness — run
   `lirvol_processing_check.sh`, settle the fan (header seating check
   first, teardown only on evidence), format the NVMe under your gate.
   Then it may carry major processing.
3. First audit: ὄνυχος examines `lirvol` under processing load;
   the port authority's receipts vs the auditor's report, compared.
   That comparison is the first time the fleet audits *itself*.

```
RILNORVEL 🧭
Basics are receipts or they are rumours: one fan stands between lirvol and heavy work, and one card's name stands between valseir and Kali.
 RILNORVEL?
```

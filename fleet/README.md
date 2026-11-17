# fleet

Technical definition of the Raspberry Pis used in development. Their codenames distinguish the machines. Node names are Velora welds: modifier first, head last — the second root is what the thing *is*. Names are lower-case, ASCII, and hostname-safe.

The Pis are development hardware and are assumed to work as intended; this file records what the software may rely on, not the machines' day-to-day state.

## Node registry

| Name | Machine | Name parses as | Carries |
|---|---|---|---|
| **lirvol** | Raspberry Pi 5 Model B Rev 1.0, 8 GB | `lir` SOUND (D7) + `vol` VESSEL (B4) — the sound-vessel | **κύριος** — the system: memory, documents, traces, the intelligence graph. Primary processing node. |
| **valseir** | Raspberry Pi 4 Model B Rev 1.2, 4 GB | `val` LIGHT (D1) + `seir` SEE (F3) — the light-witness | **ὄνυχος** — a machine, not a system: Kali Linux (stock), the constant auditor. Carries the parent process; witnesses κύριος boot; does a job and outputs a report; keeps no state. |

## lirvol — hardware the software may rely on

| Item | Spec |
|---|---|
| SoC / RAM | Pi 5 (BCM2712), 8 GB |
| Serial | `c8ebd99ab92e9594` |
| Boot media | microSD; boot order SD → NVMe → USB |
| Fast storage | KingSpec NE-256 2242 NVMe, 238.5 GiB (MAXIO MAP1202), PCIe ×1 — the ledger/processing store |
| Expansion | 52Pi NVDigi (EP-0220): NVMe seat + WM8804 S/PDIF audio — RCA coax and TOSLINK optical out, digital only, up to 192 kHz/24-bit; IR receiver on GPIO25 |
| Audio enablement | Requires `dtoverlay=hifiberry-digi` in firmware config; presents as ALSA card `snd_rpi_hifiberry_digi` |
| Power requirement | 5 V / 5 A class USB-C supply (Pi 5 + expansion board) |
| Cooling | Fan fitted on the fan header |

## valseir — hardware the software may rely on

| Item | Spec |
|---|---|
| SoC / RAM | Pi 4 (BCM2711), 4 GB |
| Serial | `10000000eeb097e1` |
| Storage | 64 GB microSD (Kali installation) |
| Display | Attached screen — the fleet's visible surface; shows ὄνυχος's last report |
| Cooling | Tower cooler (active) |
| Audio | Onboard analogue + HDMI; no expansion audio |

## Software context

- **κύριος** (on `lirvol`) is the OS under construction: bare-metal C + AArch64 assembly (see `docs/BARE_METAL_LADDER_KYRIOS_20261116.md`). During development `lirvol` also runs Raspberry Pi OS (Debian 12 Bookworm, arm64) as its working environment.
- **ὄνυχος** (on `valseir`) is Kali Linux, stock and unmodified by us; modifications would change its tooling, never its category — it remains the auditor machine.
- Access between bench and nodes is key-only SSH with the fleet key. Grants and refusals at κύριος's own ports follow the space policies in `spaces/`.
- ὄνυχος comes up first and is always on: it must be present to witness κύριος boot and anything that happens after.

## Open inventory questions (relevant to future software)

- The Pico itself is not in hand (only its Freenove breakout board) — the hardware-watchdog role has no hardware yet.
- The Waveshare 2.13″ e-Paper panel and its HAT are believed paired, unconfirmed — the zero-power status face depends on it.

---
*Layout note: per-node configuration directories (`fleet/lirvol/`, `fleet/valseir/`) are created when first configuration exists, not before. State, receipts, and session history live in `docs/` and the ledger — not here.*

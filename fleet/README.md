# fleet

Node names are assigned here. A name is a Velora weld: modifier first, head last — the second root is what the thing *is*.

## Node registry [named 2026-11-16 by RILNORVEL 🧭, at the operator's delegation]

| Name | Hostname | Unit (from `raspberry_pi_images/CATALOG.md`) | What it is | Role in the fleet |
|---|---|---|---|---|
| **lirvol** | `lirvol` | Device B — Pi 5 + audio/NVMe expansion (KingSpec M.2 seated, RCA + optical out, audio-performance chip) | `lir` SOUND (D7) + `vol` VESSEL (B4): the sound-vessel. A vessel — it holds the ledger on NVMe — characterised by sound: the audio board is its distinguishing sense. | **Primary node.** Ledger churn on NVMe; audio path when the work wants a voice. |
| **valseir** | `valseir` | Device C — plain Pi board, proper tower cooling, screen, powered at a setup prompt in its photograph | `val` LIGHT (D1) + `seir` SEE (F3): the light-witness. A witness characterised by light — it watches, and its screen is how the watching is shown. | **Supervisor / face.** Always-on display node; the fleet's visible surface. |

Names are lower-case, ASCII, DNS- and hostname-safe, and are not agent callsigns (those are ALL-CAPS trilaterals in `org_chart.jsonl`; fleet nodes are machines, and machines in this house get welds, not trilaterals).

## Measured specs — valseir [SSH, 2026-11-16, from the living machine]

Collected over SSH (`valseir@10.0.0.120`, key-only after first introduction) from the running system — the first fleet specs in this house that are not from photographs:

| Item | Receipt |
|---|---|
| Model | **Raspberry Pi 4 Model B Rev 1.2** |
| Serial | `10000000eeb097e1` |
| RAM | 4 GB class (3.7 GiB reported) |
| OS | Debian GNU/Linux 12 (bookworm), kernel `6.12.25+rpt-rpi-v8`, aarch64 |
| Storage | 64 GB SD at the port tonight (59.5 G device, 59 G root) |
| Power/thermal | `throttled=0x0` (clean — no under-voltage history), 34.5 °C at idle |
| Audio | Onboard bcm2835 headphones + two vc4 HDMI — **no expansion audio** (consistent with "the plain board") |
| Network | Wi-Fi `Twinkle`, 10.0.0.120 |

Notes: the OS hostname is still the stock `raspberrypi` — `valseir` is its fleet name and its login, and the hostname rename is a one-line operator command on the unit (`sudo hostnamectl set-hostname valseir`) whenever convenient. That a Pi 4 with 4 GB drew the supervisor/face seat is a good draw: cool, clean-throttled, and honest.

## Measured specs — lirvol [SSH, 2026-11-16, from the living machine]

Collected over SSH (`lirvol@10.0.0.196`, fleet key) from the running system, after the operator fed it properly (see power note):

| Item | Receipt |
|---|---|
| Model | **Raspberry Pi 5 Model B Rev 1.0** |
| Serial | `c8ebd99ab92e9594` (MAC `2c:cf:67:5e:ea:e4`) |
| RAM | 8 GB class (7.9 GiB reported) |
| OS | Debian GNU/Linux 12 (bookworm), kernel `6.12.25+rpt-rpi-2712`, aarch64 |
| Boot storage | 32 GB SD (29.7 G device), `BOOT_ORDER=0xf461` — SD first, then NVMe, then USB |
| NVMe | **KingSpec NE-256 2242, 238.5 GiB**, serial `0025064000083`, firmware `SN12684`, MAXIO MAP1202 controller, PCIe 5.0 GT/s ×1 (link capped Gen2 by default; device capable of more) — enumerated clean, **blank/unpartitioned** |
| Power/thermal | `throttled=0x0` (clean on the stronger supply), 45.0 °C at idle |
| Audio | **52Pi NVDigi (EP-0220) — WM8804 S/PDIF, working.** Identified 2026-11-16 from the catalog photograph + board match (single yellow RCA `RCA Out`, `Optical Out`, IR dome, KingSpec 2242 seat): the audio is *digital output only* — RCA coax + TOSLINK optical, no DAC, no analogue stage; the "optimisation" is a clean I2S→S/PDIF path (WM8804 transceiver, up to 192 kHz/24-bit). Enabled with `dtoverlay=hifiberry-digi` in `/boot/firmware/config.txt` (backup `config.txt.bak-20261116-nvdigi` beside it); after reboot `aplay -l` shows card 0 `snd_rpi_hifiberry_digi` / `wm8804-spdif-0`, modules `snd_soc_wm8804_i2c` loaded. There is no HAT EEPROM, which is why no software witness existed before the overlay was applied — the chip only speaks once spoken to. An IR receiver on GPIO25 is also on this board (52Pi spec), unused so far. |
| Network | Wi-Fi `Twinkle`, 10.0.0.196; hostname and user both `lirvol` |

Power note, filed as witnessed: on the first supply this Pi 5 + expansion board "blinked and struggled" and would not boot; on a stronger supply at another station it booted immediately, and `get_throttled` reads `0x0`. The fault was the supply, not the board, the card, or the image. The solid red LED on the audio/NVMe upper section is that board's power indication, corroborated by the SSD enumerating over PCIe — lit red here means *fed*, not *fault*.

SSH note, likewise: the seeded first boot set the hostname and landed the fleet key on the operator-created `lirvol` account (key-only SSH verified from the bench), but Wi-Fi was joined by the operator's hand at the console (`nmcli`, country US) — the seed's own `preconfigured` connection never carried it onto the network. The bench still has no fleet password for this node and needs none.

## Specs: unknown until flashed [gate] (lirvol)

Photographs gave form, not specs (catalog: "not pictured / unknown"). Real specs come from the units, not from pictures:

1. Flash a Bookworm arm64 Lite image to SD (Raspberry Pi OS Lite, Bookworm, 64-bit is the fastest honest route to hardware truth; it is a *spec-collection* image, not a κύριος image — the image gate in `HANDOFF.md` stands).
2. Boot `lirvol`, run the catalog's first-boot checks (`/proc/device-tree/model`, `free -h`, `lsblk`, `vcgencmd get_throttled`, plus audio-chip identification on the expansion board), write the output to a file on the SD's boot partition.
3. Repeat for `valseir`. Label each output file with the node name — because the Debian terminal makes copy/paste difficult, *files carry the receipts off the Pis*, not the operator's clipboard.
4. Agent files the results here as a dated amendment. Only then do fleet documents state RAM, storage capacity, exact Pi model, and audio chip as facts.

## Still unknown / not pictured

See the catalog's "Not pictured / unknown" list. Nothing in that list is resolved by naming the nodes; names are addresses for the answers, not the answers. As of the 2026-11-16 measured specs above, both nodes' model, RAM, storage, and power character are facts; on `lirvol` the audio board is now named and sounding (52Pi NVDigi, WM8804, see measured specs); what remains unknown on the fleet at large is: the Pico (only its breakout was pictured) and the e-Paper pairing. On `lirvol`, a fan **is** fitted but was observed not spinning on 2026-11-16. First interrogation (2026-11-16 morning, full text in `-os/docs/witness/rung-2-20261116.md`): SoC crossed the stock 50 °C first threshold (49.6–51.0 °C) with the fan still, and the kernel registers **no fan hwmon device and no cooling device at all** — the OS cannot see a fan, so there is no tachometer to acquit it. Suspects in order: fan lead seating / fan power feed, then the fan unit. **Do not disassemble on this evidence alone** — first confirm under load whether any cooling device ever registers; only a connection/power fault confirmed at the header earns the teardown, which you note is a total bitch.

---
*Layout note: per-node configuration directories (`fleet/lirvol/`, `fleet/valseir/`) are created when first configuration exists, not before.*

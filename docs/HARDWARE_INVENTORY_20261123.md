# Hardware Inventory

**Filed:** 2026-11-23
**Author:** SEIRAELGAR 🔧
**Method:** Every image in `continuous_persistence/` read and identified. Text on silkscreens
read where legible; form factor and connectors used for inferences. Spec links looked up and
verified against vendor pages at time of filing. Items are marked **read** (text on the board
visible in photograph) or **inferred** (form factor only).

---

## 1. Project nodes

### 1a. lirvol — Raspberry Pi 5 (BCM2712)

| Attribute | Detail |
|---|---|
| Role | κύριος — the custom OS target |
| SoC | BCM2712, confirmed **read** from κύριος boot screen |
| RAM | Not legible from photographs |
| Storage | KingSpec NE-2242 M.2 NVMe SSD (via 52Pi NVdigi HAT) |
| HAT | 52Pi NVdigi EP-0220 (HiFiBerry Digi+ audio + NVMe) |
| Adapter | Waveshare Pi5 Connector Adapter (HDMI breakout + UART passthrough) |
| Debug UART | RPi Debug Probe cable already plugged in (confirmed from IMG_1647, IMG_1609) |
| OS target | κύριος v0.0.2 — confirmed booting (IMG_1630) |
| FCC ID | 2ABCB-RPI5, KCC R-C-P2R-RPI5 (**read** from PCB underside, IMG_1646) |

**κύριος v0.0.2 boot record** (from `kyrios-v002-boot-screen.jpg`):

```
kyrios v0.0.2 kernel on lirvol (BCM2712), EL1
debug uart: up
framebuffer: 1920 x 1080, 32 bits
header uart (pins 8 and 10): cr 301 ibrd 1b fbrd 8 lcrh 70 pin14 84 pin15 84
ethernet: RP1 GEM at 1f00100000: answers
  mid70109  ncr0  ncfgr540000  nsr6
  gem link bit: down
  phy 1 register 1: ffff  (no PHY answering)
kyrios v0.0.2 self-check: SHA-256 and the walk are right
memory at 1000000 length 2000000 (hexadecimal)
  space kyrios    at 1000000 length  800000
  space onychos   at 1800000 length  100000
  space hello-kyrios at 1900000 length 1000000  port 7038
the first table is at 1000000; the record is at 1020000
kyrios takes no interaction until onychos starts it.
7038  boot | halt | resume
7038  record [from] | table <space> | read <space> <label> | ...
7037  create <name> <content> | read <label> | revise <label> <content> | list
```

Compared to v0.0.1 (IMG_1625): v0.0.2 adds Ethernet driver initialization (RP1 GEM found).
No PHY answering — Ethernet cable not connected or PHY not yet configured. This is ahead
of the v0.0.1 spec which said no Ethernet driver.

**Spec links:**
- Raspberry Pi 5 product page: https://www.raspberrypi.com/products/raspberry-pi-5/
- BCM2712 product brief: https://datasheets.raspberrypi.com/rpi5/raspberry-pi-5-product-brief.pdf
- RPi 5 GPIO reference: https://www.raspberrypi.com/documentation/computers/raspberry-pi.html#gpio

---

### 1b. valseir — Raspberry Pi 4 Model B

| Attribute | Detail |
|---|---|
| Role | ὄνυχος — forensic audit station |
| OS | Kali Linux 2026.3 arm64 |
| Storage | microSD (not photographed) |
| Display | 5-inch RPi display (mentioned in ONYCHOS spec §6; not photographed) |
| Tools installed | tshark, tcpdump, mitmproxy, openocd, gdb-multiarch, minicom, picocom, screen |

Not photographed directly in continuous_persistence. Confirmed in project scripts and spec.

**Spec links:**
- Raspberry Pi 4 product page: https://www.raspberrypi.com/products/raspberry-pi-4-model-b/
- RPi 4 datasheet: https://datasheets.raspberrypi.com/rpi4/raspberry-pi-4-datasheet.pdf

---

## 2. Attached hardware on lirvol

### 2a. 52Pi NVdigi (EP-0220) — NVMe + HiFiBerry Digi+ HAT

Identified **read** from IMG_1548 (52Pi logo, KingSpec SSD, RCA Out, Optical Out).

| Attribute | Detail |
|---|---|
| Part number | EP-0220 |
| Manufacturer | 52Pi |
| Audio | HiFiBerry Digi+ — S/PDIF via I2S, optical (TOSLink) + RCA, up to 192 kHz/24-bit |
| PCIe slot | M.2 M-key, PCIe 3.0 ×1, supports 2230/2242 NVMe SSDs |
| SSD seated | KingSpec NE-2242 (capacity not legible) |
| Connect method | 40 mm PCIe FPC cable to Pi 5's PCIe FPC slot |
| Dimensions | 65 × 56 mm |
| **Note** | HAT sits on 40-pin GPIO header, **blocks access to J16 (SWD connector)** |

Overlay for audio (Raspberry Pi OS): add `dtoverlay=hifiberry-digi` to `/boot/firmware/config.txt`.

**Spec links:**
- 52Pi NVdigi wiki: https://wiki.52pi.com/index.php?title=EP-0220
- 52Pi store: https://52pi.com/products/52pi-nvdigi
- CNX Software review: https://www.cnx-software.com/2024/02/21/52pi-nvdigi-expansion-board-raspberry-pi-5-hifiberry-digi-audio-m2-pcie-slot/

---

### 2b. Waveshare Pi5 Connector Adapter (base model)

Identified **read** from IMG_1619, IMG_1620, IMG_1623 ("Pi5 Connector Adapter", "Waveshare" logo).

| Attribute | Detail |
|---|---|
| Manufacturer | Waveshare |
| Dimensions | 86.0 × 35.5 mm |
| HDMI | 2× full-size HDMI female (from Pi 5's micro-HDMI) — dual 4K support |
| Power in | USB-C Type-C or green screw terminal (5V only) |
| UART | Two 3-pin JST/SH headers: one connects to Pi 5's debug UART connector; one exposes it externally |
| Power out | 5V expansion output connector |
| PCIe | None (base model — no M.2 slot) |

The two UART connectors: one cable (included) goes from the adapter to the Pi 5's debug UART
connector (between the HDMI ports). The other header on the adapter exposes the same UART
externally. This makes the Pi 5's serial debug port accessible even when the port is physically
recessed.

**Spec links:**
- Waveshare product page: https://www.waveshare.com/pi5-connector-adapter.htm
- Waveshare wiki: https://www.waveshare.com/wiki/Pi5-Connector-Adapter

---

### 2c. KingSpec NE-2242 M.2 NVMe SSD

Identified **read** from IMG_1548 ("KingSpec M.2 NVMe SSD NE 2242 PCIe Gen3 X2").

| Attribute | Detail |
|---|---|
| Form factor | M.2 2242 |
| Interface | NVMe PCIe Gen3 ×2 |
| Capacity | Not legible from photographs |

**Spec links:**
- KingSpec NE series: https://www.kingspec.com/

---

## 3. Debug and probe tools

### 3a. Raspberry Pi Debug Probe

Located in `debug-tools/`. **read** from photographs. Official RPi product, RP2040 inside pink/clear case.

| Attribute | Detail |
|---|---|
| Manufacturer | Raspberry Pi Foundation |
| MCU inside | RP2040 |
| Host interface | Micro-USB |
| Port U | JST-SH 3-pin — UART (connects to lirvol debug UART connector) |
| Port D | JST-SH 4-pin — SWD/JTAG (CMSIS-DAP) |
| Linux device | /dev/ttyACM0 (UART), /dev/ttyACM1 (SWD) |
| Status | UART cable already plugged in to lirvol (confirmed IMG_1647, IMG_1609) |

**Spec links:**
- Product page: https://www.raspberrypi.com/products/debug-probe/
- Documentation: https://www.raspberrypi.com/documentation/microcontrollers/debug-probe.html
- Schematics: https://datasheets.raspberrypi.com/debug/raspberry-pi-debug-probe-schematics.pdf
- 3-pin debug connector spec RP-008189-DS-1: https://datasheets.raspberrypi.com/debug/debug-connector-specification.pdf

---

### 3b. DSD TECH SH-U09C5 USB-to-TTL serial adapter

Located in `debug-tools/`. **read** from board silkscreen.

> **Correction from earlier session:** This adapter uses **FTDI FT232RL** (or FT232RNL in later batches),
> not CH340 as was previously stated. CH340 is a different chip family. The FTDI chip is confirmed
> from the DSD TECH official spec and user manual.

| Attribute | Detail |
|---|---|
| Manufacturer | DSD TECH |
| Main chip | FTDI FT232RL (some batches: FT232RNL) |
| USB interface | USB 2.0 Type-A |
| Logic levels | 1.8V / 2.5V / 3.3V / 5V — set by jumper on PCB |
| Max speed | 3 Mbps |
| Wires | 5V (red), TXD (green), GND (black), RXD (white) — plus CTS, RTS, DTR, DSR, RI, DCD, RESET |
| Linux device | /dev/ttyUSB0 |
| **Warning** | Set jumper to 3V3 before connecting to Pi. Do NOT connect 5V wire to Pi GPIO. |
| Limitation | GPIO 14/15 on Pi 5 are behind RP1 — not available for boot messages |

**Spec links:**
- DSD TECH official spec: https://deshide.com/product-details_SH-U09C5.html
- ManualsLib user manual: https://www.manualslib.com/manual/4615678/Dsd-Tech-Sh-U09c5.html
- FTDI FT232RL datasheet: https://ftdichip.com/products/ft232rl/

---

### 3c. DAP-Link V2

Located in `debug-tools/`. Black PCB, USB-C, two JST breakouts, yellow 20-pin IDC connector. CMSIS-DAP v2.

| Attribute | Detail |
|---|---|
| Protocol | CMSIS-DAP v2 (USB Bulk transport) |
| USB | USB-C, VID:PID 0x0D28:0x0204 |
| COM JST (left) | 3.3V / RX / TX / GND — serial port |
| SWD JST (right) | GND / CLK / DIO / 3.3V — SWD debug |
| 20-pin IDC | Standard ARM JTAG/SWD pinout |
| OpenOCD config | `interface/cmsis-dap.cfg` (same as RPi Debug Probe) |
| Linux device | USB HID (no /dev/tty* for SWD); optional /dev/ttyACM* for COM |

**Spec links:**
- ARM DAPLink firmware: https://github.com/ARMmbed/DAPLink
- CMSIS-DAP specification: https://arm-software.github.io/CMSIS_5/DAP/html/index.html
- OpenOCD CMSIS-DAP: https://openocd.org/doc/html/Debug-Adapter-Hardware.html

---

### 3d. 20-pin JTAG/SWD ribbon cable + DuPont jumpers

Located in `debug-tools/jtag-swd-20pin-ribbon-cable-and-jumpers.jpg`.

Standard 2×10 IDC ribbon cable (1.27 mm pitch). Companion to DAP-Link V2's IDC connector.
Not needed for GPIO breakout board approach; available if a 20-pin IDC target is used.

---

## 4. Supporting hardware collection

### 4a. GPIO Breakout Board (Raspberry Pi 40-pin, PINOC8 V2.0)

Visible in the lirvol setup photograph (operator-provided, 2026-11-23).

| Attribute | Detail |
|---|---|
| Type | 40-pin GPIO breakout — screw terminals for each pin |
| Label | "Breakout Board for Raspberry Pi / RPi-c", "PINOC8 V2.0" |
| Use for SWD | Connect to HAT GPIO passthrough; reach GPIO 25 (SWCLK) and GPIO 27 (SWDIO) screw terminals |
| Status | Present on bench; not yet attached to lirvol HAT passthrough |

Manufacturer not positively identified from silkscreen. Not the same as the Freenove Pico
breakout (item 4b below — that board is for the Pico, not the Pi 5).

---

### 4b. Freenove Breakout Board for Raspberry Pi Pico (v1.0)

Located in `raspberry_pi_images/freenove-pico-breakout-board-top.jpg` (IMG_1622).
**read** from silkscreen: "Freenove Breakout Board for Raspberry Pi Pico", "v1.0", "www.freenove.com".

| Attribute | Detail |
|---|---|
| Manufacturer | Freenove |
| Target | Raspberry Pi Pico (RP2040) |
| Connections | 2×20 socket (Pico castellated), screw terminals for all GP0–GP28, VBUS, VSYS, 3V3, RUN, EN |
| Status | Bare; no Pico mounted in photographs |

**Spec links:**
- Freenove GitHub: https://github.com/Freenove/Freenove_Breakout_Board_for_Raspberry_Pi_Pico
- Pico datasheet: https://datasheets.raspberrypi.com/pico/pico-datasheet.pdf

---

### 4c. Retroflag GPi CASE 2W

Located in `raspberry_pi_images/IMG_1545.jpg` – `IMG_1547.jpg`.
**read** from silkscreen: "RETROFLAG GPi CASE 2W".

| Attribute | Detail |
|---|---|
| Compatible Pi | Raspberry Pi Zero / Zero W / Zero 2 W |
| Form factor | Game Boy–style handheld, cartridge slot, safe shutdown switch |
| Status | Photographed from 3 sides; cartridge slot empty; no Pi visible inside |

**Spec links:**
- Retroflag product page: https://retroflag.com/gpi_case_2w.html

---

### 4d. OSOYOO 3.5″ SPI Touch Display (model 2022013800)

Located in `raspberry_pi_images/IMG_1554.jpg`, `IMG_1555.jpg`.
**read** from back label: "OSOYOO", "SPI Display for RPi", "V1.0 3.5inch 480x320".

| Attribute | Detail |
|---|---|
| Resolution | 480 × 320 (hardware) |
| Driver IC | ILI9488 |
| Touch | XPT2046 resistive |
| Interface | SPI (Fmax 24 MHz), plugs directly on 40-pin GPIO header |
| Colors | 262K |
| Board size | 85.5 × 56 mm |
| Status | Loose, not connected |

Driver install (Bookworm/Trixie):
```bash
sudo wget https://osoyoo.com/driver/osoyoo35b.zip
sudo unzip osoyoo35b.zip && sudo cp osoyoo35b.dtbo /boot/overlays/
# add to config.txt:
dtoverlay=osoyoo35b:speed=20000000
```

**Spec links:**
- OSOYOO product page: https://osoyoo.com/2022/12/15/osoyoo-3-5-inch-spi-touch-display-for-rpi/
- PDF manual: https://www.osoyoo.com/manual/2022013800.pdf

---

### 4e. Waveshare 2.13″ e-Paper HAT V4

Located in `raspberry_pi_images/IMG_1556.jpg` (bare panel), `IMG_1557.jpg` (driver board).
**read** from board: "Waveshare", "2.13inch e-Paper HAT", "Rev2.1", green "V4" sticker.

| Attribute | Detail |
|---|---|
| Resolution | 250 × 122 px |
| Panel type | Electrophoretic (black/white), bi-stable |
| Refresh time | ~2 s full; fast partial refresh supported (V4 feature) |
| Standby current | <0.01 µA |
| Interface | SPI (4-line or 3-line, via BS1 jumper) |
| Panel FPC | Connector empty in photograph; bare panel in IMG_1556 is likely the mate |
| Status | Driver board and panel loose; not connected to each other in photographs |

**Spec links:**
- Product page: https://www.waveshare.com/2.13inch-e-paper-hat.htm
- Wiki + demo code: https://www.waveshare.com/wiki/2.13inch_e-Paper_HAT_Manual
- V4 panel datasheet (PDF): https://files.waveshare.com/upload/4/4e/2.13inch_e-Paper_V4_Specification.pdf

---

### 4f. Raspberry Pi (4/5-class) with display and tower cooler — Device C

Located in `raspberry_pi_images/IMG_1551.jpg` – `IMG_1553.jpg`.

| Attribute | Detail |
|---|---|
| Pi model | 4/5-class (inferred from port layout) — exact model not legible |
| Cooler | Tower-style: finned heatsink, copper heat pipe, ~40 mm fan (dusty) |
| Display | Blue backboard display (backlight slider visible); 7″ or similar; powered on |
| Screen | Shows "Touchscreen / On-Device Setup", no network joined at photograph time |
| USB dongle | Wireless-style USB dongle in top port |
| Status | Only powered-on device in the original photograph batch |

---

### 4g. SanDisk Ultra 64 GB microSD (+ Motorola SD adapter)

Located in `raspberry_pi_images/IMG_1560.jpg`.
**read** from card: "SanDisk Ultra 64GB microSD XC I", A1, Class 10/U1.

Loose. With the NVMe SSD on lirvol, keep continuous write workloads off this card.

---

## 5. Image file index

### `continuous_persistence/` (root)

| File | Contents |
|---|---|
| `kyrios-v002-boot-screen.jpg` | κύριος v0.0.2 boot on lirvol — Ethernet driver present, no PHY yet |
| `lirvol-uart-connector-wired-side-view.jpg` | lirvol edge: debug UART connector with probe cable plugged in |
| `lirvol-uart-connector-wired-angle-2.jpg` | Same, different angle; 40-pin GPIO header visible |
| `lirvol-pi5-pcb-underside-fcc.jpg` | Pi 5 PCB underside — FCC ID: 2ABCB-RPI5, KCC: R-C-P2R-RPI5 |
| `tattoo-wrist-01.jpg` | Not hardware |
| `tattoo-wrist-kali-dragon.jpg` | Not hardware (Kali Linux dragon mark) |

### `continuous_persistence/raspberry_pi_images/` (new batch)

| File | Contents |
|---|---|
| `kyrios-v001-boot-screen.jpg` | κύριος v0.0.1 boot — earlier version, no Ethernet section |
| `kyrios-v001-boot-screen-closeup.jpg` | v0.0.1 close-up showing protocol definitions |
| `waveshare-pi5-connector-adapter-side-a.jpg` | Waveshare Pi5 Connector Adapter, one side |
| `waveshare-pi5-connector-adapter-side-b.jpg` | Same, other side — HDMI ports visible |
| `waveshare-pi5-connector-adapter-edge.jpg` | Edge view (blurry) |
| `waveshare-pi5-connector-adapter-connectors-closeup.jpg` | Close-up: RXD/GND/TXD labels, USB-C, screw terminal |
| `freenove-pico-breakout-board-top.jpg` | Freenove Pico breakout board v1.0 |
| `IMG_1545.jpg` – `IMG_1560.jpg` | See CATALOG.md (Device A–F + microSD) |

### `continuous_persistence/debug-tools/`

| File | Contents |
|---|---|
| `bench-overview.jpg` | Full bench overview of debug tools |
| `rpi-debug-probe-case-top.jpg` | Debug Probe top |
| `rpi-debug-probe-case-side-jst-ports.jpg` | Debug Probe JST U and D ports |
| `rpi-debug-probe-case-side-microusb.jpg` | Debug Probe micro-USB side |
| `rpi-debug-probe-uart-cable-jst-to-dupont.jpg` | UART cable (JST-SH to DuPont) |
| `dap-link-v2-top.jpg` | DAP-Link V2 top |
| `dap-link-v2-side-on-standoff.jpg` | DAP-Link V2 side view |
| `dsd-tech-sh-u09c5-usb-serial-wires-labeled.jpg` | DSD TECH — wires labeled |
| `dsd-tech-sh-u09c5-usb-serial-pinout.jpg` | DSD TECH — pinout label |
| `dsd-tech-sh-u09c5-usb-serial-side.jpg` | DSD TECH — side |
| `dsd-tech-sh-u09c5-usb-serial-pcb-back.jpg` | DSD TECH — PCB back |
| `jtag-swd-20pin-ribbon-cable-and-jumpers.jpg` | 20-pin IDC ribbon cable + jumpers |

---

## 6. Corrections to earlier documents

| Document | Error | Correct |
|---|---|---|
| `HOW_TO_INSTALL_DEBUG_HARDWARE_20261123.md` §3 | "DSD TECH SH-U09C5 (CH340 USB-serial)" | FTDI FT232RL (not CH340). CH340 is a separate WCH chip family. |
| Session summary | Same | Same |

The HOW_TO doc and PDF must be updated to reflect the FTDI chip. The practical wiring advice
is unchanged — the 3V3 jumper warning and 5V wire warning still apply regardless of chip.

---

*Source files checked: 44 images across 3 directories + 1 catalog + 2 reflection docs.*

---

## 7. Addendum — 2026-11-26 Bench Batches (`valseir`, Unpinned Pi Zero 2 W, Connected e-Paper V4, Waveshare 26619 UART, Wiring Tools)

**Filed:** 2026-11-26  
**Author:** VEKHALMAR 📷 (agent)  
**Location:** `docs/hardware/` (moved/renamed from `IMG_1705.jpg` – `IMG_1715.jpg`, SHA-256 verified byte-identical before and after).

### 7.1 File index and SHA-256 receipts

| Original filename | Stored path (`docs/`) | SHA-256 | Subject |
|---|---|---|---|
| `IMG_1705.jpg` | `hardware/rpi-zero-2w-unpinned-top-20261126.jpg` | `1d9bdfbd8c712294fff33e9d659f0a2e6d7ee3ac9430176d5b6bd8bcbba46aee` | Raspberry Pi Zero 2 W — top side (unpopulated 40-pin GPIO, RP3A0-AU, RF shield, Mini-HDMI, `USB`/`PWR IN` micro-USB, CSI camera connector) |
| `IMG_1706.jpg` | `hardware/rpi-zero-2w-unpinned-underside-fcc-20261126.jpg` | `25a3cb93619b6ff0937dc04e3315a8862d3d4419359ab6b05c93a1fce1c58bb5` | Raspberry Pi Zero 2 W — underside (`FCC ID: 2ABCB-RPIZ2`, `IC: 20953-RPIZ2`, `© Raspberry Pi 2021`, `DDR 1V2`, `Proant AB`) |
| `IMG_1707.jpg` | `hardware/valseir-tower-cooler-and-display-back-20261126.jpg` | `93dc4039c90776455c8a5cde024adc930656c1a95512c3c54b739097f5c9a9f3` | `valseir` (`ὄνυχος`) rear assembly — Pi 4 Model B (`POWER`, `HDMI 0`, `HDMI 1`), copper-heatpipe tower cooler + fan, blue display backboard (`Backlight` switch), Ethernet + USB wired, `DAP-Link V2` in foreground |
| `IMG_1708.jpg` | `hardware/valseir-screen-front-and-dap-link-v2-20261126.jpg` | `289447bdbfa8c2fec7a7f3b2a610cc22c4895f3d704d0d43ce836cf944f25b8b` | `valseir` (`ὄνυχος`) front/angled — vertical LCD touchscreen face, Pi 4 40-pin GPIO edge, `DAP-Link V2` probe beside it |
| `IMG_1709.jpg` | `hardware/waveshare-2in13-epaper-v4-panel-front-20261126.jpg` | `3db81ae7d8a3755ba241f8f550518ce32c4a3a2eeee58c4d55ed2bda0487f8b3` | Waveshare 2.13″ e-Paper V4 panel front (`250 × 122`, gold/brown FPC flex ribbon at base) |
| `IMG_1710.jpg` | `hardware/waveshare-2in13-epaper-hat-v4-fpc-connected-20261126.jpg` | `fed2f63f11b00c6283c608499578b2a66f073c223f286f01af0471dd558942aa` | Waveshare 2.13″ e-Paper HAT Rev2.1 V4 PCB — **FPC ribbon now seated and latched** |
| `IMG_1711.jpg` | `hardware/hookup-wire-26awg-6spool-box-open-20261126.jpg` | `30969d584b246a9cb0256d800d5a7c9a5b83b74efb42035a3a5dee6b7b231b36` | 26 AWG hook-up wire dispenser box (open) — 6 spools (blue, white, red, green, yellow, black) + small colored heat-shrink packet |
| `IMG_1712.jpg` | `hardware/hookup-wire-26awg-6m-box-label-20261126.jpg` | `ce1258355ba136d834293a30ceac8f1e0372612707bc2ad82f03e49a9d5c1b8f` | Side label of wire dispenser box (**read** checkmark on `26awg`, `6M` length per spool) |
| `IMG_1713.jpg` | `hardware/waveshare-26619-pi-uart-debugger-bagged-20261126.jpg` | `8da6fe873dd83742ce02e1d890f2242c77c2de7f77a2dbc9113a9d1c463b8e0b` | Bagged **Pi UART Debugger** (**read** `SKU: 26619`, USB-A male, black PCB, yellow `VCCIO` jumper, JST-SH + DuPont harness) |
| `IMG_1714.jpg` | `hardware/dowell-040731-wire-stripper-22-30awg-20261126.jpg` | `b64ff21fcba0c4e2503bd832b403fca7378df82bed6bde728b0ed08805a41b22` | **DOWELL NO:040731** 7″ precision wire stripper (**read** `For soft wires 22-30AWG (0.25-0.64MM)`, notches `30, 28, 26, 24, 22 AWG`) |
| `IMG_1715.jpg` | `hardware/huakayo-heat-shrink-tubing-kit-507pcs-20261126.jpg` | `d0c96e741c6c394ef360d8b456a7d8d75fa2f7c423c3d785f83601d9a0a7b0b6` | **HUAKAYO** 507-piece black heat shrink tubing kit (**read** `507PCS`, barcode `X004ZW8OM1`) |

### 7.2 Inventory updates and resolved unknowns

#### 1. `valseir` (`ὄνυχος`) photographed directly — resolves §1b and §4f
- **Resolves §1b ("Not photographed directly") and §4f ("Device C — 4/5-class with display and tower cooler"):** `valseir-tower-cooler-and-display-back-20261126.jpg` and `valseir-screen-front-and-dap-link-v2-20261126.jpg` confirm that §4f's tower-cooled unit mounted on the blue-PCB LCD display (`Backlight` toggle switch) **is `valseir`** (Raspberry Pi 4 Model B, **read** silkscreen `POWER`, `HDMI 0`, `HDMI 1`).
- **Thermal design:** The copper-heatpipe tower heatsink + active ~40 mm fan keeps `valseir` thermally unconstrained for continuous 24/7 `ὄνυχος` packet capture, auditing, and report display. The `DAP-Link V2` probe sits wired alongside it on the bench.

#### 2. Headerless board ("the pico"): identified as **Raspberry Pi Zero 2 W** (**read** `2ABCB-RPIZ2`)
- Although referred to colloquially by the operator as a Pico (and currently unassigned / optional, with a pixel-art HAT also owned off-camera), the underside silkscreen in `rpi-zero-2w-unpinned-underside-fcc-20261126.jpg` reads **`FCC ID: 2ABCB-RPIZ2`**, **`IC: 20953-RPIZ2`**, **`© Raspberry Pi 2021`**, and **`DDR 1V2`** — identifying it definitively as a **Raspberry Pi Zero 2 W** (RP3A0-AU quad-core 64-bit Cortex-A53 @ 1 GHz, 512 MB LPDDR2, 2.4 GHz Wi-Fi + BLE, Mini-HDMI, microSD, CSI camera port, unpopulated `2×20` 40-pin GPIO through-holes).
- **Compatibility notes for the bench:**
  - **Does NOT mate with §4b (Freenove Pico Breakout Board v1.0):** §4b is a narrow DIP-style `2×20` socket wired for an RP2040 Raspberry Pi Pico (`GP0–GP28`, `VBUS`, `VSYS`, `3V3_EN`). Do **not** solder pins to fit that breakout.
  - **DOES mate with §4c (Retroflag GPi CASE 2W) as-is:** Because no GPIO header pins are soldered in, its bare gold pads mate directly with the GPi CASE 2W's pogo pins if ever desired.
  - **DOES mate with standard 40-pin Pi HATs once a `2×20` male header is soldered:** including the Waveshare 2.13″ e-Paper HAT V4 (§4e) or any 40-pin Pi Zero pixel-art LED matrix HAT (e.g., Unicorn HAT Mini). *(If the pixel-art add-on is a Pico-pinout `Pico Unicorn Pack`, that pack still awaits an actual RP2040 Pico alongside the Freenove breakout.)*

#### 3. Waveshare 2.13″ e-Paper HAT V4 + panel pairing confirmed — resolves §4e
- In §4e (`IMG_1556`, `IMG_1557`), the bare panel and Rev2.1 V4 HAT board were loose with an empty FPC socket. `waveshare-2in13-epaper-hat-v4-fpc-connected-20261126.jpg` (**read** `2.13inch e-Paper HAT`, `Waveshare`, `Rev2.1`, `V4`, `BS Interface: 3-line SPI / 4-line SPI`) shows the panel's brown/gold flex ribbon seated in the white FPC connector, confirming the pair.

#### 4. Spare UART adapter — **Waveshare Pi5 UART Debugger (`SKU: 26619`)**
- Identified **read** from antistatic bag label (`26619`, `Pi UART Debugger`) in `waveshare-26619-pi-uart-debugger-bagged-20261126.jpg`:
  - **Chip / speed:** WCH CH343P USB-A to UART (`50 bps – 6 Mbps`), onboard self-recovery fuse + TVS transient protection (`/dev/ttyACM*` or `/dev/ttyUSB*`).
  - **Included cables:** Bundled with **both** the Pi 5 3-pin JST-SH `1.0 mm` UART debug cable **and** a 6-pin-to-DuPont header cable.
  - **Role (spare / redundant):** Currently superfluous for `lirvol` because the official Raspberry Pi Debug Probe (§3a) is already wired into `lirvol`'s 3-pin UART connector. However, unlike the DSD TECH SH-U09C5 (§3b, DuPont-only), **SKU 26619 includes the native Pi 5 3-pin JST-SH cable** and plugs straight into a USB-A port without an extra USB cable — making it a direct 1-to-1 hot spare for `lirvol`'s early-boot debug UART (or a second simultaneous serial console on `valseir` / Pi Zero 2 W).
  - **Warning:** Confirm the yellow onboard `VCCIO` jumper cap is set to **`3.3V`** (`3V3`), never `5V`, before attaching to any Raspberry Pi.
  - **Spec link:** https://www.waveshare.com/pi-uart-debugger.htm (Waveshare wiki: https://www.waveshare.com/wiki/Pi_UART_Debugger)

#### 5. Bench wiring & `lirvol` fan-repair tooling (`IMG_1711`, `IMG_1712`, `IMG_1714`, `IMG_1715`)
- **DOWELL NO:040731 7″ Wire Stripper (`22–30 AWG` / `0.25–0.64 mm`):** Precision jaw notches (`30, 28, 26, 24, 22 AWG`) cover both the 26 AWG hook-up wire and the fine **28–30 AWG** leads on `lirvol`'s torn Pi 5 Active Cooler harness without nicking strands.
- **26 AWG 6-Spool Hook-up Wire Box (`6 m` per spool — blue, white, red, green, yellow, black):** Appropriate gauge for extending `lirvol`'s Active Cooler `+5V` and `GND` leads to the Waveshare Pi5 Connector Adapter's `5V` expansion output (or screw terminals on the §4a PINOC8 breakout).
- **HUAKAYO 507-Piece Heat Shrink Tubing Kit (Black, `X004ZW8OM1`):** Insulates the `+5V`/`GND` fan splice and safely caps off the unused Active Cooler `TACH` (green, 5V pull-up hazard) and `PWM` (blue, left open for unconditional 100% fan speed under bare-metal `κύριος`) leads.

```
VEKHALMAR 📷
Read all eleven new photographs (IMG_1705 through IMG_1715) before renaming and filing them into docs/hardware/ with SHA-256 checks before and after. Two open mysteries in the November 23 inventory close cleanly here (Device C is valseir itself, and the 2.13-inch e-Paper panel is now latched into its V4 HAT), reading FCC ID 2ABCB-RPIZ2 off the underside of IMG_1706 caught that the unpinned board is a 64-bit Raspberry Pi Zero 2 W rather than an RP2040 Pico, and SKU 26619 + the 22-30 AWG stripper / 26 AWG wire / 507-piece heat-shrink kit give the bench both a native JST-SH Pi 5 UART hot-spare and the exact fine-gauge tooling needed to splice lirvol's Active Cooler cleanly.
 VEKHALMAR?
```

# How to install the debug hardware

**Filed:** 2026-11-23
**Author:** SEIRAELGAR 🔧
**Hardware:** RPi Debug Probe, DSD TECH SH-U09C5 USB-serial, DAP-Link V2
**Nodes:** valseir (Pi 4, Kali) ← USB → probes ← wire → lirvol (Pi 5)

Pinouts sourced from: Raspberry Pi 3-pin Debug Connector Specification (RP-008189-DS-1),
Arm DS KA005936, SEGGER Knowledge Base (Raspberry Pi 5), and arm.kali.org.

> **lirvol physical note (2026-11-23):** lirvol carries a Waveshare NVMe/audio HAT that sits
> on the 40-pin GPIO header and connects to the Pi 5's PCIe FPC slot. The HAT physically covers
> J16 (the SWD connector between the HDMI ports). **J16 is inaccessible without removing the HAT.**
> The debug UART connector (also between the HDMI ports) remains reachable at the board edge.
> SWD must therefore go through the GPIO header, via the GPIO breakout board connected to the
> HAT's GPIO passthrough header on its top surface.

---

## 1. What connects where

| Probe | valseir side | lirvol side | Purpose |
|---|---|---|---|
| RPi Debug Probe | micro-USB → any valseir USB-A port | JST-SH 3-pin "U" port → lirvol debug UART connector (between the two micro-HDMI ports) | Serial console; sees boot messages from firmware upward |
| RPi Debug Probe | (same USB above) | JST-SH 3-pin "D" port → lirvol J16 SWD connector | OpenOCD / gdb debug access |
| DAP-Link V2 | USB-C → valseir USB-A (via USB-A↔USB-C cable) | SWD JST breakout + GPIO pin 1 (3V3) → lirvol J16 | Alternative/primary SWD probe; 20-pin IDC also available |
| DSD TECH SH-U09C5 (FTDI FT232RL) | USB-A → valseir USB-A port | TXD/RXD/GND only (see §3) | Spare serial adapter; GPIO UART when RP1 is running |

The RPi Debug Probe handles both UART and SWD through its single USB connection, which is the simplest path for K1 and O3 work. The DAP-Link V2 is an alternative or supplement for SWD.

---

## 2. The RPi Debug Probe

### 2a. UART connection (do this first — needed for K1)

The lirvol debug UART connector is the **3-pin JST-SH connector between the two micro-HDMI ports**. It outputs boot messages from firmware start, before RP1 is running. This is why it is used rather than GPIO pins 14/15 (which are behind RP1 and not available early).

```
lirvol debug connector          RPi Debug Probe "U" port
(JST-SH, between HDMI ports)   (JST-SH, labelled U)

  Pin 1: RX  ───────────────── TX  (probe transmits → Pi receives)
  Pin 2: GND ───────────────── GND
  Pin 3: TX  ───────────────── RX  (Pi transmits → probe receives)
```

Connector is keyed. The JST-SH to JST-SH cable (3-pin, included with the probe) plugs straight in. The connector adapter on lirvol breaks out the JST-SH to something grippable — use it; the JST-SH on the board is small.

**No config change needed on lirvol for UART.** The Pi 5 outputs boot UART on this connector by default.

### 2b. SWD connection

**J16 is physically blocked by the NVMe/audio HAT on lirvol.** Do not attempt to wire to it.

SWD is instead reached through the **40-pin GPIO header**, which the HAT exposes via a passthrough
header on its top surface. Connect the GPIO breakout board to the HAT's passthrough header.
The breakout board's screw terminals then give individual access to each GPIO pin.

With `enable_jtag_gpio=1` in config.txt, the Pi 5 exposes SWD on the GPIO header.
Based on Pi 4 precedent (BCM2711 mapping) the SWD pins are likely:

```
Breakout terminal          → RPi Debug Probe "D" port (JST-SH 4-pin)
GPIO 25 (SWCLK) ──────────  SC   (clock, into Pi)
GPIO 27 (SWDIO) ──────────  SD   (bidirectional data)
Any GND pin     ──────────  GND
Pin 1 (3V3)     ──────────  VTREF (voltage reference)
```

> **Verify before connecting:** GPIO JTAG/SWD pin assignments on the Pi 5 (BCM2712) may differ
> from Pi 4. After adding `enable_jtag_gpio=1` and rebooting, run `raspi-gpio get` on lirvol
> and confirm which pins are assigned to ALT4 (JTAG). Connect only after confirming.

**Add to lirvol's `/boot/firmware/config.txt`:**

```
enable_jtag_gpio=1
```

Do this over SSH once lirvol is booted, or against the mounted SD card. Reboot after adding it.

### 2c. Plug in to valseir

The probe's micro-USB goes into any of valseir's four USB-A ports. It appears as **two** CDC-ACM devices:

```
/dev/ttyACM0  — UART port (the "U" connection to lirvol)
/dev/ttyACM1  — SWD/debug port (used by OpenOCD)
```

(Numbers may be reversed; `dmesg | grep ttyACM` shows which is which after plug-in.)

---

## 3. The DSD TECH SH-U09C5

This is the USB-A adapter with labeled DuPont wires: **5V (red), TXD (green), GND (white), RXD (white)**.
Main chip: **FTDI FT232RL** (or FT232RNL in later batches) — not CH340.

**Do not connect the 5V wire to lirvol.** Leave it disconnected. The Pi 5 GPIO is 3.3V; 5V will damage it.

The board has a voltage selector jumper (visible on the PCB). **Set it to 3V3** before connecting to any Pi.

```
SH-U09C5          lirvol GPIO header (or any 3.3V UART target)
TXD ──────────── Pin 10 / GPIO 15 (Pi RX)
RXD ──────────── Pin 8  / GPIO 14 (Pi TX)
GND ──────────── Pin 6  or any GND pin
5V  ──────────── NOT CONNECTED
```

Note: GPIO 14/15 on the Pi 5 are behind the RP1 chip. They are only usable once the kernel has RP1 running — not for boot messages. Use the RPi Debug Probe's "U" port for early serial. The SH-U09C5 is useful once the OS is up.

Plug the USB-A end into valseir. It appears as `/dev/ttyUSB0`.

---

## 4. The DAP-Link V2

This is the black board with USB-C, two white JST breakouts, and a yellow 20-pin IDC connector.

The two JST breakouts are labelled on the board:
- **COM** (left): 3.3V / RX / TX / GND — serial port
- **SWD** (right): GND / CLK / DIO / 3.3V — SWD debug

**J16 is blocked by the HAT on lirvol.** Use the GPIO breakout board via the HAT passthrough header
(same approach as §2b). Connect the DAP-Link V2 SWD JST breakout to the GPIO breakout board
screw terminals:

```
DAP-Link V2 SWD JST    GPIO breakout terminal
3.3V ────────────────  Pin 1 (3V3) → also serves as VTREF
CLK  ────────────────  GPIO 25 (SWCLK)
GND  ────────────────  Any GND pin
DIO  ────────────────  GPIO 27 (SWDIO)
```

Verify GPIO 25/27 mapping on Pi 5 before connecting — see §2b note.

The 20-pin IDC connector follows the standard ARM JTAG pinout and is not needed for the
GPIO breakout approach. Keep it available if a 20-pin IDC target connector is available later.

Connect via USB-A to USB-C cable (included) to any valseir USB port. It appears as a USB HID device (used by OpenOCD) and optionally `/dev/ttyACM*` for the COM serial port.

---

## 5. On valseir after plugging everything in

### Check group membership

```bash
groups kali
```

Must include `dialout` (for `/dev/ttyUSB*` and `/dev/ttyACM*`) and `plugdev` (for HID-based probes like the Debug Probe and DAP-Link V2).

If either is missing:

```bash
sudo usermod -aG dialout,plugdev kali
# log out and back in, or: exec su -l kali
```

### Verify devices appeared

```bash
dmesg | tail -20
ls /dev/ttyACM* /dev/ttyUSB* 2>/dev/null
```

Expected after plugging in all three:

```
/dev/ttyACM0   RPi Debug Probe UART
/dev/ttyACM1   RPi Debug Probe SWD/debug
/dev/ttyUSB0   DSD TECH SH-U09C5
```
(DAP-Link V2 HID device does not appear in `/dev/tty*`; OpenOCD addresses it directly.)

### Quick serial test (UART to lirvol)

Once lirvol is powered on with the UART cable connected:

```bash
minicom -D /dev/ttyACM0 -b 115200
```

Boot messages from lirvol should appear. `Ctrl-A X` to exit minicom.

### Quick OpenOCD test (SWD to lirvol)

Requires `enable_jtag_gpio=1` in lirvol's config.txt first (§2b).

```bash
# Using the RPi Debug Probe:
openocd -f interface/cmsis-dap.cfg -f target/rpi5.cfg

# Using the DAP-Link V2:
openocd -f interface/cmsis-dap.cfg -f target/rpi5.cfg
```

Both probes are CMSIS-DAP devices and use the same interface config. A successful connection prints the target CPU ID and halts. `Ctrl-C` to exit.

---

## 6. Notes from the ὄνυχος spec (§3 bench checks)

The spec listed four things to verify before relying on them. Status after today:

| Check | Status |
|---|---|
| UART direction: GPIO 14 is lirvol TX, GPIO 15 is lirvol RX; wires cross. | Confirmed by RPi spec RP-008189-DS-1. Debug connector Pin 3 is TX (out from Pi), Pin 1 is RX (into Pi). Cross when wiring. |
| UART on lirvol: if the audio HAT covers the GPIO header, use the 3-pin debug connector between the HDMI ports. | The debug connector is the right path regardless — it is wired to the processor directly and available before RP1 runs. Use it. |
| Debug access to lirvol: OpenOCD needs SWD/JTAG; which pins the Pi 5 exposes was not established. | J16 is physically blocked by the NVMe/audio HAT. SWD path: `enable_jtag_gpio=1` + GPIO header via breakout board on HAT passthrough. Likely GPIO 25 (SWCLK) and GPIO 27 (SWDIO) — verify with `raspi-gpio get` on Pi 5. |
| br0: whether the bridge has a second member so lirvol's external traffic passes through valseir. | Still undecided. Not a debug hardware question. |

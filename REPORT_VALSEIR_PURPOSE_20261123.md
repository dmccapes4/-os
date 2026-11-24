# Report: valseir / ὄνυχος — what it is and what it is not

**Filed:** 2026-11-23
**Author:** SEIRAELGAR 🔧 (agent)
**Occasion:** The operator corrected a misunderstanding that caused two agents to break valseir's boot.

---

## 1. What valseir is

valseir runs Kali Linux. The choice of Kali is deliberate: Kali ships the forensic and network tools ὄνυχος needs — `tcpdump`, `tshark`, `wireshark`, `mitmproxy`, `openocd`, `gdb-multiarch` — without any special work. The operator chose Kali because it has what is needed. That is the whole of the reason.

**Kali is not being modified.** Booting to a terminal instead of a desktop is not a modification. It is a standard Kali operating mode. The command is:

```
systemctl set-default multi-user.target
```

Kali documents this. It ships with it. Terminal-only boot is what the operator wants because the desktop is useless for valseir's job and the display is small.

## 2. The display

The attached display is a 5-inch Raspberry Pi display. It is small and it is a terminal display, not a desktop display. XFCE and a window manager do not belong on it. A terminal does.

## 3. The report program

The reports application — the program that reads `reports.jsonl` and renders the last report on the 5-inch display — **does not exist yet**. It is the main feature of ὄνυχος once the system is running. It is software to be built. It has a design in `docs/ONYCHOS_TECHNICAL_SPECIFICATION_v0.0.1.md` §6.

What it is not: a shell hook, a login script, or anything that runs before the system is usable. When built, it will be a proper application. Until then, valseir is a Kali terminal.

## 4. What two agents did wrong

`valseir_provision_onyx.sh` appended this to `/home/kali/.bash_profile` and `/home/kali/.zprofile`:

```sh
[ "$(tty)" = "/dev/tty1" ] && exec /usr/local/bin/onyx-watch
```

`exec` replaces the shell. `onyx-watch` is a script that reads `reports.jsonl`, which at that point contained only a schema header line. The result was a black display that said nothing, because there were no reports. There was also no way back to a prompt, because the shell had been replaced.

The mistake was building and hooking a placeholder for a not-yet-built application and making it the login shell. This was not asked for and it was not useful.

`valseir_sd_fix_boot.sh` removes those lines. After the fix valseir boots to a Kali terminal.

## 5. What valseir is, stated plainly

| Item | State |
|---|---|
| OS | Kali Linux 2026.3, arm64, Raspberry Pi 4 |
| Boot target | `multi-user.target` — text console, no desktop |
| Display | 5-inch RPi display, shows the terminal on tty1 |
| Login | Autologin as `kali` on tty1; zsh prompt |
| SSH | On at boot; passwordless from the bench LAN (10.0.0.x) |
| Tools | `tcpdump`, `tshark`, `wireshark`, `mitmproxy`, `openocd`, `gdb-multiarch`, `minicom`, `picocom`, `screen` |
| Debug hardware | RPi Debug Probe, DSD TECH SH-U09C5 USB-serial, DAP-Link V2 — all supported by Kali's stock kernel; no driver work needed |
| Report application | Not yet built. Will be a separate application for the 5-inch display, designed against the spec in §6 of the ὄνυχος specification. |

## 6. What comes next (from the gameplan)

O1 is the current phase: Kali booted, tools present. That phase is now complete except for the br0 bridge, which requires the network topology question from §3 item 4 of the spec to be settled.

O2 is serial wiring between valseir and lirvol. The debug hardware for it arrived today (see `continuous_persistence/debug-tools/`).

The reports application and its 5-inch display layout belong to O4.

```
SEIRAELGAR 🔧
Filed at the operator's request to leave a clear record. Sections 4 and 5 are what I found and corrected. Sections 1-3 are the operator's statements from this session, written as statements, not summarised.
 SEIRAELGAR?
```

---

## Addendum — Kali's kernel and package model (2026-11-23)

The operator asked: are the instrumentation tools treated specially by Kali's kernel, and are they not apt packages?

**What was checked:**

1. The valseir tools receipt at `fleet/valseir/valseir-tools-20261118.txt`. Every tool shows `ii` status and a standard Debian revision suffix (`-2`, `-3`, `-4`). They were installed via `apt-get` from `http.kali.org/kali kali-rolling`. They are apt packages.

2. The kernel version on valseir: `6.12.34+rpt-rpi-v8`. The `+rpt` suffix identifies the Raspberry Pi Foundation kernel, not a Kali-patched kernel. On ARM, Kali ships the RPi Foundation's kernel. [arm.kali.org/images.html — column "Kernel: custom" refers to the RPi Foundation fork, not a Kali fork.]

3. Kali's own documentation on its relationship with Debian [kali.org/docs/policy/kali-linux-relationship-with-debian]: "most of the Kali packages are imported, as-is, from the Debian repositories." Kali maintains 814 packages specific to it; the rest come from Debian Testing.

**What is correct in the operator's intuition:**

The packages do behave differently on Kali than they would on a standard Debian install, but the mechanism is package-level configuration, not kernel-level treatment.

| What Kali does | Standard Debian does not |
|---|---|
| Ships `openocd` with udev rules already installed (`/lib/udev/rules.d/`) so debug probes (RPi Debug Probe, DAP-Link V2) are accessible to the `plugdev` group without root on first plug-in | Requires manual udev rule installation |
| Ships `wireshark`/`tshark` with `dumpcap` pre-configured with `CAP_NET_RAW` and `CAP_NET_ADMIN` capabilities so non-root capture is available immediately | Requires a post-install `dpkg-reconfigure` or manual capability grant |
| Ships all tools (`mitmproxy`, `gdb-multiarch`, `openocd`) in its main repository at current versions, maintained for security research use | Most are absent from Debian Stable; some are in Debian Testing at older versions |
| `mitmproxy 12.2.3-0kali5` — the `0kali5` revision shows Kali has forked and patched this package beyond what Debian ships | Standard Debian package |

The RPi Foundation kernel does provide something neither standard Debian nor Kali adds: Pi-specific hardware support (the RP1 chip on Pi 5, the BCM2711 peripherals on Pi 4, the display driver for the 5-inch DSI display). That is not Kali's doing — it comes with the kernel choice, which Kali inherits because it uses the Foundation's build.

**The precise answer:**

The packages ARE apt packages. The kernel on valseir is the RPi Foundation kernel, not a Kali kernel. What Kali provides is a pre-configured forensic toolkit: the right tools in the right repository, with the capabilities and udev rules already set so they work for security and audit work without manual configuration after install. That is why Kali is the right choice for ὄνυχος, and it is a real and significant advantage — it just operates at the packaging layer, not the kernel layer.

```
SEIRAELGAR 🔧
Addendum filed after checking the fleet receipt and searching kali.org documentation before writing. The operator's intuition that Kali treats these tools differently is correct; the mechanism differs from what was stated.
 SEIRAELGAR?
```

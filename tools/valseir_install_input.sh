#!/bin/sh
# valseir_install_input.sh: keyboard and mouse support for valseir
# (Kali, arm64): Bluetooth tools, USB and input inspection tools.
#
# Run ON valseir, as root:   sudo sh valseir_install_input.sh
# Running it again is harmless. A receipt is printed and saved in the
# directory the script is run from.
#
# Note: valseir's onboard Bluetooth is off by design (dtoverlay=
# disable-bt put the PL011 UART on pins 8/10 for lirvol). So:
#   - a Logitech USB receiver (Bolt or Unifying) works as plain USB HID;
#   - Bluetooth pairing needs a USB Bluetooth adapter plugged in.
set -eu

TOOLS="bluez bluez-tools usbutils evtest libinput-tools"
OUT="valseir-input-$(date +%Y%m%d).txt"

[ "$(id -u)" = 0 ] || { echo "run as root: sudo sh $0" >&2; exit 2; }
[ "$(dpkg --print-architecture)" = arm64 ] || { echo "not arm64: this is for valseir" >&2; exit 1; }

export DEBIAN_FRONTEND=noninteractive
apt-get update
# shellcheck disable=SC2086
apt-get install -y $TOOLS

systemctl enable --now bluetooth.service || true

{
    echo "node: $(hostname)"
    date -u +%Y-%m-%dT%H:%M:%S+00:00
    echo "== package versions =="
    # shellcheck disable=SC2086
    dpkg-query -W -f='${Package} ${Version} ${db:Status-Abbrev}\n' $TOOLS
    echo "== bluetooth service =="
    systemctl is-enabled bluetooth.service || true
    systemctl is-active bluetooth.service || true
    bluetoothctl --version || true
    echo "== bluetooth adapters (none expected: onboard BT is off) =="
    hciconfig -a 2>/dev/null || echo "no adapter"
    echo "== usb devices =="
    lsusb || true
    echo "== input devices =="
    cat /proc/bus/input/devices | grep -E '^N: Name=' || true
    ls -l /dev/input/ || true
} | tee "$OUT"

echo "receipt: $(pwd)/$OUT" >&2

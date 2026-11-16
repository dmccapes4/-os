#!/bin/sh
# Run ON a Pi (as root or with sudo) after first boot of a spec-collection image.
# Writes specs-<hostname>.txt to the boot partition so the card itself carries
# the receipt back to the bench — no terminal copy/paste required.
# Read-only collection: this script changes nothing except its own output file.
set -u

OUT_NAME="specs-$(hostname).txt"
for d in /boot/firmware /boot; do
  if [ -d "$d" ] && touch "$d/.$OUT_NAME.tmp" 2>/dev/null; then
    rm -f "$d/.$OUT_NAME.tmp"
    OUT="$d/$OUT_NAME"
    break
  fi
done
OUT="${OUT:-./$OUT_NAME}"

{
  echo "node: $(hostname)"
  echo "date: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
  echo "--- model ---"
  cat /proc/device-tree/model 2>/dev/null; echo
  tr -d '\0' < /proc/device-tree/serial-number 2>/dev/null | sed 's/^/serial: /'
  echo "--- os ---"
  cat /etc/os-release 2>/dev/null | head -3
  uname -a
  echo "--- memory ---"
  free -h
  echo "--- storage ---"
  lsblk -o NAME,SIZE,MODEL,TRAN,FSTYPE,MOUNTPOINT 2>/dev/null
  echo "--- throttling (0x0 = clean) ---"
  vcgencmd get_throttled 2>/dev/null || echo "vcgencmd not available"
  echo "--- audio ---"
  cat /proc/asound/cards 2>/dev/null || echo "no /proc/asound/cards"
  aplay -l 2>/dev/null || true
  echo "--- usb / pci ---"
  lsusb 2>/dev/null || true
  lspci 2>/dev/null || true
  echo "--- cpu ---"
  grep -E "model name|Model|Hardware" /proc/cpuinfo 2>/dev/null | head -4
  nproc
} > "$OUT" 2>&1

echo "Wrote $OUT"
cat "$OUT"

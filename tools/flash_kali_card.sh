#!/bin/sh
# Flash the verified Kali Raspberry Pi arm64 image to a card for valseir
# (the audit station). Prepared by an agent; run by the operator
# (house rule: no host sudo from agents). DESTRUCTIVE to the target.
# Guards: 25-70 GB target only, never the disk mounted at /, typed
# confirmation required. Image + SHA-256 verified at download time and
# re-verified here before a byte moves.
#
# Usage: ./-os/tools/flash_kali_card.sh /dev/mmcblk0
set -eu

DEV="${1:-}"
IMG="$HOME/pi-images/kali/kali-linux-2026.3-raspberry-pi-arm64.img.xz"
SUMS="$HOME/pi-images/kali/SHA256SUMS"

[ -b "$DEV" ] || { echo "not a block device: $DEV" >&2; exit 2; }
[ -f "$IMG" ] || { echo "image missing: $IMG" >&2; exit 2; }
echo "--- verifying image checksum ---"
(cd "$(dirname "$IMG")" && grep raspberry-pi-arm64.img.xz SHA256SUMS | sha256sum -c -)

SIZE_BYTES="$(lsblk -bno SIZE "$DEV" | head -1)"
SIZE_GB=$((SIZE_BYTES / 1000000000))
echo "target: $DEV  ${SIZE_GB} GB  -> Kali 2026.3 arm64 for valseir (audit)"
if [ "$SIZE_GB" -lt 25 ] || [ "$SIZE_GB" -gt 70 ]; then
  echo "refusing: not a 25-70 GB device" >&2; exit 1
fi
if lsblk -no MOUNTPOINT "$DEV" | grep -qx "/"; then
  echo "refusing: target holds the running root" >&2; exit 1
fi
lsblk "$DEV"

printf "Type 'FLASH KALI' to erase %s: " "$DEV"
read CONFIRM
[ "$CONFIRM" = "FLASH KALI" ] || { echo "aborted"; exit 1; }

echo "--- writing image (3.1 GB compressed; several minutes) ---"
xzcat "$IMG" | sudo dd of="$DEV" bs=4M status=progress conv=fsync
sync
echo "Done. Card into valseir, screen/keyboard or network attached, power on."
echo "First boot expands and lands at a Kali login; fleet introduction"
echo "(hostname, SSH key, network) happens after, from the bench."

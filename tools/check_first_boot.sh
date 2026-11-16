#!/bin/sh
# check_first_boot.sh — inspect a κύριος SD card's boot partition for first-boot receipts.
# Run by the operator (house rule: no host sudo from agents).
# Usage: ./-os/tools/check_first_boot.sh /dev/mmcblk0
set -eu

DEV="${1:-}"
[ -n "$DEV" ] || { echo "usage: $0 /dev/<sd-device>" >&2; exit 2; }
[ -b "$DEV" ] || { echo "not a block device: $DEV" >&2; exit 2; }

# Find the FAT boot partition.
BOOT_PART=""
for p in "${DEV}p1" "${DEV}1"; do
  if [ -b "$p" ]; then
    BOOT_PART="$p"
    break
  fi
done
[ -n "$BOOT_PART" ] || { echo "no boot partition found on $DEV" >&2; exit 1; }

MNT="$(mktemp -d)"
echo "--- mounting boot partition $BOOT_PART ---"
sudo mount "$BOOT_PART" "$MNT"

echo ""
echo "--- first-boot files ---"
ls -la "$MNT" | grep -E "firstrun|kyrios|specs|cmdline|config" || true

echo ""
echo "--- cmdline.txt ---"
cat "$MNT/cmdline.txt" || true

echo ""
echo "--- firstrun.sh (if present) ---"
if [ -f "$MNT/firstrun.sh" ]; then
  cat "$MNT/firstrun.sh"
else
  echo "firstrun.sh is gone — the one-shot already ran."
fi

echo ""
echo "--- kyrios-first-boot-status.txt (if present) ---"
if [ -f "$MNT/kyrios-first-boot-status.txt" ]; then
  cat "$MNT/kyrios-first-boot-status.txt"
else
  echo "kyrios-first-boot-status.txt missing"
fi

echo ""
echo "--- specs-lirvol.txt (if present) ---"
if [ -f "$MNT/specs-lirvol.txt" ]; then
  head -20 "$MNT/specs-lirvol.txt"
else
  echo "specs-lirvol.txt missing"
fi

sudo umount "$MNT"
rmdir "$MNT"

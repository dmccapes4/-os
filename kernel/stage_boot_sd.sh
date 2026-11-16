#!/bin/sh
# Stage the kyrios B1 boot payload onto a FAT boot partition.
# Usage: ./stage_boot_sd.sh /path/to/mounted/boot-partition
# Run by the operator after mounting a SPARE card's boot partition
# (house rule: no agent host sudo). Overwrites config.txt on that
# partition — it is meant for a spare card, not a working Pi OS card.
set -eu

HERE="$(cd "$(dirname "$0")" && pwd)"
MNT="${1:-}"
[ -n "$MNT" ] && [ -d "$MNT" ] || { echo "usage: $0 /mounted/boot-partition" >&2; exit 2; }
[ -f "$HERE/kernel8.img" ] || { echo "kernel8.img missing — run make in $HERE first" >&2; exit 1; }

cp "$HERE"/boot-sd/* "$MNT"/
cp "$HERE/kernel8.img" "$MNT/kernel8.img"
sync
echo "staged on $MNT:"
ls -l "$MNT"
echo
echo "sha256 of the kernel that will boot:"
sha256sum "$MNT/kernel8.img"
echo "Next: unmount, card into valseir (Pi 4), its screen attached, power on."
echo "Pass: the screen says HELLO-KYRIOS on a deep slate field — photograph it."

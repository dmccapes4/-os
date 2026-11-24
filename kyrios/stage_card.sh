#!/bin/sh
# stage_card.sh: put kyrios on a Raspberry Pi 5 boot partition.
#
# Usage: ./stage_card.sh /path/to/mounted/boot-partition
#
# The partition must already be a Raspberry Pi boot partition with the
# Pi 5 device tree on it (a freshly flashed Raspberry Pi OS card is one).
# This script adds two files and changes nothing else:
#   kyrios-v0.0.0.img   the image built by `make`
#   config.txt          from boot/config.txt
# The config.txt found on the card is kept as config.txt.before-kyrios the
# first time, and that copy is never overwritten. To undo: copy it back
# over config.txt.
#
# No sudo. Mount first with:  udisksctl mount -b /dev/mmcblk0p1
# Running it again is harmless.
set -eu

HERE="$(cd "$(dirname "$0")" && pwd)"
MNT="${1:-}"
IMG="kyrios-v0.0.0.img"
SRC="$HERE/_build/$IMG"

[ -n "$MNT" ] && [ -d "$MNT" ] || { echo "usage: $0 /mounted/boot-partition" >&2; exit 2; }
[ -f "$SRC" ] || { echo "no image at $SRC (run make first)" >&2; exit 1; }
[ -f "$MNT/bcm2712-rpi-5-b.dtb" ] || { echo "no bcm2712-rpi-5-b.dtb on $MNT: not a Pi 5 boot partition" >&2; exit 1; }
[ -f "$MNT/config.txt" ] || { echo "no config.txt on $MNT: not a Pi boot partition" >&2; exit 1; }

if [ ! -f "$MNT/config.txt.before-kyrios" ]; then
    cp "$MNT/config.txt" "$MNT/config.txt.before-kyrios"
    echo "kept the card's config.txt as config.txt.before-kyrios"
fi
cp "$SRC" "$MNT/$IMG"
cp "$HERE/boot/config.txt" "$MNT/config.txt"
sync

WANT="$(sha256sum "$SRC" | cut -d' ' -f1)"
HAVE="$(sha256sum "$MNT/$IMG" | cut -d' ' -f1)"
[ "$WANT" = "$HAVE" ] || { echo "the image on the card does not match the build" >&2; exit 1; }
cmp -s "$HERE/boot/config.txt" "$MNT/config.txt" || { echo "config.txt on the card does not match" >&2; exit 1; }

echo "staged on $MNT:"
echo "  $IMG  $(wc -c < "$MNT/$IMG") bytes  sha256 $HAVE"
echo "  config.txt"
echo "Next: unmount (udisksctl unmount -b <device>), then the card goes into lirvol."

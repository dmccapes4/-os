#!/bin/sh
# Stage a kyrios kernel onto a mounted FAT boot partition (valseir, Pi 4).
# Usage: ./stage_boot_sd.sh /path/to/mounted/boot-partition [image-name]
# Default image: kyrios-b2.img (built by `make` and copied to boot-sd/).
# Preserves whatever config.txt it finds as config.txt.before-<image>;
# firmware files already on the card are left alone — a working Pi boot
# partition is all this needs. No sudo: mount with udisksctl first.
set -eu

HERE="$(cd "$(dirname "$0")" && pwd)"
MNT="${1:-}"
IMG="${2:-kyrios-b2.img}"
[ -n "$MNT" ] && [ -d "$MNT" ] || { echo "usage: $0 /mounted/boot-partition [image]" >&2; exit 2; }
[ -f "$HERE/boot-sd/$IMG" ] || { echo "payload missing: boot-sd/$IMG (run make, then cp kernel8.img boot-sd/$IMG)" >&2; exit 1; }
[ -f "$MNT/start4.elf" ] || { echo "no start4.elf on $MNT — is this a Pi boot partition?" >&2; exit 1; }

cp "$HERE/boot-sd/$IMG" "$MNT/$IMG"
[ -f "$MNT/config.txt" ] && cp "$MNT/config.txt" "$MNT/config.txt.before-$IMG"
cat > "$MNT/config.txt" <<EOF
# kyrios boot config - valseir (Pi 4, BCM2711). Previous config kept as
# config.txt.before-$IMG. Firmware loads OUR $IMG and nothing else.
arm_64bit=1
kernel=$IMG
disable_overscan=1
hdmi_force_hotplug=1
EOF
sync
echo "staged $IMG on $MNT:"
sha256sum "$MNT/$IMG"
grep kernel= "$MNT/config.txt"
echo "Next: unmount, card into valseir, screen on, power."

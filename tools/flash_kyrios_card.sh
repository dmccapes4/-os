#!/bin/sh
# flash_kyrios_card.sh — flash a κύριος v0.0.1 image to lirvol's SD card.
#
# Uses the Bookworm arm64 Lite base image and seeds it with the project skeleton
# (ledger, spaces, processors) so the Pi boots straight into the first κύριος
# processor and writes its first receipt to the NVMe ledger.
#
# Prepared by an agent; run by the operator (house rule: no host sudo from agents).
# DESTRUCTIVE: the target device is erased.
# Usage: ./-os/tools/flash_kyrios_card.sh /dev/mmcblk0
set -eu

DEV="${1:-}"
NODE="lirvol"
IMG="${KYRIOS_BASE_IMG:-$HOME/pi-images/2025-05-06-raspios-bookworm-arm64-lite.img.xz}"
SPECS="$(dirname "$0")/first_boot_specs.sh"
FIRST_BOOT="$(dirname "$0")/first_boot_kyrios.sh"
KYRIOS_ROOT_SRC="$(dirname "$0")/.."
KEY="$HOME/.ssh/kyrios_fleet_node"
SSH_USER="kyrios"

[ -n "$DEV" ] || { echo "usage: $0 /dev/<sd-device>" >&2; exit 2; }
[ -b "$DEV" ] || { echo "not a block device: $DEV" >&2; exit 2; }
[ -f "$IMG" ] || { echo "base image missing: $IMG (download the Bookworm arm64 Lite image to ~/pi-images)" >&2; exit 2; }
[ -f "$SPECS" ] || { echo "spec collector missing: $SPECS" >&2; exit 2; }
[ -f "$FIRST_BOOT" ] || { echo "first-boot script missing: $FIRST_BOOT" >&2; exit 2; }

SIZE_BYTES="$(lsblk -bno SIZE "$DEV" | head -1)"
SIZE_GB=$((SIZE_BYTES / 1000000000))
echo "target: $DEV  ${SIZE_GB} GB  node: $NODE"
if [ "$SIZE_GB" -lt 25 ] || [ "$SIZE_GB" -gt 70 ]; then
  echo "refusing: target is not a 25–70 GB device (the 32 GB or 64 GB card)" >&2; exit 1
fi
if lsblk -no MOUNTPOINT "$DEV" | grep -qx "/"; then
  echo "refusing: target holds the running root filesystem" >&2; exit 1
fi
lsblk "$DEV"

# Fleet SSH key on this bench (passphrase-free by design; scoped to fleet nodes).
if [ ! -f "$KEY" ]; then
  echo "--- generating fleet SSH key: $KEY ---"
  ssh-keygen -t ed25519 -N "" -f "$KEY" -C "kyrios-fleet-node"
fi
PUBKEY="$(cat "$KEY.pub")"

printf "Wi-Fi SSID [Twinkle]: "
read WIFI_SSID
WIFI_SSID="${WIFI_SSID:-Twinkle}"
printf "Wi-Fi passphrase (hidden; goes only onto this card, then deletes itself): "
stty -echo
read WIFI_PSK
stty echo
printf "\n"
[ -n "$WIFI_PSK" ] || { echo "refusing: empty Wi-Fi passphrase" >&2; exit 1; }
printf "Wi-Fi country [US]: "
read WIFI_COUNTRY
WIFI_COUNTRY="${WIFI_COUNTRY:-US}"

printf "Type 'FLASH %s' to erase %s and write the κύριος image: " "$NODE" "$DEV"
read CONFIRM
[ "$CONFIRM" = "FLASH $NODE" ] || { echo "aborted (confirmation did not match)"; exit 1; }

# Shell-quote a string for safe embedding in generated firstrun.sh.
sq() { printf "'%s'" "$(printf '%s' "$1" | sed "s/'/'\\\\''/g")"; }
Q_SSID="$(sq "$WIFI_SSID")"; Q_PSK="$(sq "$WIFI_PSK")"
Q_COUNTRY="$(sq "$WIFI_COUNTRY")"; Q_PUBKEY="$(sq "$PUBKEY")"

echo "--- writing base image (several minutes) ---"
xzcat "$IMG" | sudo dd of="$DEV" bs=4M status=progress conv=fsync
sync
sudo partprobe "$DEV" 2>/dev/null || true
sleep 2

# Identify partitions after partprobe.
BOOT_PART=""; ROOT_PART=""
for p in "${DEV}p1" "${DEV}1"; do
  [ -b "$p" ] && BOOT_PART="$p" && break
done
for p in "${DEV}p2" "${DEV}2"; do
  [ -b "$p" ] && ROOT_PART="$p" && break
done
[ -n "$BOOT_PART" ] || { echo "no boot partition found" >&2; exit 1; }
[ -n "$ROOT_PART" ] || { echo "no root partition found" >&2; exit 1; }

MNT_BOOT="$(mktemp -d)"
MNT_ROOT="$(mktemp -d)"

echo "--- seeding boot partition $BOOT_PART ---"
sudo mount "$BOOT_PART" "$MNT_BOOT"
sudo cp "$SPECS" "$MNT_BOOT/first_boot_specs.sh"
sudo cp "$FIRST_BOOT" "$MNT_BOOT/first_boot_kyrios.sh"
sudo chmod +x "$MNT_BOOT/first_boot_specs.sh" "$MNT_BOOT/first_boot_kyrios.sh"

sudo tee "$MNT_BOOT/firstrun.sh" >/dev/null <<EOF
#!/bin/bash
set +e
CUSTOM=/usr/lib/raspberrypi-sys-mods/imager_custom
if [ -f "\$CUSTOM" ]; then
  \$CUSTOM set_hostname lirvol
else
  echo lirvol > /etc/hostname
  sed -i "s/127.0.1.1.*/127.0.1.1 lirvol/" /etc/hosts
fi
hostname lirvol 2>/dev/null
id kyrios >/dev/null 2>&1 || useradd -m -s /bin/bash -G sudo kyrios
if [ -f "\$CUSTOM" ]; then
  \$CUSTOM enable_ssh -k $Q_PUBKEY
  \$CUSTOM set_wlan $Q_SSID $Q_PSK $Q_COUNTRY
else
  systemctl enable ssh 2>/dev/null
fi
sh /boot/firmware/first_boot_kyrios.sh || sh /boot/first_boot_kyrios.sh || true
sh /boot/firmware/first_boot_specs.sh || sh /boot/first_boot_specs.sh || true
rm -f /boot/firmware/firstrun.sh /boot/firstrun.sh
sed -i 's| systemd.run.*||g' /boot/firmware/cmdline.txt 2>/dev/null
sed -i 's| systemd.run.*||g' /boot/cmdline.txt 2>/dev/null
exit 0
EOF
sudo chmod +x "$MNT_BOOT/firstrun.sh"

# Pi 5 + audio board overlay for lirvol.
if ! grep -q "^dtoverlay=hifiberry-digi" "$MNT_BOOT/config.txt"; then
  sudo tee -a "$MNT_BOOT/config.txt" >/dev/null <<'EOF'

# κύριος lirvol v0.0.1
# 52Pi NVDigi / WM8804 S/PDIF
# See -os/fleet/README.md measured specs for board identification.
dtoverlay=hifiberry-digi
EOF
fi

CMDLINE="$MNT_BOOT/cmdline.txt"
if ! grep -q "firstrun.sh" "$CMDLINE"; then
  sudo sed -i 's|$| systemd.run=/boot/firmware/firstrun.sh systemd.run_success_action=reboot systemd.unit=kernel-command-line.target|' "$CMDLINE"
fi
sudo umount "$MNT_BOOT"
rmdir "$MNT_BOOT"

echo "--- copying κύριος skeleton to root partition $ROOT_PART ---"
sudo mount "$ROOT_PART" "$MNT_ROOT"
sudo mkdir -p "$MNT_ROOT/opt/kyrios"
# Copy the whole -os tree into /opt/kyrios on the Pi.
sudo cp -a "$KYRIOS_ROOT_SRC/." "$MNT_ROOT/opt/kyrios/"
sudo find "$MNT_ROOT/opt/kyrios" -type f \( -name "*.sh" -o -name "*.py" \) -exec chmod +x {} \;

# Build receipt inside the image so the Pi knows what it is.
sudo tee "$MNT_ROOT/opt/kyrios/build/BUILD_RECEIPT_$(date -u +%Y%m%dT%H%M%SZ).md" >/dev/null <<EOF2
# κύριος v0.0.1 build receipt

**Node:** lirvol
**Flashed:** $(date -u +%Y-%m-%dT%H:%M:%SZ)
**Base image:** $IMG
**Source workspace:** $KYRIOS_ROOT_SRC
**Fleet SSH key fingerprint:** $(ssh-keygen -lf "$KEY.pub" | awk '{print $2}')
**Operator action:** FLASH $NODE to $DEV
EOF2

sudo umount "$MNT_ROOT"
rmdir "$MNT_ROOT"

sync
unset WIFI_PSK Q_PSK
echo "Done. Card is seeded for node: $NODE (user $SSH_USER, key-only SSH, Wi-Fi at first boot)"
echo "Next: insert card into $NODE, power on, wait for the automatic reboot, then from this bench:"
echo "  ssh -i $KEY $SSH_USER@$NODE.local"
echo "Status and specs will land on the card's boot partition as:"
echo "  kyrios-first-boot-status.txt and specs-lirvol.txt"

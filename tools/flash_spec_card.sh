#!/bin/sh
# Flash the Bookworm spec-collection image to an SD card and pre-seed it the
# way a Pi is actually first met: SSH on, Wi-Fi joined, then specs.
# On first boot the Pi: sets its node hostname, creates the fleet user,
# enables key-only SSH, joins the Wi-Fi the operator typed at flash time,
# writes its own specs file to the boot partition, and reboots once.
# After that: ssh kyrios@<node>.local   (key-only; private key on this bench)
#
# Secrets: the Wi-Fi passphrase is typed at a hidden prompt on THIS bench,
# written only into firstrun.sh on the card, and firstrun.sh deletes itself
# after first boot. It is never printed and never enters this repo.
#
# Prepared by an agent; run by the operator (house rule: no host sudo from agents).
# DESTRUCTIVE: the target device is erased. The script refuses anything that
# is not a 25–70 GB disk (the fleet's 32 GB / 64 GB cards), refuses the disk
# mounted at /, and requires the operator to type FLASH <node> before writing.
#
# Usage: ./-os/tools/flash_spec_card.sh /dev/mmcblk0 lirvol
#        ./-os/tools/flash_spec_card.sh /dev/mmcblk0 valseir
set -eu

DEV="${1:-}"
NODE="${2:-}"
IMG="$HOME/pi-images/2025-05-06-raspios-bookworm-arm64-lite.img.xz"
SPECS="$(dirname "$0")/first_boot_specs.sh"
KEY="$HOME/.ssh/kyrios_fleet_node"
SSH_USER="kyrios"

case "$NODE" in
  lirvol|valseir) ;;
  *) echo "usage: $0 /dev/<sd-device> lirvol|valseir" >&2; exit 2 ;;
esac
[ -b "$DEV" ] || { echo "not a block device: $DEV" >&2; exit 2; }
[ -f "$IMG" ] || { echo "image missing: $IMG (agent re-downloads it; do not substitute another)" >&2; exit 2; }
[ -f "$SPECS" ] || { echo "spec collector missing: $SPECS" >&2; exit 2; }

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

# Fleet SSH key on this bench (generated once, passphrase-free by design: it is
# scoped to the fleet nodes only — see the 2026-11-16 key record in fleet docs).
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
[ -n "$WIFI_PSK" ] || { echo "refusing: empty Wi-Fi passphrase (first step with a Pi is SSH; SSH needs the network)" >&2; exit 1; }
printf "Wi-Fi country [US]: "
read WIFI_COUNTRY
WIFI_COUNTRY="${WIFI_COUNTRY:-US}"

printf "Type 'FLASH %s' to erase %s and write the spec image: " "$NODE" "$DEV"
read CONFIRM
[ "$CONFIRM" = "FLASH $NODE" ] || { echo "aborted (confirmation did not match)"; exit 1; }

# Single-quote a string for safe embedding in the generated firstrun.sh.
sq() { printf "'%s'" "$(printf '%s' "$1" | sed "s/'/'\\\\''/g")"; }
Q_SSID="$(sq "$WIFI_SSID")"; Q_PSK="$(sq "$WIFI_PSK")"
Q_COUNTRY="$(sq "$WIFI_COUNTRY")"; Q_PUBKEY="$(sq "$PUBKEY")"

echo "--- writing image (several minutes) ---"
xzcat "$IMG" | sudo dd of="$DEV" bs=4M status=progress conv=fsync
sync
sudo partprobe "$DEV" 2>/dev/null || true
sleep 2

PART="${DEV}p1"
[ -b "$PART" ] || PART="${DEV}1"
MNT="$(mktemp -d)"
echo "--- seeding boot partition $PART (hostname, user, SSH, Wi-Fi, specs) ---"
sudo mount "$PART" "$MNT"
sudo cp "$SPECS" "$MNT/first_boot_specs.sh"
sudo tee "$MNT/firstrun.sh" >/dev/null <<EOF
#!/bin/bash
set +e
CUSTOM=/usr/lib/raspberrypi-sys-mods/imager_custom
if [ -f "\$CUSTOM" ]; then
  \$CUSTOM set_hostname $NODE
else
  echo $NODE > /etc/hostname
  sed -i "s/127.0.1.1.*/127.0.1.1 $NODE/" /etc/hosts
fi
hostname $NODE 2>/dev/null
id $SSH_USER >/dev/null 2>&1 || useradd -m -s /bin/bash -G sudo $SSH_USER
if [ -f "\$CUSTOM" ]; then
  \$CUSTOM enable_ssh -k $Q_PUBKEY
  \$CUSTOM set_wlan $Q_SSID $Q_PSK $Q_COUNTRY
else
  systemctl enable ssh 2>/dev/null
fi
sh /boot/firmware/first_boot_specs.sh || sh /boot/first_boot_specs.sh || true
rm -f /boot/firmware/firstrun.sh /boot/firstrun.sh
sed -i 's| systemd.run.*||g' /boot/firmware/cmdline.txt 2>/dev/null
sed -i 's| systemd.run.*||g' /boot/cmdline.txt 2>/dev/null
exit 0
EOF
sudo chmod +x "$MNT/firstrun.sh" "$MNT/first_boot_specs.sh"
CMDLINE="$MNT/cmdline.txt"
if ! grep -q "firstrun.sh" "$CMDLINE"; then
  sudo sed -i 's|$| systemd.run=/boot/firmware/firstrun.sh systemd.run_success_action=reboot systemd.unit=kernel-command-line.target|' "$CMDLINE"
fi
sudo umount "$MNT"
rmdir "$MNT"
sync
# The passphrase lives only in this shell's memory and on the card (which deletes it at first boot).
unset WIFI_PSK Q_PSK
echo "Done. Card is seeded for node: $NODE (user $SSH_USER, key-only SSH, Wi-Fi at first boot)"
echo "Next: card into the $NODE Pi, power on, wait for the reboot, then from this bench:"
echo "  ssh -i $KEY $SSH_USER@$NODE.local"
echo "Specs also land on the card as specs-$NODE.txt if SSH needs a witness."

#!/bin/sh
# lirvol_sd_step1.sh — text console, sshd, no-password LAN access on lirvol.
#
# lirvol is the κύριος machine (Raspberry Pi 5). This script configures a
# Kali Linux SD card for lirvol before first boot: the same discipline as
# valseir_sd_step1.sh. The κύριος bare-metal image has no Ethernet driver yet
# (spec §9.4); Kali is the tooling OS for this phase.
#
# The lirvol root must already be mounted. Run as root on the bench:
#   sudo sh lirvol_sd_step1.sh /media/lucifer-chains/ROOTFS
#
# What it changes on that root:
#   1. default target becomes multi-user (text, no XFCE/LightDM).
#   2. ssh.service starts at multi-user.
#   3. tty1 logs in as kali (autologin).
#   4. From 10.0.0.* or localhost, PAM grants kali on first attempt without
#      checking the password. PermitEmptyPasswords set for that range.
#   5. hostname set to lirvol (Kali image default is kali-raspberrypi).
#
# After first boot:
#   ssh kali@lirvol.local          (avahi, once avahi-daemon is installed)
#   ssh kali@<IP>                  (check router DHCP table for lirvol)
# Then run tools/lirvol_provision.sh (once written) over SSH.
#
# Running this script again is harmless.

set -eu

ROOT=${1:-}
[ "$(id -u)" = 0 ] || { echo "run as root: sudo sh $0 <mounted-root>" >&2; exit 2; }
[ -n "$ROOT" ] && [ -f "$ROOT/etc/hostname" ] || {
    echo "usage: sudo sh $0 /media/lucifer-chains/ROOTFS" >&2; exit 2;
}
grep -q '^ID=kali$' "$ROOT/etc/os-release" || { echo "not a Kali root" >&2; exit 1; }

# ── 1. Text console (multi-user, no display manager) ─────────────────────
ln -sfn /usr/lib/systemd/system/multi-user.target \
    "$ROOT/etc/systemd/system/default.target"

# ── 2. SSH at boot ─────────────────────────────────────────────────────────
install -d "$ROOT/etc/systemd/system/multi-user.target.wants"
ln -sfn /usr/lib/systemd/system/ssh.service \
    "$ROOT/etc/systemd/system/multi-user.target.wants/ssh.service"
rm -f "$ROOT/etc/ssh/sshd_not_to_be_run"

# ── 3. tty1 autologin as kali ─────────────────────────────────────────────
install -d "$ROOT/etc/systemd/system/getty@tty1.service.d"
cat > "$ROOT/etc/systemd/system/getty@tty1.service.d/autologin.conf" << 'EOF'
[Service]
ExecStart=
ExecStart=-/sbin/agetty --autologin kali --noclear %I $TERM
EOF

# ── 4. Passwordless SSH from the bench LAN ────────────────────────────────
cat > "$ROOT/etc/security/access-sshd-lan.conf" << 'EOF'
# kali from the bench LAN is granted without password check.
+ : kali : 10.0.0. 127.0.0.1 ::1
- : ALL : ALL
EOF
chmod 644 "$ROOT/etc/security/access-sshd-lan.conf"

pam="$ROOT/etc/pam.d/sshd"
if ! grep -q 'access-sshd-lan.conf' "$pam"; then
    tmp=$(mktemp)
    awk '
        /^@include common-auth$/ && !done {
            print "auth sufficient pam_access.so accessfile=/etc/security/access-sshd-lan.conf"
            done = 1
        }
        { print }
    ' "$pam" > "$tmp"
    cat "$tmp" > "$pam"
    rm -f "$tmp"
fi

cat > "$ROOT/etc/ssh/sshd_config.d/lan-noauth.conf" << 'EOF'
# Allow kali from the bench LAN through to PAM without a typed password.
Match Address 10.0.0.0/24,127.0.0.1,::1
    PermitEmptyPasswords yes
EOF
chmod 644 "$ROOT/etc/ssh/sshd_config.d/lan-noauth.conf"

# ── 5. Hostname ────────────────────────────────────────────────────────────
# Kali Pi images start as kali-raspberrypi. Set it to lirvol now so avahi
# advertises lirvol.local from the first boot.
OLD_HOSTNAME=$(cat "$ROOT/etc/hostname")
printf 'lirvol\n' > "$ROOT/etc/hostname"
# Update /etc/hosts so the new hostname resolves locally.
if grep -q "$OLD_HOSTNAME" "$ROOT/etc/hosts" 2>/dev/null; then
    sed -i "s/$OLD_HOSTNAME/lirvol/g" "$ROOT/etc/hosts"
fi
echo "hostname: $OLD_HOSTNAME -> lirvol"

# ── Summary ───────────────────────────────────────────────────────────────
sync
echo ""
echo "default target: $(readlink "$ROOT/etc/systemd/system/default.target")"
echo "ssh wants: $(readlink "$ROOT/etc/systemd/system/multi-user.target.wants/ssh.service")"
echo "hostname: $(cat "$ROOT/etc/hostname")"
echo "pam lan line:"
grep 'access-sshd-lan' "$pam"
echo ""
echo "Unmount the card, put it in lirvol, and power on."
echo "lirvol comes up second: valseir (ὄνυχος) must be running first."
echo ""
echo "Find lirvol on the LAN:"
echo "  ssh kali@lirvol.local          (needs avahi-daemon installed; first boot may not have it)"
echo "  ssh kali@<IP>                  (check router DHCP table, or: arp-scan -l | grep -i raspberry)"
echo ""
echo "Once in, run: sudo sh lirvol_provision.sh"
echo "(lirvol_provision.sh is not yet written; it will follow the same"
echo " pattern as valseir_provision_onyx.sh once the lirvol role is settled.)"

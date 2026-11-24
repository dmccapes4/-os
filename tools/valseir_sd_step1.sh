#!/bin/sh
# valseir_sd_step1.sh: text console, sshd, no password check on the LAN.
#
# The valseir root must already be mounted. Run as root:
#   sudo sh valseir_sd_step1.sh /media/lucifer-chains/ROOTFS
#
# What it changes on that root:
#   1. default target becomes multi-user, so the next boot stops at a
#      terminal. LightDM is the display manager and is not started from
#      multi-user.
#   2. ssh.service stays wanted by multi-user, so sshd starts at boot.
#   3. tty1 logs in as kali.
#   4. For user kali, from 10.0.0.* or localhost, sshd's PAM stack grants
#      access on the first password attempt and does not check the
#      secret. Other origins still go through the normal password check.
#      PermitEmptyPasswords is set for 10.0.0.0/24 so an empty attempt
#      is not refused before PAM.
# Running it again changes nothing that is already set.
set -eu

ROOT=${1:-}
[ "$(id -u)" = 0 ] || { echo "run as root: sudo sh $0 <mounted-root>" >&2; exit 2; }
[ -n "$ROOT" ] && [ -f "$ROOT/etc/hostname" ] || { echo "usage: sudo sh $0 /media/lucifer-chains/ROOTFS" >&2; exit 2; }
grep -qx 'kali-raspberrypi' "$ROOT/etc/hostname" || { echo "hostname is not kali-raspberrypi" >&2; exit 1; }
grep -q '^ID=kali$' "$ROOT/etc/os-release" || { echo "not a Kali root" >&2; exit 1; }

ln -sfn /usr/lib/systemd/system/multi-user.target "$ROOT/etc/systemd/system/default.target"

install -d "$ROOT/etc/systemd/system/multi-user.target.wants"
ln -sfn /usr/lib/systemd/system/ssh.service \
    "$ROOT/etc/systemd/system/multi-user.target.wants/ssh.service"
rm -f "$ROOT/etc/ssh/sshd_not_to_be_run"

install -d "$ROOT/etc/systemd/system/getty@tty1.service.d"
cat > "$ROOT/etc/systemd/system/getty@tty1.service.d/autologin.conf" << 'EOF'
[Service]
ExecStart=
ExecStart=-/sbin/agetty --autologin kali --noclear %I $TERM
EOF

cat > "$ROOT/etc/security/access-sshd-lan.conf" << 'EOF'
# First matching line wins. kali from the bench LAN is granted.
# Anything else is denied here and falls through to the normal password check.
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
# Empty password attempts from the bench LAN are allowed through to PAM.
# PAM grants kali from that LAN without checking the secret.
Match Address 10.0.0.0/24,127.0.0.1,::1
    PermitEmptyPasswords yes
EOF
chmod 644 "$ROOT/etc/ssh/sshd_config.d/lan-noauth.conf"

sync
echo "default target: $(readlink "$ROOT/etc/systemd/system/default.target")"
echo "ssh wants: $(readlink "$ROOT/etc/systemd/system/multi-user.target.wants/ssh.service")"
echo "pam lan line:"
grep 'access-sshd-lan' "$pam"
echo "Unmount the card, put it back in valseir, and boot."

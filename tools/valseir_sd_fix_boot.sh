#!/bin/sh
# valseir_sd_fix_boot.sh — restore valseir to a normal Kali terminal boot.
#
# valseir_provision_onyx.sh added an exec hook to .bash_profile and .zprofile
# that replaced the login shell with the onyx-watch script on tty1. The
# reports application is not built yet. This left valseir appearing dead on
# the attached display.
#
# This script removes that hook. Nothing else is changed.
# Multi-user target, SSH, autologin as kali: all stay.
# After this, valseir boots to a normal Kali terminal prompt on tty1.
#
# The valseir root must already be mounted. Run as root on the bench:
#   sudo mount /dev/mmcblk0p2 /mnt
#   sudo sh valseir_sd_fix_boot.sh /mnt
#   sudo umount /mnt
#
# Running this script again is harmless.

set -eu

ROOT=${1:-}
[ "$(id -u)" = 0 ] || { echo "run as root: sudo sh $0 <mounted-root>" >&2; exit 2; }
[ -n "$ROOT" ] && [ -f "$ROOT/etc/hostname" ] || {
    echo "usage: sudo sh $0 /mnt" >&2; exit 2;
}
grep -q '^ID=kali$' "$ROOT/etc/os-release" || { echo "not a Kali root" >&2; exit 1; }

CHANGED=0

for profile in "$ROOT/home/kali/.bash_profile" "$ROOT/home/kali/.zprofile"; do
    if [ -f "$profile" ] && grep -q 'onyx-watch' "$profile"; then
        # Remove the two lines added by valseir_provision_onyx.sh.
        # They are:  # onyx: show the last report on the attached display
        #            [ "$(tty)" = "/dev/tty1" ] && exec /usr/local/bin/onyx-watch
        tmp=$(mktemp)
        grep -v 'onyx-watch\|onyx: show the last report' "$profile" > "$tmp"
        cat "$tmp" > "$profile"
        rm -f "$tmp"
        echo "removed onyx-watch hook from $profile"
        CHANGED=$((CHANGED + 1))
    else
        echo "$profile: no hook found (already clean)"
    fi
done

if [ "$CHANGED" -eq 0 ]; then
    echo "nothing to change — card may already be clean"
fi

# Confirm the boot setup is intact.
echo ""
echo "== boot state =="
echo "default target: $(readlink "$ROOT/etc/systemd/system/default.target" 2>/dev/null || echo '(not set)')"
echo "ssh enabled: $(test -L "$ROOT/etc/systemd/system/multi-user.target.wants/ssh.service" && echo yes || echo NO)"
echo "autologin: $(cat "$ROOT/etc/systemd/system/getty@tty1.service.d/autologin.conf" 2>/dev/null | grep autologin || echo '(not set)')"
echo ""
echo "Done. Unmount the card and put it back in valseir."
echo "valseir will boot to a Kali terminal prompt on tty1."
echo "SSH: ssh kali@<IP>  (no password needed from 10.0.0.x)"

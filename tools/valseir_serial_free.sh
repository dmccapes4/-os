#!/bin/sh
# valseir_serial_free.sh: stop valseir talking on its own serial wire.
#
# Run ON valseir, as root:   sudo sh valseir_serial_free.sh
# Then reboot valseir. Running it again changes nothing.
#
# The wire on pins 8/10 is for kyrios. Two things on valseir use it by
# default once the UART exists, and both are silenced here:
#   1. a login getty on ttyAMA0 (stopped, then masked so it stays off);
#   2. the kernel console named in cmdline.txt (the console=serial0,...
#      word is removed; the old file is kept as cmdline.txt.before-onyx).
# valseir keeps its screen console (tty1) and SSH.
set -eu

CMDLINE=/boot/firmware/cmdline.txt

[ "$(id -u)" = 0 ] || { echo "run as root: sudo sh $0" >&2; exit 2; }

systemctl stop serial-getty@ttyAMA0.service 2>/dev/null || true
systemctl mask serial-getty@ttyAMA0.service

if [ -f "$CMDLINE" ]; then
    if grep -qw "console=serial0,115200" "$CMDLINE"; then
        [ -f "$CMDLINE.before-onyx" ] || cp "$CMDLINE" "$CMDLINE.before-onyx"
        sed -i 's/console=serial0,115200 //; s/ console=serial0,115200//' "$CMDLINE"
        echo "removed the serial console from $CMDLINE"
    else
        echo "no serial console word in $CMDLINE"
    fi
    cat "$CMDLINE"
else
    echo "no $CMDLINE" >&2; exit 1
fi
sync
echo "Now reboot valseir."

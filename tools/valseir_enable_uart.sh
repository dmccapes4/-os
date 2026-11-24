#!/bin/sh
# valseir_enable_uart.sh: give valseir (Pi 4, Kali) a serial port on header
# pins 8 and 10, for the wire to lirvol (game plan step O2).
#
# Run ON valseir, as root:   sudo sh valseir_enable_uart.sh
# Then reboot valseir. Running it again changes nothing.
#
# enable_uart=1 turns the header UART on; dtoverlay=disable-bt puts the
# PL011 (stable baud rate) on pins 8/10 instead of the mini UART and
# turns Bluetooth off, which onychos does not use. The config found is
# kept once as config.txt.before-onyx-uart.
set -eu

CFG=/boot/firmware/config.txt

[ "$(id -u)" = 0 ] || { echo "run as root: sudo sh $0" >&2; exit 2; }
[ -f "$CFG" ] || { echo "no $CFG: this is for valseir" >&2; exit 1; }

if [ ! -f "$CFG.before-onyx-uart" ]; then
    cp "$CFG" "$CFG.before-onyx-uart"
    echo "kept the old config as $CFG.before-onyx-uart"
fi

for line in "enable_uart=1" "dtoverlay=disable-bt"; do
    if grep -qx "$line" "$CFG"; then
        echo "already there: $line"
    else
        echo "$line" >> "$CFG"
        echo "added: $line"
    fi
done
sync
tail -4 "$CFG"
echo "Now reboot valseir."

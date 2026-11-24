#!/bin/sh
# valseir_console_boot.sh: text console, ssh at boot, no password prompt.
#
# Run ON valseir, as root:   sudo sh valseir_console_boot.sh
# Then reboot valseir. Running it again changes nothing that is already set.
#
# What it does:
#   1. default target becomes multi-user, so boot stops at a terminal and
#      the enabled services start. A display manager, if one is enabled,
#      is disabled.
#   2. ssh is enabled, so it starts at boot.
#   3. tty1 logs in as kali with no prompt. kali's password is removed,
#      sudo does not ask, and sshd accepts an empty password.
#
# Wake-on-LAN is not in this script. The Pi 4 Ethernet hardware cannot
# wake the board from halt; Raspberry Pi marked that request wontfix
# because the hardware cannot do it (raspberrypi/rpi-eeprom issue 160).
# The bootloader wake is a pin (GPIO3 or GLOBAL_EN to ground), not a
# packet. See WAKE_ON_GPIO in the bootloader configuration.
set -eu

[ "$(id -u)" = 0 ] || { echo "run as root: sudo sh $0" >&2; exit 2; }

id kali >/dev/null 2>&1 || { echo "no user kali" >&2; exit 1; }

systemctl set-default multi-user.target

for dm in lightdm gdm gdm3 sddm xdm; do
    if systemctl list-unit-files "${dm}.service" --no-legend 2>/dev/null | grep -q "${dm}.service"; then
        systemctl disable --now "${dm}.service" 2>/dev/null || true
        echo "disabled ${dm}"
    fi
done

systemctl enable ssh
systemctl start ssh

install -d -m 755 /etc/systemd/system/getty@tty1.service.d
cat > /etc/systemd/system/getty@tty1.service.d/autologin.conf << 'EOF'
[Service]
ExecStart=
ExecStart=-/sbin/agetty --autologin kali --noclear %I $TERM
EOF
systemctl daemon-reload
systemctl restart getty@tty1.service || true

cat > /etc/sudoers.d/kali-nopasswd << 'EOF'
kali ALL=(ALL) NOPASSWD: ALL
EOF
chmod 440 /etc/sudoers.d/kali-nopasswd
visudo -cf /etc/sudoers.d/kali-nopasswd

passwd -d kali

install -d -m 755 /etc/ssh/sshd_config.d
cat > /etc/ssh/sshd_config.d/empty-password.conf << 'EOF'
PermitEmptyPasswords yes
EOF
# A later Match or the main file can still force "no". Say so if it does.
if sshd -T 2>/dev/null | grep -q '^permitemptypasswords no$'; then
    echo "sshd still has PermitEmptyPasswords no after the drop-in" >&2
    exit 1
fi
systemctl reload ssh || systemctl restart ssh

sync
echo "default target: $(systemctl get-default)"
echo "ssh: $(systemctl is-enabled ssh) / $(systemctl is-active ssh)"
echo "Now reboot valseir."

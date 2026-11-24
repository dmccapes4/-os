#!/bin/sh
# valseir_br0_nm.sh — set up br0 on valseir using NetworkManager.
#
# valseir_br0.sh wrote systemd-networkd files. Kali on Pi uses NetworkManager,
# so those files were ignored. This script replaces it.
#
# Run ON valseir, as root:
#   sudo sh valseir_br0_nm.sh [IFACE]
#
# IFACE defaults to eth0. Override if Kali named it differently:
#   sudo sh valseir_br0_nm.sh enp2s0
#
# ── BEFORE RUNNING: SSH SURVIVAL ──────────────────────────────────────────
# br0 is specified at 0.0.0.0 (no IP). Enslaving eth0 to br0 removes the IP
# from eth0. If eth0 is your only path to valseir, your SSH session dies.
#
# Two options before proceeding:
#
#   A. Configure WiFi first (wlan0) so SSH has another path:
#        sudo nmcli device wifi connect "<SSID>" password "<PASS>"
#      Then come back and run this script. SSH continues on wlan0 at whatever
#      IP the router gives it. Use: ssh kali@kali-raspberrypi.local
#
#   B. Give br0 a temporary DHCP address (departs from spec §3 0.0.0.0, but
#      keeps valseir reachable on the same IP):
#        DHCP_BR0=yes sudo sh valseir_br0_nm.sh
#      Later, when WiFi is set up and confirmed, remove br0's IP:
#        nmcli con mod br0 ipv4.method disabled ipv6.method ignore
#        nmcli con up br0
#
# The spec (§3 item 4) leaves a second bridge member (for routing lirvol's
# external traffic through valseir) undecided. This script creates a
# single-member bridge: eth0 → br0. Add a second member when that is settled.
#
# ── TIDY UP THE OLD ATTEMPT ───────────────────────────────────────────────
# The previous valseir_br0.sh wrote systemd-networkd files. This removes them
# and disables systemd-networkd (NetworkManager is what Kali runs).
#
# Running this script again is harmless.

set -eu

IFACE="${1:-eth0}"
DHCP_BR0="${DHCP_BR0:-no}"
OUT="valseir-br0-nm-$(date +%Y%m%d).txt"

[ "$(id -u)" = 0 ] || { echo "run as root: sudo sh $0 [IFACE]" >&2; exit 2; }
[ "$(dpkg --print-architecture)" = arm64 ] || { echo "not arm64; this is for valseir" >&2; exit 1; }

ip link show "$IFACE" > /dev/null 2>&1 || {
    echo "interface $IFACE not found. Available:" >&2
    ip -o link show | awk -F': ' '{print "  " $2}' >&2
    exit 1
}

# Check if WiFi is up — reassure the operator that SSH has a fallback.
WIFI_IP=""
if ip link show wlan0 > /dev/null 2>&1; then
    WIFI_IP=$(ip -4 addr show wlan0 2>/dev/null | awk '/inet /{print $2}' | head -1)
fi

if [ "$DHCP_BR0" != "yes" ] && [ -z "$WIFI_IP" ]; then
    echo "──────────────────────────────────────────────────────────────────"
    echo "STOP: WiFi (wlan0) is not up and DHCP_BR0 is not set."
    echo ""
    echo "If you enslave eth0 to br0 (no IP) now, SSH will drop and not"
    echo "return unless you have a serial console or physical keyboard."
    echo ""
    echo "Option A — configure WiFi first, then re-run this script:"
    echo "  sudo nmcli device wifi connect \"<SSID>\" password \"<PASS>\""
    echo ""
    echo "Option B — give br0 a DHCP address (temporary, departs from spec):"
    echo "  sudo DHCP_BR0=yes sh $0 $IFACE"
    echo "──────────────────────────────────────────────────────────────────"
    exit 1
fi

if [ -n "$WIFI_IP" ]; then
    echo "WiFi is up at $WIFI_IP — SSH survives on wlan0."
elif [ "$DHCP_BR0" = "yes" ]; then
    echo "DHCP_BR0=yes — br0 will request an IP; valseir stays reachable."
fi

echo "Will bridge $IFACE into br0."
echo "Proceeding in 5 seconds... (Ctrl-C to abort)"
sleep 5

# ── Remove the old systemd-networkd files if present ──────────────────────
for f in 10-br0.netdev "20-br0-${IFACE}.network" 30-br0.network; do
    if [ -f "/etc/systemd/network/$f" ]; then
        rm -f "/etc/systemd/network/$f"
        echo "removed /etc/systemd/network/$f"
    fi
done

# ── Create the bridge with NetworkManager ─────────────────────────────────

# 1. Remove any existing br0 connection.
nmcli con del br0 2>/dev/null || true
nmcli con del "br0-${IFACE}" 2>/dev/null || true
nmcli con del "br0-slave-${IFACE}" 2>/dev/null || true

# 2. Create the bridge device.
if [ "$DHCP_BR0" = "yes" ]; then
    nmcli con add type bridge ifname br0 con-name br0 \
        bridge.stp no \
        ipv4.method auto \
        ipv6.method ignore
else
    # 0.0.0.0 — no IP on br0, as per spec §3.
    nmcli con add type bridge ifname br0 con-name br0 \
        bridge.stp no \
        ipv4.method disabled \
        ipv6.method ignore
fi

# 3. Enslave IFACE to br0.
nmcli con add type bridge-slave ifname "$IFACE" master br0 con-name "br0-slave-${IFACE}"

# 4. Disable the existing IFACE connection so NetworkManager hands it to br0.
#    Find the connection by interface.
OLD_CON=$(nmcli -t -f NAME,DEVICE con show --active 2>/dev/null \
    | awk -F: "\$2==\"$IFACE\"{print \$1}" | head -1)
if [ -n "$OLD_CON" ] && [ "$OLD_CON" != "br0-slave-${IFACE}" ]; then
    nmcli con down "$OLD_CON" 2>/dev/null || true
    # Set it to not autoconnect so it does not fight br0 on reboot.
    nmcli con mod "$OLD_CON" connection.autoconnect no 2>/dev/null || true
    echo "disabled existing connection: $OLD_CON"
fi

# 5. Bring up the bridge.
nmcli con up "br0-slave-${IFACE}"
nmcli con up br0

sleep 3

# ── Receipt ────────────────────────────────────────────────────────────────
{
    echo "node: $(hostname)"
    date -u +%Y-%m-%dT%H:%M:%S+00:00
    tr -d '\0' < /proc/device-tree/model 2>/dev/null; echo
    echo "== NetworkManager connections =="
    nmcli -t -f NAME,TYPE,DEVICE,STATE con show 2>/dev/null
    echo "== br0 =="
    ip addr show br0 2>/dev/null || echo "(br0 not found)"
    echo "== bridge members =="
    ip link show master br0 2>/dev/null || echo "(no members yet)"
    echo "== eth0 =="
    ip addr show "$IFACE" 2>/dev/null
    echo "== wlan0 =="
    ip addr show wlan0 2>/dev/null || echo "(no wlan0)"
    echo "== capture test (5 packets on br0) =="
    timeout 5 tcpdump -i br0 -c 5 -nn 2>&1 || true
} | tee "$OUT"

echo ""
echo "Receipt: $(pwd)/$OUT"
echo "Copy to fleet/valseir/ on the operator machine."
if [ "$DHCP_BR0" = "yes" ]; then
    echo ""
    echo "br0 has a DHCP address (temporary). Once WiFi is confirmed as the"
    echo "management path, remove it:"
    echo "  nmcli con mod br0 ipv4.method disabled ipv6.method ignore"
    echo "  nmcli con up br0"
fi
echo ""
echo "Capture on br0:  tcpdump -i br0 -nn"
echo "Tshark on br0:   tshark -i br0"

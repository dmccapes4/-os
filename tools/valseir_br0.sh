#!/bin/sh
# valseir_br0.sh — set up br0 on valseir for traffic capture (ὄνυχος §3).
#
# Run ON valseir, as root, over SSH:
#   sudo sh valseir_br0.sh [IFACE]
#
# IFACE defaults to eth0. Override if Kali named it differently:
#   sudo sh valseir_br0.sh enp2s0
#
# ── READ THIS BEFORE RUNNING ──────────────────────────────────────────────
# This script enslaves IFACE to br0, which removes the IP from IFACE and
# puts it on br0. If IFACE is your only path to valseir (the path your SSH
# session is using) you will lose the connection part-way through.
#
# Safe to run when either:
#   a. SSH is on wlan0 (WiFi), not on IFACE, or
#   b. You are on the serial console (minicom/picocom), or
#   c. You accept that the session will drop and reconnect to br0's IP.
#
# After running, SSH reconnects the same way:
#   ssh kali@kali-raspberrypi.local   (avahi keeps the name)
#
# §3 item 4 of the spec asks whether br0 should have a second member so
# that lirvol's traffic to external addresses passes through valseir.
# This script creates a single-member bridge (eth0 → br0, capture-only).
# To add a second member later:
#   ip link set <second-iface> master br0
#   # and add a second .network file for it in /etc/systemd/network/
#
# Running this script again is harmless; existing config files are only
# written if they do not already exist.

set -eu

IFACE="${1:-eth0}"
OUT="valseir-br0-$(date +%Y%m%d).txt"

[ "$(id -u)" = 0 ] || { echo "run as root: sudo sh $0 [IFACE]" >&2; exit 2; }
[ "$(dpkg --print-architecture)" = arm64 ] || { echo "not arm64; this is for valseir" >&2; exit 1; }

# Confirm the interface exists before touching anything.
ip link show "$IFACE" > /dev/null 2>&1 || {
    echo "interface $IFACE not found. Run: ip link" >&2
    echo "Available interfaces:" >&2
    ip -o link show | awk -F': ' '{print "  " $2}' >&2
    exit 1
}

echo "Will bridge $IFACE into br0 (capture, no IP on br0)."
echo "If $IFACE carries your SSH session it will drop. Ctrl-C now to abort."
echo "Proceeding in 5 seconds..."
sleep 5

# ── systemd-networkd files ─────────────────────────────────────────────────

NETDIR=/etc/systemd/network

# br0 netdev (the bridge itself).
if [ ! -f "$NETDIR/10-br0.netdev" ]; then
    cat > "$NETDIR/10-br0.netdev" << 'EOF'
[NetDev]
Name=br0
Kind=bridge
EOF
    echo "wrote $NETDIR/10-br0.netdev"
else
    echo "$NETDIR/10-br0.netdev already exists"
fi

# Enslave IFACE to br0.
MEMBER_FILE="$NETDIR/20-br0-${IFACE}.network"
if [ ! -f "$MEMBER_FILE" ]; then
    cat > "$MEMBER_FILE" << EOF
[Match]
Name=$IFACE

[Network]
Bridge=br0
EOF
    echo "wrote $MEMBER_FILE"
else
    echo "$MEMBER_FILE already exists"
fi

# br0 itself: no IP (capture-only, as per spec §3).
if [ ! -f "$NETDIR/30-br0.network" ]; then
    cat > "$NETDIR/30-br0.network" << 'EOF'
[Match]
Name=br0

[Network]
LinkLocalAddressing=no
DHCP=no
EOF
    echo "wrote $NETDIR/30-br0.network"
else
    echo "$NETDIR/30-br0.network already exists"
fi

# ── Enable and apply ───────────────────────────────────────────────────────
systemctl enable systemd-networkd
systemctl restart systemd-networkd

# Give networkd a moment to bring up br0.
sleep 3

# ── Verify ────────────────────────────────────────────────────────────────
{
    echo "node: $(hostname)"
    date -u +%Y-%m-%dT%H:%M:%S+00:00
    echo "== bridges =="
    ip link show type bridge 2>/dev/null || echo "(ip link type bridge failed)"
    echo "== br0 details =="
    ip addr show br0 2>/dev/null || echo "(br0 not up yet — may need reboot)"
    echo "== bridge members =="
    bridge link show 2>/dev/null || ip link show master br0 2>/dev/null || echo "(check with: ip link show master br0)"
    echo "== capture test (5 packets on br0, then Ctrl-C if it hangs) =="
    timeout 5 tcpdump -i br0 -c 5 -nn 2>&1 || true
} | tee "$OUT"

echo ""
echo "Receipt: $(pwd)/$OUT"
echo "Copy to fleet/valseir/ on the operator machine."
echo ""
echo "Capture on br0:  tcpdump -i br0 -nn"
echo "Tshark on br0:   tshark -i br0"
echo "If br0 is not up after checking the receipt, reboot valseir."

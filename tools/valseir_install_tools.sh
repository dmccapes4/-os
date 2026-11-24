#!/bin/sh
# valseir_install_tools.sh: install the tools named for onychos on valseir
# (Kali, arm64) and write the receipt for game plan step O1.
#
# Run ON valseir, as root:   sudo sh valseir_install_tools.sh
#
# Installs only the list in docs/ONYCHOS_TECHNICAL_SPECIFICATION_v0.0.1.md
# section 4. Add package names to EXTRA to install more:
#   sudo EXTRA="bridge-utils tshark" sh valseir_install_tools.sh
# Running it again is harmless. The receipt is printed and also saved in the
# directory the script is run from; it belongs in -os/fleet/valseir/.
set -eu

TOOLS="tcpdump wireshark mitmproxy minicom picocom screen openocd telnet gdb-multiarch"
EXTRA="${EXTRA:-}"
OUT="valseir-tools-$(date +%Y%m%d).txt"

[ "$(id -u)" = 0 ] || { echo "run as root: sudo sh $0" >&2; exit 2; }
[ "$(dpkg --print-architecture)" = arm64 ] || { echo "not arm64: this is for valseir" >&2; exit 1; }

export DEBIAN_FRONTEND=noninteractive
apt-get update
# shellcheck disable=SC2086
apt-get install -y $TOOLS $EXTRA

{
    echo "node: $(hostname)"
    date -u +%Y-%m-%dT%H:%M:%S+00:00
    tr -d '\0' < /proc/device-tree/model 2>/dev/null; echo
    grep -E '^(PRETTY_NAME|VERSION)=' /etc/os-release
    uname -r
    echo "== apt sources =="
    grep -rhE '^(deb|URIs|Suites|Components)' /etc/apt/sources.list /etc/apt/sources.list.d 2>/dev/null || true
    echo "== package versions =="
    # shellcheck disable=SC2086
    dpkg-query -W -f='${Package} ${Version} ${db:Status-Abbrev}\n' $TOOLS $EXTRA
} | tee "$OUT"

echo "receipt: $(pwd)/$OUT" >&2

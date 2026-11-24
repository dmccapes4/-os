#!/bin/sh
# valseir_provision_onyx.sh — provision valseir for ὄνυχος (onyx).
#
# Run ON valseir, as root, over SSH:
#   sudo sh valseir_provision_onyx.sh
#
# Prerequisites (already done):
#   valseir_sd_step1.sh    — SSH, text console, tty1 autologin as kali
#   valseir_install_tools.sh — tcpdump, wireshark, etc.
#
# What this script does:
#   1. Installs tshark (separate from wireshark) and avahi-daemon.
#   2. Adds kali to the wireshark group so tshark runs without root.
#   3. Masks sleep, suspend, hibernate — power-always-on.
#   4. Adds consoleblank=0 to /boot/firmware/cmdline.txt — display never blanks.
#   5. Creates /var/lib/onyx/ and seeds an empty reports.jsonl.
#   6. Installs onyx-watch to /usr/local/bin/.
#   7. Hooks onyx-watch into kali's tty1 autologin (bash and zsh).
#   8. Enables and starts avahi-daemon (kali-raspberrypi.local on the LAN).
#   9. Writes a receipt.
#
# Bridge (br0) is NOT set up here. It is a separate script: valseir_br0.sh.
# Run that only after confirming SSH is reachable by a path that does not go
# through eth0 (spec §3 item 4 is undecided; the bridge risks killing SSH if
# eth0 is your only path back).
#
# Running this script again is harmless. Steps are guarded to skip what is
# already done.

set -eu

OUT="valseir-onyx-$(date +%Y%m%d).txt"

[ "$(id -u)" = 0 ] || { echo "run as root: sudo sh $0" >&2; exit 2; }
[ "$(dpkg --print-architecture)" = arm64 ] || { echo "not arm64; this is for valseir" >&2; exit 1; }

# ── 1. Install tshark and avahi ────────────────────────────────────────────
export DEBIAN_FRONTEND=noninteractive
apt-get update -qq

# Allow tshark to be installed non-interactively with the wireshark group
# allowed to capture (equivalent to answering Yes in dpkg-reconfigure).
echo "wireshark wireshark/install-setuid boolean true" \
    | debconf-set-selections
apt-get install -y tshark avahi-daemon

# ── 2. kali in the wireshark group ────────────────────────────────────────
if ! id -nG kali | tr ' ' '\n' | grep -qx wireshark; then
    usermod -aG wireshark kali
    echo "added kali to wireshark group (re-login or newgrp wireshark to activate)"
else
    echo "kali already in wireshark group"
fi

# ── 3. Never sleep, never suspend ─────────────────────────────────────────
for target in sleep.target suspend.target hibernate.target hybrid-sleep.target; do
    systemctl mask "$target" 2>/dev/null || true
done

# ── 4. Display never blanks ───────────────────────────────────────────────
CMDLINE=/boot/firmware/cmdline.txt
if [ -f "$CMDLINE" ]; then
    if ! grep -q 'consoleblank=0' "$CMDLINE"; then
        # cmdline.txt is a single line; append the parameter.
        sed -i 's/$/ consoleblank=0/' "$CMDLINE"
        echo "consoleblank=0 added to $CMDLINE"
    else
        echo "consoleblank=0 already in $CMDLINE"
    fi
else
    echo "WARNING: $CMDLINE not found; set consoleblank=0 manually in the kernel command line" >&2
fi

# ── 5. Report directory and seed file ─────────────────────────────────────
mkdir -p /var/lib/onyx
if [ ! -f /var/lib/onyx/reports.jsonl ]; then
    # Schema line matching the proposal in §6.1 of the spec.
    printf '{"schema":"onyx-reports-v0.0.1","fields":{"report":"label","noted":"the receipt, standing without the source","source":"record|network|debug|serial","problem":"(only if something is wrong)","seen":"(optional) where it was when noted","closed":"(last line of a report) iso8601","evict":"(optional) doc ids suggested for eviction","scores":"(optional) scored connascence for kyrios edges"},"update":{"what":"schema","fields":"..."}}\n' \
        > /var/lib/onyx/reports.jsonl
    echo "seeded /var/lib/onyx/reports.jsonl"
else
    echo "/var/lib/onyx/reports.jsonl already exists"
fi
chown -R root:kali /var/lib/onyx
chmod 775 /var/lib/onyx
chmod 664 /var/lib/onyx/reports.jsonl

# ── 6. Install onyx-watch ─────────────────────────────────────────────────
cat > /usr/local/bin/onyx-watch << 'WATCHEOF'
#!/bin/sh
# onyx-watch — show the last report from /var/lib/onyx/reports.jsonl.
# Refreshes every ONYX_INTERVAL seconds (default 60).
# Meant for tty1; SSH sessions run a normal shell.
#
# Display format: one line per noted receipt in the last closed report.
# Problem lines are flagged with ⚠.  If no closed report exists, the last
# 20 raw lines of the file are shown so something is always on screen.

REPORTS="${ONYX_REPORTS:-/var/lib/onyx/reports.jsonl}"
INTERVAL="${ONYX_INTERVAL:-60}"

render() {
    printf '\033[2J\033[H'   # clear screen, home cursor
    printf '═══ ὄνυχος  %s ═══\n\n' "$(date '+%Y-%m-%d %H:%M:%S')"

    if [ ! -s "$REPORTS" ]; then
        printf '  (no reports yet)\n'
        return
    fi

    # Python 3 is on Kali; use it for JSON parsing.
    python3 - "$REPORTS" << 'PYEOF'
import sys, json

path = sys.argv[1]
objects = []
try:
    with open(path) as f:
        for raw in f:
            raw = raw.strip()
            if not raw:
                continue
            try:
                objects.append(json.loads(raw))
            except json.JSONDecodeError:
                pass
except OSError as e:
    print(f"  (cannot read {path}: {e})")
    sys.exit(0)

# Skip the schema line (no "report" key).
records = [o for o in objects if "report" in o]

if not records:
    # Nothing structured yet; show raw tail.
    try:
        with open(path) as f:
            lines = f.readlines()
        for line in lines[-20:]:
            print(" ", line.rstrip())
    except OSError:
        pass
    sys.exit(0)

# Find the label of the last closed report.
last_label = None
for obj in reversed(records):
    if "closed" in obj:
        last_label = obj.get("report")
        break

# If no closed report, show the most recent open one.
if last_label is None:
    last_label = records[-1].get("report")

block = [o for o in records if o.get("report") == last_label]

for obj in block:
    noted   = obj.get("noted", "")
    source  = obj.get("source", "")
    problem = obj.get("problem", "")
    closed  = obj.get("closed", "")
    evict   = obj.get("evict", [])
    scores  = obj.get("scores", "")

    if noted:
        tag  = f"[{source}] " if source else ""
        flag = "  ⚠" if problem else ""
        print(f"  {tag}{noted}{flag}")
        if problem:
            print(f"       → {problem}")
    if closed:
        label_str = f"report {last_label}"
        print(f"\n  {label_str}  closed {closed}")
        if evict:
            print(f"  eviction suggested: {', '.join(str(e) for e in evict)}")
        if scores:
            print(f"  scores: {scores}")
PYEOF
}

# Only take over tty1; other terminals get a normal shell.
[ "$(tty)" = "/dev/tty1" ] || {
    echo "onyx-watch: not on tty1, exiting" >&2
    exit 0
}

# Main loop.
while true; do
    render
    sleep "$INTERVAL"
done
WATCHEOF

chmod 755 /usr/local/bin/onyx-watch

# ── 7. Hook into tty1 autologin (bash and zsh) ────────────────────────────
# The autologin.conf in valseir_sd_step1.sh logs in kali on tty1.
# A login shell on tty1 sources .bash_profile (bash) or .zprofile (zsh).
# exec replaces the shell with the watcher; SSH sessions are not affected.

HOOK='[ "$(tty)" = "/dev/tty1" ] && exec /usr/local/bin/onyx-watch'

for profile in /home/kali/.bash_profile /home/kali/.zprofile; do
    if grep -q 'onyx-watch' "$profile" 2>/dev/null; then
        echo "$profile: hook already present"
    else
        printf '\n# onyx: show the last report on the attached display\n%s\n' "$HOOK" \
            >> "$profile"
        chown kali:kali "$profile"
        echo "$profile: hook added"
    fi
done

# ── 8. avahi-daemon (kali-raspberrypi.local on the LAN) ───────────────────
systemctl enable avahi-daemon
systemctl start  avahi-daemon || systemctl restart avahi-daemon

# ── 9. Receipt ────────────────────────────────────────────────────────────
{
    echo "node: $(hostname)"
    date -u +%Y-%m-%dT%H:%M:%S+00:00
    tr -d '\0' < /proc/device-tree/model 2>/dev/null; echo
    grep -E '^(PRETTY_NAME|VERSION)=' /etc/os-release
    uname -r
    echo "== sleep targets =="
    for t in sleep.target suspend.target hibernate.target hybrid-sleep.target; do
        printf '%s: %s\n' "$t" "$(systemctl is-enabled "$t" 2>/dev/null || echo masked)"
    done
    echo "== cmdline consoleblank =="
    grep -o 'consoleblank=[^ ]*' /boot/firmware/cmdline.txt 2>/dev/null || echo "(not found)"
    echo "== package versions =="
    dpkg-query -W -f='${Package} ${Version} ${db:Status-Abbrev}\n' tshark avahi-daemon
    echo "== wireshark group members =="
    getent group wireshark
    echo "== avahi =="
    systemctl is-active avahi-daemon
    echo "== reports.jsonl =="
    wc -l /var/lib/onyx/reports.jsonl
    echo "== onyx-watch =="
    ls -l /usr/local/bin/onyx-watch
} | tee "$OUT"

echo ""
echo "Receipt: $(pwd)/$OUT"
echo "Copy to fleet/valseir/ on the operator machine."
echo ""
echo "Reboot for consoleblank=0 to take effect."
echo "After reboot the display on tty1 shows the last report."
echo "SSH: ssh kali@kali-raspberrypi.local"
echo ""
echo "Bridge (br0) is not set up yet. Run valseir_br0.sh when the"
echo "network topology is confirmed (spec §3 item 4)."

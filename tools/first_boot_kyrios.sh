#!/bin/sh
# first_boot_kyrios.sh — run ONCE on lirvol at first boot (as root).
# Sets up the NVMe ledger, runs the hello-kyrios processor, and writes a
# status receipt to the boot partition so the operator can read it without SSH.
set -eu

KYRIOS_ROOT="${KYRIOS_ROOT:-/opt/kyrios}"
LEDGER_DIR="/var/lib/kyrios/ledger"
LEDGER="$LEDGER_DIR/active.jsonl"
PROCESSOR="$KYRIOS_ROOT/processors/hello-kyrios.sh"
SPECS_SCRIPT="/boot/firmware/first_boot_specs.sh"
STATUS_NAME="kyrios-first-boot-status.txt"

log() { echo "first_boot_kyrios: $*" >&2; }

# 1. Collect hardware specs to the boot partition the way the catalog asked.
if [ -x "$SPECS_SCRIPT" ]; then
  sh "$SPECS_SCRIPT" || log "spec collection exited $?"
fi

# 2. Find NVMe. Prefer the first nvme block device.
NVME=""
for dev in /dev/nvme0n1 /dev/nvme1n1; do
  if [ -b "$dev" ]; then
    NVME="$dev"
    break
  fi
done

mkdir -p "$LEDGER_DIR"

if [ -n "$NVME" ]; then
  log "NVMe found: $NVME"
  # If blank/unpartitioned, create one ext4 partition for the ledger.
  if ! lsblk -no TYPE "$NVME" | grep -q "^part$"; then
    log "NVMe is blank; creating partition and ext4 filesystem"
    echo 'label: gpt' | sfdisk "$NVME" || true
    echo ',,L' | sfdisk "$NVME" || true
    sleep 1
    PART="${NVME}p1"
    [ -b "$PART" ] || PART="${NVME}1"
    mkfs.ext4 -L kyrios-ledger "$PART"
  fi
  PART="${NVME}p1"
  [ -b "$PART" ] || PART="${NVME}1"
  mount "$PART" "$LEDGER_DIR" || {
    log "failed to mount $PART; falling back to SD ledger"
  }
else
  log "no NVMe found; ledger lives on SD at $LEDGER_DIR"
fi

# 3. Seed the ledger with a root receipt.
if [ ! -f "$LEDGER" ]; then
  touch "$LEDGER"
fi

# 4. Run the v0.0.1 processor.
if [ -x "$PROCESSOR" ]; then
  KYRIOS_LEDGER="$LEDGER" KYRIOS_PROCESSOR_OUT="/var/lib/kyrios/processors" "$PROCESSOR" || log "processor exited $?"
else
  log "processor not found: $PROCESSOR"
fi

# 5. Write status to boot partition.
for d in /boot/firmware /boot; do
  if [ -d "$d" ] && touch "$d/.$STATUS_NAME.tmp" 2>/dev/null; then
    rm -f "$d/.$STATUS_NAME.tmp"
    STATUS="$d/$STATUS_NAME"
    break
  fi
done
STATUS="${STATUS:-./$STATUS_NAME}"

{
  echo "node: $(hostname)"
  echo "date: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
  echo "ledger: $LEDGER"
  echo "ledger_lines: $(wc -l < "$LEDGER")"
  echo "nvme: ${NVME:-none}"
  echo "processor_out:"
  cat /var/lib/kyrios/processors/hello-kyrios.out 2>/dev/null || true
} > "$STATUS"

log "wrote status to $STATUS"

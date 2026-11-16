#!/bin/sh
# hello-kyrios processor — v0.0.1 proof processor.
# Runs once, appends one receipt, exits.
set -eu

KYRIOS_ROOT="${KYRIOS_ROOT:-/opt/kyrios}"
LEDGER="${KYRIOS_LEDGER:-/var/lib/kyrios/ledger/active.jsonl}"
PROCESSOR="$KYRIOS_ROOT/processors/hello-kyrios.sh"
RECEIPT_TOOL="$KYRIOS_ROOT/ledger/receipt.py"
OUT_DIR="${KYRIOS_PROCESSOR_OUT:-/var/lib/kyrios/processors}"
OUT_FILE="$OUT_DIR/hello-kyrios.out"

die() { echo "hello-kyrios: $*" >&2; exit 1; }

[ -f "$PROCESSOR" ] || die "processor not found: $PROCESSOR"
[ -x "$RECEIPT_TOOL" ] || die "receipt tool not found: $RECEIPT_TOOL"
[ -d "$OUT_DIR" ] || mkdir -p "$OUT_DIR"

# Document hash of this wrapper.
DOCUMENT_HASH="$(sha256sum "$PROCESSOR" | awk '{print $1}')"
TIMESTAMP="$(date -u +%Y-%m-%dT%H:%M:%SZ)"
PAYLOAD="hello-kyrios executed on $(hostname) at $TIMESTAMP"
PAYLOAD_HASH="$(printf '%s' "$PAYLOAD" | sha256sum | awk '{print $1}')"

# Key fingerprint: the fleet SSH key fingerprint from the local host.
KEY_FILE="${KYRIOS_FLEET_KEY:-/home/kyrios/.ssh/authorized_keys}"
if [ -f "$KEY_FILE" ]; then
  KEY_FINGERPRINT="$(ssh-keygen -lf "$KEY_FILE" | awk '{print $2}' || echo '')"
else
  KEY_FINGERPRINT=""
fi

# Parent hash: hash of the most recent ledger line, or "root".
if [ -f "$LEDGER" ] && [ -s "$LEDGER" ]; then
  PARENT_HASH="$(sha256sum "$LEDGER" | awk '{print $1}')"
else
  PARENT_HASH="root"
fi

# Build the receipt.
RECEIPT="$(cat <<EOF
{
  "receipt_id": "",
  "version": "kyrios-trace-v0.0.1",
  "space": "hello-kyrios",
  "port": 7037,
  "key_fingerprint": "$KEY_FINGERPRINT",
  "action": "grant",
  "processor": "$PROCESSOR",
  "document_hash": "$DOCUMENT_HASH",
  "parent_hash": "$PARENT_HASH",
  "timestamp_utc": "$TIMESTAMP",
  "clew": [
    "-os/README.md",
    "-os/ledger/trace-format.md",
    "-os/ledger/receipt.py",
    "-os/spaces/hello-kyrios.md",
    "-os/processors/hello-kyrios.md"
  ],
  "eviction_status": null,
  "eviction_location": null,
  "payload_hash": "$PAYLOAD_HASH",
  "operator_decision": "operator-granted-build-v0.0.1",
  "notes": "first processor execution on lirvol"
}
EOF
)"

# Append to ledger.
RECEIPT_ID="$(echo "$RECEIPT" | "$RECEIPT_TOOL" --ledger "$LEDGER" --no-hash-check)"

# Write human-readable output.
{
  echo "node: $(hostname)"
  echo "receipt_id: $RECEIPT_ID"
  echo "payload: $PAYLOAD"
  echo "ledger: $LEDGER"
} > "$OUT_FILE"

echo "hello-kyrios: wrote receipt $RECEIPT_ID to $LEDGER"

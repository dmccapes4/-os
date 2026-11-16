#!/usr/bin/env python3
"""
ledger/receipt.py — Validate and append a receipt to the κύριος ledger.

Stdlib only. Runs on the bench or on a Pi. Never edits existing lines.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import sys
import uuid
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

REQUIRED_FIELDS = {
    "receipt_id": str,
    "version": str,
    "space": str,
    "port": int,
    "key_fingerprint": str,
    "action": str,
    "processor": str,
    "document_hash": str,
    "parent_hash": str,
    "timestamp_utc": str,
    "clew": list,
    "eviction_status": (str, type(None)),
    "eviction_location": (str, type(None)),
}

ALLOWED_ACTIONS = {"arrive", "grant", "refuse", "evict", "audit", "suggest"}


def _type_ok(value: Any, expected: type | tuple[type, ...]) -> bool:
    if isinstance(expected, tuple):
        return any(isinstance(value, t) for t in expected)
    return isinstance(value, expected)


def _sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(8192), b""):
            h.update(chunk)
    return h.hexdigest()


def _now() -> str:
    return datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def validate(receipt: dict[str, Any], check_hash: bool = True) -> None:
    """Validate a receipt dict. Raises ValueError on failure."""
    for field, expected in REQUIRED_FIELDS.items():
        if field not in receipt:
            raise ValueError(f"missing required field: {field}")
        if not _type_ok(receipt[field], expected):
            raise ValueError(f"field {field} has wrong type: {type(receipt[field]).__name__}")

    if receipt["action"] not in ALLOWED_ACTIONS:
        raise ValueError(f"invalid action: {receipt['action']}")

    try:
        int(receipt["port"])
    except (TypeError, ValueError):
        raise ValueError("port must be an integer")

    if check_hash and receipt["processor"]:
        proc_path = Path(receipt["processor"])
        if proc_path.is_file():
            actual = _sha256_file(proc_path)
            if actual != receipt["document_hash"]:
                raise ValueError(
                    f"document_hash mismatch for {proc_path}: "
                    f"expected {actual}, got {receipt['document_hash']}"
                )


def append_receipt(receipt: dict[str, Any], ledger_path: Path, check_hash: bool = True) -> str:
    """Validate and append one receipt. Returns the receipt_id."""
    if "receipt_id" not in receipt or not receipt["receipt_id"]:
        receipt["receipt_id"] = str(uuid.uuid4())
    if "timestamp_utc" not in receipt or not receipt["timestamp_utc"]:
        receipt["timestamp_utc"] = _now()

    validate(receipt, check_hash=check_hash)

    ledger_path.parent.mkdir(parents=True, exist_ok=True)
    line = json.dumps(receipt, ensure_ascii=False, sort_keys=True)
    with open(ledger_path, "a", encoding="utf-8") as f:
        f.write(line + "\n")
    return receipt["receipt_id"]


def main() -> int:
    parser = argparse.ArgumentParser(description="Append a κύριος receipt")
    parser.add_argument("--ledger", type=Path, default=Path("/var/lib/kyrios/ledger/active.jsonl"),
                        help="Path to the ledger file")
    parser.add_argument("--receipt", type=Path, default=None,
                        help="JSON file containing the receipt")
    parser.add_argument("--no-hash-check", action="store_true",
                        help="Skip processor document_hash verification")
    parser.add_argument("--dump-schema", action="store_true",
                        help="Print the required fields and exit")
    args = parser.parse_args()

    if args.dump_schema:
        print(json.dumps({k: str(v) for k, v in REQUIRED_FIELDS.items()}, indent=2))
        return 0

    if args.receipt:
        with open(args.receipt, "r", encoding="utf-8") as f:
            receipt = json.load(f)
    else:
        receipt = json.load(sys.stdin)

    try:
        rid = append_receipt(receipt, args.ledger, check_hash=not args.no_hash_check)
    except (ValueError, json.JSONDecodeError, OSError) as e:
        print(f"ERROR: {e}", file=sys.stderr)
        return 1

    print(rid)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

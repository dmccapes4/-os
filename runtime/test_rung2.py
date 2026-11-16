#!/usr/bin/env python3
"""
Rung 2 acceptance: a space accepts input at its TCP port, under a key,
inside a size — bench form. Run: -os/.venv/bin/python runtime/test_rung2.py

Cases: health, right key grants, wrong key refuses, oversize document
refuses, oversize frame refuses, quota refuses. Every arrival leaves the
receipt pair the schema predicts; the ledger validates line by line via
ledger/receipt.py.
"""
from __future__ import annotations

import hashlib
import json
import socket
import subprocess
import sys
import tempfile
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "ledger"))
import receipt as receipt_tool

FAILED = []


def check(label: str, cond: bool, detail: str = "") -> None:
    print(("PASS " if cond else "FAIL ") + label + (f" — {detail}" if detail else ""))
    if not cond:
        FAILED.append(label)


def send(port: int, obj: dict | bytes) -> dict:
    line = obj if isinstance(obj, bytes) else json.dumps(obj).encode()
    with socket.create_connection(("127.0.0.1", port), timeout=10) as s:
        s.sendall(line + b"\n")
        buf = b""
        while not buf.endswith(b"\n"):
            chunk = s.recv(65536)
            if not chunk:
                break
            buf += chunk
    return json.loads(buf)


def main() -> int:
    tmp = Path(tempfile.mkdtemp(prefix="kyrios-rung2-"))
    ledger = tmp / "ledger.jsonl"
    key_text = "rung2-test-key"
    key_fp = hashlib.sha256(key_text.encode()).hexdigest()
    cfg = {
        "name": "hello-kyrios", "port": 7037, "bind": "127.0.0.1",
        "max_doc_bytes": 64, "max_frame_bytes": 4096, "quota_docs": 2,
        "write_key_fingerprints": [key_fp],
    }
    cfg_path = tmp / "hello-kyrios.json"
    cfg_path.write_text(json.dumps(cfg))
    port = 17037

    proc = subprocess.Popen(
        [sys.executable, str(ROOT / "runtime/kyriosd.py"),
         "--space", str(cfg_path), "--ledger", str(ledger),
         "--port", str(port)],
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    try:
        for _ in range(50):
            try:
                r = send(port, {"type": "health"})
                break
            except OSError:
                time.sleep(0.2)
        else:
            raise RuntimeError("daemon did not come up")

        check("health answers", r.get("result") == "ok", str(r))
        r = send(port, {"type": "arrive", "key": key_text,
                        "payload": {"hello": "kyrios"}})
        check("right key grants", r.get("result") == "grant", str(r))
        r = send(port, {"type": "arrive", "key": "wrong-key",
                        "payload": {"hello": "intruder"}})
        check("wrong key refuses", r.get("result") == "refuse"
              and r.get("reason") == "key", str(r))
        r = send(port, {"type": "arrive", "key": key_text,
                        "payload": {"blob": "x" * 200}})
        check("oversize document refuses", r.get("result") == "refuse"
              and r.get("reason") == "size", str(r))
        big = json.dumps({"type": "arrive", "key": key_text,
                          "payload": "y" * 8000}).encode()
        r = send(port, big)
        check("oversize frame refuses", r.get("result") == "refuse"
              and r.get("reason") == "frame_size", str(r))
        r = send(port, {"type": "arrive", "key": key_text,
                        "payload": {"hello": "again"}})
        check("second grant within quota", r.get("result") == "grant", str(r))
        r = send(port, {"type": "arrive", "key": key_text,
                        "payload": {"hello": "third"}})
        check("quota refuses", r.get("result") == "refuse"
              and r.get("reason") == "quota", str(r))
    finally:
        proc.terminate()
        out, _ = proc.communicate(timeout=5)

    # Ledger: every line validates; actions alternate arrive->verdict;
    # the health probe left no receipts.
    lines = [ln for ln in ledger.read_bytes().splitlines() if ln.strip()]
    actions = []
    ok = True
    for ln in lines:
        rec = json.loads(ln)
        try:
            receipt_tool.validate(rec, check_hash=False)
        except ValueError as e:
            ok = False
            print(f"  invalid receipt: {e}")
        actions.append(rec["action"])
    check("every ledger line validates", ok, f"{len(lines)} lines")
    expected = ["arrive", "grant", "arrive", "refuse", "arrive", "refuse",
                "arrive", "refuse", "arrive", "grant", "arrive", "refuse"]
    check("receipt sequence is arrive->verdict, health silent",
          actions == expected, str(actions))
    # Chain: each receipt's parent is the hash of the previous line.
    chain_ok = json.loads(lines[0])["parent_hash"] == "root" and all(
        json.loads(lines[i])["parent_hash"]
        == hashlib.sha256(lines[i - 1]).hexdigest()
        for i in range(1, len(lines)))
    check("parent_hash chain unbroken", chain_ok)

    print(f"\nwitness dir: {tmp}")
    if FAILED:
        print("RUNG 2: FAIL", FAILED)
        return 1
    print("RUNG 2: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

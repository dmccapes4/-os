#!/usr/bin/env python3
"""
arrive.py — the owned client for a kyrios space port.

Usage:
  arrive.py --port 7037 --key-file <path> --payload '<json>'
  arrive.py --port 7037 --health

Sends exactly one line of JSON over TCP; prints the one-line reply.
The key file's contents (stripped) are the arriving key; only its
SHA-256 fingerprint ever reaches the ledger.
"""
from __future__ import annotations

import argparse
import json
import socket
import sys
from pathlib import Path


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser(description="arrive at a kyrios space port")
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--port", type=int, required=True)
    ap.add_argument("--key-file", type=Path, default=None)
    ap.add_argument("--payload", default="null", help="JSON payload text")
    ap.add_argument("--health", action="store_true")
    args = ap.parse_args(argv)

    if args.health:
        req: dict = {"type": "health"}
    else:
        key = args.key_file.read_text(encoding="utf-8").strip() \
            if args.key_file else ""
        req = {"type": "arrive", "key": key,
               "payload": json.loads(args.payload)}

    line = json.dumps(req, ensure_ascii=False).encode() + b"\n"
    with socket.create_connection((args.host, args.port), timeout=10) as s:
        s.sendall(line)
        reply = b""
        while not reply.endswith(b"\n"):
            chunk = s.recv(65536)
            if not chunk:
                break
            reply += chunk
    print(reply.decode("utf-8").strip())
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))

#!/usr/bin/env python3
"""
kyriosd — rung 2: one space, one TCP port, arrivals under a key.

Wire protocol (ports talk TCP; nothing here knows HTTP):
  request : one line of UTF-8 JSON, newline-terminated
            {"type": "health"}
            {"type": "arrive", "key": "<key text>", "payload": <any JSON>}
  response: one line of UTF-8 JSON
            {"result": "ok", ...}                 health
            {"result": "grant", "receipt_id": …}  arrival accepted
            {"result": "refuse", "reason": …}     arrival refused (receipted)

Every arrive attempt — granted or refused — appends an arrive receipt and a
verdict receipt to the ledger, chained by parent_hash (SHA-256 of the
previous ledger line). Frames larger than the space's frame limit are
refused and receipted, never crashed on.

Stdlib only, on purpose: the same file runs on the bench, on lirvol's
stock Bookworm, and later inside the Buildroot image.

Config: the space's JSON sidecar (spaces/<name>.json):
  name, port, bind, max_doc_bytes, max_frame_bytes, quota_docs,
  write_key_fingerprints: [sha256 hex of accepted key texts]
"""
from __future__ import annotations

import hashlib
import json
import socketserver
import sys
import threading
import uuid
from datetime import datetime, timezone
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "ledger"))
import receipt as receipt_tool  # ledger/receipt.py — validation lives there

VERSION = "kyrios-trace-v0.0.1"
PROCESSOR = "runtime/kyriosd.py"


def now_utc() -> str:
    return datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def sha256_hex(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


class Space:
    def __init__(self, config_path: Path, ledger_path: Path):
        cfg = json.loads(config_path.read_text(encoding="utf-8"))
        self.name: str = cfg["name"]
        self.port: int = int(cfg["port"])
        self.bind: str = cfg.get("bind", "127.0.0.1")
        self.max_doc_bytes: int = int(cfg["max_doc_bytes"])
        self.max_frame_bytes: int = int(cfg.get("max_frame_bytes", 1_000_000))
        self.quota_docs: int = int(cfg["quota_docs"])
        self.write_key_fps: set[str] = set(cfg.get("write_key_fingerprints", []))
        self.ledger_path = ledger_path
        self.lock = threading.Lock()
        self.doc_hash = sha256_hex(Path(__file__).read_bytes())
        self.granted = self._count_granted()

    def _ledger_lines(self) -> list[bytes]:
        if not self.ledger_path.is_file():
            return []
        return [ln for ln in self.ledger_path.read_bytes().splitlines() if ln.strip()]

    def _count_granted(self) -> int:
        n = 0
        for ln in self._ledger_lines():
            try:
                rec = json.loads(ln)
            except json.JSONDecodeError:
                continue
            if rec.get("space") == self.name and rec.get("action") == "grant":
                n += 1
        return n

    def _append(self, action: str, key_fp: str, notes: str,
                payload_hash: str | None = None) -> tuple[str, str]:
        """Append one receipt (assumes self.lock held). Returns (id, line_hash)."""
        lines = self._ledger_lines()
        parent = sha256_hex(lines[-1]) if lines else "root"
        rec: dict = {
            "receipt_id": str(uuid.uuid4()),
            "version": VERSION,
            "space": self.name,
            "port": self.port,
            "key_fingerprint": key_fp,
            "action": action,
            "processor": PROCESSOR,
            "document_hash": self.doc_hash,
            "parent_hash": parent,
            "timestamp_utc": now_utc(),
            "clew": ["-os/README.md", "-os/ledger/trace-format.md",
                     f"-os/spaces/{self.name}.md"],
            "eviction_status": None,
            "eviction_location": None,
            "notes": notes,
        }
        if payload_hash:
            rec["payload_hash"] = payload_hash
        receipt_tool.validate(rec, check_hash=False)
        line = json.dumps(rec, ensure_ascii=False, sort_keys=True).encode()
        self.ledger_path.parent.mkdir(parents=True, exist_ok=True)
        with open(self.ledger_path, "ab") as f:
            f.write(line + b"\n")
            f.flush()
        return rec["receipt_id"], sha256_hex(line)

    def arrive(self, key_fp: str, payload: bytes) -> dict:
        """One arrival. payload is the canonical JSON bytes of the payload."""
        payload_hash = sha256_hex(payload)
        with self.lock:
            self._append("arrive", key_fp, "arrival at port", payload_hash)
            if key_fp not in self.write_key_fps:
                rid, _ = self._append("refuse", key_fp, "key not in write set",
                                      payload_hash)
                return {"result": "refuse", "reason": "key", "receipt_id": rid}
            if len(payload) > self.max_doc_bytes:
                rid, _ = self._append(
                    "refuse", key_fp,
                    f"document {len(payload)}B over max_doc_bytes "
                    f"{self.max_doc_bytes}", payload_hash)
                return {"result": "refuse", "reason": "size",
                        "receipt_id": rid}
            if self.granted >= self.quota_docs:
                rid, _ = self._append(
                    "refuse", key_fp,
                    f"quota_docs {self.quota_docs} reached", payload_hash)
                return {"result": "refuse", "reason": "quota",
                        "receipt_id": rid}
            rid, _ = self._append("grant", key_fp, "arrival granted",
                                  payload_hash)
            self.granted += 1
            return {"result": "grant", "receipt_id": rid}


class Handler(socketserver.StreamRequestHandler):
    space: Space  # set on server

    def handle(self) -> None:
        space = self.server.space  # type: ignore[attr-defined]
        # Read one frame, enforcing the frame limit while reading so an
        # oversize sender is refused rather than buffered without bound.
        frame = self.rfile.readline(space.max_frame_bytes + 2)
        if not frame:
            return
        if len(frame) > space.max_frame_bytes:
            key_fp = ""
            with space.lock:
                rid, _ = space._append(
                    "arrive", key_fp, "oversize frame arrival")
                rid, _ = space._append(
                    "refuse", key_fp,
                    f"frame over max_frame_bytes {space.max_frame_bytes}")
            self._reply({"result": "refuse", "reason": "frame_size",
                         "receipt_id": rid})
            return
        try:
            req = json.loads(frame.decode("utf-8"))
        except (UnicodeDecodeError, json.JSONDecodeError):
            self._reply({"result": "refuse", "reason": "malformed"})
            return
        rtype = req.get("type")
        if rtype == "health":
            self._reply({"result": "ok", "space": space.name,
                         "port": space.port, "granted": space.granted})
            return
        if rtype != "arrive":
            self._reply({"result": "refuse", "reason": "unknown_type"})
            return
        key_text = str(req.get("key", ""))
        key_fp = sha256_hex(key_text.encode()) if key_text else ""
        payload = json.dumps(req.get("payload"),
                             ensure_ascii=False,
                             sort_keys=True).encode()
        self._reply(space.arrive(key_fp, payload))

    def _reply(self, obj: dict) -> None:
        self.wfile.write(json.dumps(obj, ensure_ascii=False).encode() + b"\n")
        self.wfile.flush()


class Server(socketserver.ThreadingTCPServer):
    daemon_threads = True
    allow_reuse_address = True
    space: Space


def main(argv: list[str]) -> int:
    import argparse
    ap = argparse.ArgumentParser(description="kyriosd — one space, one port")
    ap.add_argument("--space", type=Path, required=True,
                    help="space JSON sidecar")
    ap.add_argument("--ledger", type=Path, required=True)
    ap.add_argument("--port", type=int, default=None,
                    help="override the sidecar port (tests)")
    args = ap.parse_args(argv)

    space = Space(args.space, args.ledger)
    if args.port is not None:
        space.port = args.port
    srv = Server((space.bind, space.port), Handler)
    srv.space = space
    print(f"kyriosd: space {space.name} on {space.bind}:{space.port} "
          f"(ledger {space.ledger_path})", flush=True)
    try:
        srv.serve_forever()
    except KeyboardInterrupt:
        pass
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))

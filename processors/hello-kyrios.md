# Processor — `hello-kyrios`

**Type:** shell-wrapped Velora skeleton  
**Space:** `hello-kyrios` (port `7037`)  
**Runtime:** `processors/hello-kyrios.sh`  
**Output:** one receipt appended to `/var/lib/kyrios/ledger/active.jsonl`

## What it does

1. Verifies it can read the ledger directory.
2. Computes its own SHA-256 hash.
3. Builds a receipt describing an `arrive` → `grant` event at the `hello-kyrios` space.
4. Appends the receipt via `ledger/receipt.py`.
5. Writes a short status file to `/var/lib/kyiros/processors/hello-kyrios.out`.

## Properties

- **Deterministic.** Given the same processor file and the same ledger state, the action is the same.
- **Bounded.** It runs once, writes one receipt, and exits. No loop.
- **Documented.** The processor itself is on the shelf next to what it produces.

## Why this shape

v0.0.1 must prove that κύριος can run a processor as a document and leave a trace. It must not reproduce the `full_metal_packet.py` trap: open-ended, HITL-driven, debug loops. `hello-kyronos` is intentionally trivial so the trace is the only interesting output.

## Clew

- `-os/README.md`
- `-os/ledger/trace-format.md`
- `-os/ledger/receipt.py`
- `-os/spaces/hello-kyrios.md`

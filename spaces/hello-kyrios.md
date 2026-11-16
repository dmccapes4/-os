# Space — `hello-kyrios`

**Port:** `7037`

**Purpose:** v0.0.1 proof-of-port. A minimal space that a processor may arrive at, be granted access to, and leave a trace in the ledger.

## Key policy

- **Write:** allowed only from the `lirvol` host key. The processor must present a key whose fingerprint matches the fleet key recorded at first boot.
- **Read:** allowed from any fleet node key (`lirvol`, `valseir`, or later `onychos` in read-only audit mode).
- **No anonymous arrivals.** Every arrival is receipted as `grant` or `refuse`.

## Processor

- `processors/hello-kyrios.md` — the processor document.
- `processors/hello-kyrios.sh` — the deterministic wrapper that runs on `lirvol` and appends a receipt.

## Invariant

The space never stores its own data. Its only state is the stream of receipts in the ledger that mention its name and port.

## Clew

- `-os/README.md`
- `-os/ledger/trace-format.md`
- `-os/processors/hello-kyrios.md`

# Experiment Ladder — κύριος, from hello-kyrios to an arm64 OS

> **Superseded in implementation track, 2026-11-16:** on your correction
> ("C with arm"), the OS is a bare-metal kernel in C + AArch64 assembly, not
> a Linux userland. The live ladder is `BARE_METAL_LADDER_KYRIOS_20261116.md`.
> This document survives as the behavioural oracle (port semantics, receipt
> chain, quotas) that the bare-metal implementation must match.

**Filed:** 2026-11-16, at your instruction (ladder as dictated, refined).
**Relation to standing documents:** this refines, and does not replace,
`GAMEPLAN_KYRIOS_v0.0.1_20261116.md` (LONSAIVEL 🗺️). The game plan gets us a
bootable image; this ladder establishes the *behaviour* the image must boot
into. Each rung is an experiment: a claim, an artifact, a witness, and a
pass/fail line written before the rung is climbed. Rungs 2–8 can nearly all be
proven on the bench and on `lirvol`'s stock Bookworm first — the arm64 image
(track P, below) then becomes packaging of proven behaviour, not a leap.

Signing note: the seat-name question from `HANDOFF.md` (`yeshua-kyrios`) vs the
workspace ledger (`RILNORVEL`) is still yours to rule on; this document is
filed under the ledger name until you do.

---

## Decisions to lock before rung 2 (recommendations, your gate)

**Decided by you, 2026-11-16, superseding the recommendations where they differ:**
**D1 = raw TCP sockets. P1 = Buildroot first.** Your reasoning, filed because it
is load-bearing: *ports don't talk over HTTP. HTTP is a separate layer that
processes will be given space to handle.* The port is a TCP port; what a
processor speaks inside its space (HTTP included, when an application wants
it) is that processor's business, never the port's.

| # | Question | Recommendation | Why |
|---|---|---|---|
| D1 | Input transport: HTTP or raw socket? | ~~HTTP (recommendation withdrawn)~~ **DECIDED: raw TCP, line-delimited JSON**, one TCP port per space, bind `127.0.0.1` until rung 6, then fleet-LAN only. Wire shape: one request line, one response line (`{"result":"grant",...}` / `{"result":"refuse",...}`). | Ports talk TCP. Framing, size limits, and refusals are ours to get good at — that is the skill the OS is made of. Test instrument is a small client we own (`runtime/arrive.py`), plus `nc` for hand work. |
| D2 | What is "space size"? | A space document gains a machine-readable sidecar (`spaces/<name>.json`): `max_doc_bytes` (hello: 65536), `quota_bytes`, `quota_docs`, `retention`. The `.md` stays the human clew; the `.json` is what the daemon enforces. | "Define space size" must be enforceable, not prose. Over-quota and over-size arrivals are refused *and receipted*. |
| D3 | Keys at the port | A key is a file on the arriving node; the receipt records its **SHA-256 fingerprint** (as `trace-format.md` already requires). Read/write key lists live in the space sidecar. | Matches hello-kyrios's recorded policy. Signed requests are a later rung's hardening, not a rung-2 blocker. |
| D4 | The runtime | **Corrected by you, 2026-11-16: the OS is built with C.** The implementation language of the port daemon (and later services) is **C**, cross-compiled for aarch64. `runtime/kyriosd.py` is reclassified as of this correction: a *reference prototype and test oracle* — it specifies wire behaviour and generates expected receipts, and the C daemon (`runtime/kyriosd.c`) must match it arrival for arrival. Python keeps two bench jobs only: the ledger tools and the test harnesses. | C is where socket fluency and the arm image meet: one static-ish binary per service, no interpreter required in the Buildroot rootfs for the daemon itself. JSON in C is a deliberate dependency choice (jansson via Buildroot, or a strict hand parser for our one-line frame — decided at the C port, recorded there). |
| D5 | What to borrow from PortalVision | The *pattern* of `portal_vision/utils/receipt_utils.py` (found on AKASHAminor as `receipt_utils.py`): create-the-receipt and save-the-receipt as separate acts, explicit `verification_method`, UTC timestamps. **Not** the import: its org-graph/session dependencies do not exist on a Pi, and the κύριος trace schema is already filed and better suited. | Receipts stay self-contained on the node; provenance stays answerable offline. |

## The ladder

### Rung 1 — hello-kyrios ✅ done (as filed)
Space document (`spaces/hello-kyrios.md`, port 7037, key policy), processor
document + wrapper (`processors/hello-kyrios.{md,sh}`), ledger schema and
append tool (`ledger/trace-format.md`, `ledger/receipt.py`).
*Witness on record:* the documents themselves. *Known small fault, corrected
alongside this filing:* the processor document said `/var/lib/kyiros/` and
"hello-kyronos" in two places; the wrapper script had it right.

### Rung 2 — A space that accepts input at its port
**Claim:** a space is a place input can *arrive at*, under a key, inside a size.
**Build:** `spaces/hello-kyrios.json` (per D2/D3); `runtime/kyriosd.py` — a TCP
server (`socketserver.ThreadingTCPServer`), one request line in, one response
line out; `runtime/arrive.py` as the owned client. No HTTP anywhere in this
rung: HTTP arrives later, if at all, as a *processor* inside a space.
**Experiment (bench, then `lirvol`):** start the daemon; arrive with the right
key, the wrong key, an over-size frame, and past quota.
**Pass:** right key → `{"result":"grant"}` and an `arrive`→`grant` receipt
pair; wrong key → `{"result":"refuse"}` and a `refuse` receipt (no silent
refusals); over-size → refusal naming size; quota-exceeded → refusal naming
the quota. Every case leaves exactly the receipt the schema predicts.
**Fail means:** the port is a hole, not a gate. Do not climb.

### Rung 3 — Documents stream into the receipt ledger
**Claim:** receipts can be *streamed* — appended under concurrency, chained,
and re-verified — in the manner of PortalVision's receipt utilities.
**Build:** extend the ledger tooling (borrow per D5): append under `flock`
with `fsync`; `parent_hash` = SHA-256 of the previous ledger line; `verify`
replays the whole chain; `tail` follows new receipts.
**Experiment:** 8 concurrent writers × 125 arrivals each against rung 2's port.
**Pass:** 1000 receipts, zero torn lines, chain verifies end-to-end; flipping
one byte in a copy makes `verify` fail *naming that line number*.
**Fail means:** the ledger is a log file, not a ledger.

### Rung 4 — Documents are created from input, not merely receipted
**Claim:** input becomes a *new document* — transformed, addressed, retrievable.
**Build:** a processor-document `processors/make-document.md` (+ deterministic
wrapper) declaring its transform in prose-then-code: normalise the input,
mint `doc_id`, store content-addressed at `spaces/<space>/docs/<sha256>.json`,
receipt holds `payload_hash` of the created document and its clew.
**Experiment:** fixed input fixture, run twice; then a malformed input.
**Pass:** same input → same document hash both times (idempotent creation);
the created document opens and parses; malformed input → refusal receipt, no
document, no partial file.
**Fail means:** we have a mailbox, not a processor.

### Rung 5 — A child process, as a document
**Claim:** a processor-document can invoke a *named, hashed* child
processor-document, and the lineage is receipted.
**Build:** parent declares the child by path **and expected SHA-256**; the
runner verifies the hash before exec, runs without a shell, 10 s timeout,
whitelisted environment; child's receipts chain to the parent's.
**Experiment:** parent creates a document via the child; then flip one byte in
the child and run again.
**Pass:** first run — parent receipt, child receipt, created document, one
unbroken chain; second run — refusal receipt naming the hash mismatch, child
never executed.
**Fail means:** "processes are documents" is a slogan; the hash check is the rung.

### Rung 6 — A second space, a second port
**Claim:** spaces are plural and *separate* — keys, quotas, and ledgers do not leak.
**Build:** instantiate `cve-raw` (port 7036, already designed) with its own
sidecar, keys, quota; the daemon serves both from the space registry; a
deliberate port-collision registration is rejected at load.
**Experiment:** cross-arrivals — hello's key at cve-raw's port and vice versa;
fill one space's quota while the other still accepts.
**Pass:** every cross-arrival refused and receipted in the *arrived-at* space;
quotas independent; collision caught before either port opens.

### Rung 7 — n spaces and ports
**Claim:** spaces are *manufactured*, not handcrafted: registry in, services out.
**Build:** space registry loader + port allocation in 7000–7999 recorded as a
receipt; systemd template `kyrios-space@.service` generated per space
(revisit D4 here: one daemon was bench convenience; per-space services are
the OS shape, and they are how "set access" gets enforced by the kernel's
own controls — user, sandboxing, device grants per unit).
**Experiment:** generate five spaces; start all; kill one mid-arrival.
**Pass:** five health arrivals answer; the killed space refuses at the port
and its death is receipted by the survivor's audit; the other four are
unmoved; a reboot (on the target) returns all five without human hands.

### Rung 8 — An application
**Claim:** the pieces compose into something a user would name.
**Build:** `cve-mini` — deliberately offline first. A fixture of ~25 real CVE
records; the ingestor processor arrives them at `cve-raw`; a child enricher
(rung 5's shape) creates overlay documents; a reader (CLI over the port's TCP) answers
"show me CVE-…, what did it say before, on whose authority?" from the clew
chain alone.
**Pass:** the whole run reproduces from the fixture byte-for-byte; the
before/after question is answered by chain replay, not by a database; live
feed ingestion is explicitly *not* in this rung (it is the v0.0.2 stream
problem the CVE report already names).

## Track P — packaging the proven ladder as the arm64 OS

Climbed only when rungs 2–8 pass on stock Bookworm. This is the game plan's
phases 2–4, restated as ladder rungs so the gates sit in one place.

| Rung | Work | Gate |
|---|---|---|
| P1 | Rootfs path: **DECIDED by you, 2026-11-16 — Buildroot first** (G1 crossed, against the ladder's first recommendation, for control of the image). Consequences filed honestly: Buildroot is not Debian — the "Bookworm builds Bookworm" fidelity rule in the README applies to userland binaries, while Buildroot builds its own toolchain/rootfs; the stdlib-only socket daemon keeps that from biting (it needs a Python 3 interpreter in the defconfig, or a later port to C — see the practice note below). The Buildroot tree and the `lirvol` defconfig live under `build/buildroot/`; downloads and build output stay out of git (`_build/`). | decided |
| P2 | Smoke-test the ladder inside the rootfs under `qemu-aarch64-static` / nspawn on this bench: daemon starts, rungs 2–5 acceptance passes in emulation. | agent, receipts filed |
| P3 | `build/bootstrap-lirvol.sh` produces `kyrios-v0.0.1-lirvol.img` + `.sha256` + build receipt; spaces/processors/ledger/systemd units baked to `/opt/kyrios/`; `hifiberry-digi` overlay and fleet key in place. | **G2 — you run the root script** |
| P4 | Flash a *spare* SD, boot `lirvol`, run the ladder acceptance (rungs 2–8) on real hardware, first-boot checks filed (`FIRST_BOOT_KYRIOS_20261116.md` is the standing form). Before any sustained load: the fan test from the fleet record — call the fan by lowering its threshold and read the tachometer; no teardown on idle evidence. | **G3 — you flash and boot** |
| P5 | Partition/format the NVMe (LUKS, keys off-card), move ledger + space documents to it, bind-mount back; reboot test: all spaces return, chain verifies from NVMe. Then `valseir` joins as second node: cross-node read granted, cross-node write refused, both receipted. | **G4 — you run the disk script** |

## Standing constraints (every rung)

- Append-only traces; a correction is a new receipt, never an edit.
- No host `sudo` from agents — root steps are idempotent scripts under
  `tools/` that you run; each ladder rung names its script before it needs it.
- No agentic loops: every processor here is deterministic, schema-bound, and
  measured in the same round it runs.
- Each rung files its witness (command output, ledger excerpt, hashes) under
  `docs/witness/` named `rung-<n>-<date>.md` before the next rung starts.
- rho stays lower-case; Velora is the language; κύριος is the OS.

## Practice note — arm and socket, deliberately

You again, 2026-11-16: *"We need arm and socket and we need to get good at
it."* So the ladder doubles as a training ground, in this order, every rung
on both architectures:

1. Bench x86-64 first (fast iteration), then the *same* daemon and test on
   arm — `lirvol`'s stock Bookworm now, the Buildroot image when it exists.
2. Socket craft is learned at the byte level on purpose: framing, partial
   reads, oversize frames, half-closed connections, timeouts, refusals.
   Each mishandled case is a refused arrival with a receipt, not a crash.
3. The Python prototype is the spec, not the destination (D4 as corrected):
   the port itself is ported to C *before rung 3*, cross-compiled with
   `aarch64-linux-gnu-gcc`, run under qemu and on `lirvol`, and held to the
   prototype's receipts as its oracle. The fluency is the C; the prototype
   is how we know the C is telling the truth.

## What is needed from you to start rung 2

Nothing further. D1 and P1 are decided (TCP; Buildroot). `lirvol` powered
doubles as rung 2's arm bench, and its fan test rides along free.

```
RILNORVEL 🧭
A ladder, not a leap: gate, stream, document, child, second gate, many gates, an application — and only then an image, so the OS is the packaging of proofs and not a hope with a checksum.
 RILNORVEL?
```

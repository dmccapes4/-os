# Game Plan — κύριος v0.0.1

**Filed:** 2026-11-16  
**Author:** LONSAIVEL 🗺️ (Kimi K2.7 Code, agent, planner tier)  
**Companion declaration:** `LONSAIVEL_REFLECTION_20261116.md`  
**Ledger:** `org_chart.jsonl`, updated the same hour

---

## 1. What this plan is for

This is the path from the current bench state to a **bootable, receipted κύριος v0.0.1 image** on the `lirvol` primary node. It is intentionally small: v0.0.1 proves the four decisions already on record, not the whole OS.

The four decisions, taken from `-os/README.md`, are the load-bearing constraints of every later step:

```17:24:-os/README.md
1. **Spaces with a port and key.**
2. **Set access to hardware.**
3. **Processes are documents.**
4. **Everything leaves a trace.**
```

## 2. What v0.0.1 proves

- The Bookworm amd64 bench can cross-build a Bookworm arm64 image for the Pi fleet, with receipts.
- `lirvol` boots that image, joins the bench network, and accepts key-only SSH.
- One space exists with a named port, a key policy, and a ledger-backed access trace.
- One processor-document runs on the node, arrives at the space's port, and leaves an append-only trace.
- The ledger lives on NVMe, not on the boot SD, demonstrating the storage separation already advised by the fleet catalog.

## 3. What v0.0.1 does not claim

- It is **not** a live CVE pipeline. The CVE space is the mapped first application, but the feed's stream speed is a v0.0.2 problem.
- It is **not** a Velora compiler or runtime. Processor-documents may be Velora text plus a deterministic shell wrapper until a runtime exists.
- It does **not** replace Debian userland. v0.0.1 is a Debian arm64 image dressed in κύριος shape.
- It does **not** implement a kernel-level port-authority module. The "port" in v0.0.1 is a documented socket number plus key policy, with the kernel's ordinary access controls underneath.

## 4. Current state (the starting line)

The bench is ready for everything short of flashing:

```25:40:-os/docs/BENCH_STATE_20261116.md
## Toolchain [measured after operator grant, 2026-11-16]
...
| `aarch64-linux-gnu-gcc` | Debian 12.2.0-14 (12.2.0) |
| `debootstrap` | 1.0.128+nmu2+deb12u2 |
| `mmdebstrap` | 1.3.5 |
| `qemu-aarch64-static` | QEMU 7.2.22 ... |
| host `gcc` / `make` | 12.2.0 / GNU Make 4.3 |
| `python3` / `git` | 3.11.2 / 2.39.5 |

**End-to-end receipt ...**
Cross-compile: works. Arm64 execution under qemu with the cross sysroot: works.
```

The fleet's primary and supervisor nodes are partially measured:

```30:47:-os/fleet/README.md
## Measured specs — lirvol
| Model | **Raspberry Pi 5 Model B Rev 1.0** |
| RAM | 8 GB class |
| OS | Debian GNU/Linux 12 (bookworm) ... |
| NVMe | **KingSpec NE-256 2242, 238.5 GiB**, serial ... blank/unpartitioned |
| Audio | **52Pi NVDigi (EP-0220) — WM8804 S/PDIF, working.** |
```

The first application is already mapped:

```1:4:-os/continuous_persistence/REPORT_KYRIOS_CVE_20261115.md
**Filed:** 2026-11-15 · **Companion:** `REPORT_CVELISTV5_0XOSPREY_20261115.md`
**Subject:** κύριος (the OS ...) and how its architecture could benefit the work that `cvelistV5` / `cve.0xosprey.com` represent.
**Status of κύριος itself:** design stage.
```

## 5. Sequence of work

### Phase 0 — Ledger format and receipt tool (no image required)

1. Define the trace schema in `ledger/trace-format.md`.
   - Required fields per receipt: `space`, `port`, `key_fingerprint`, `action` (`arrive` | `grant` | `refuse`), `processor`, `document_hash`, `parent_hash`, `timestamp_utc`, `clew` (list of source documents).
   - Append-only rule: a receipt is never edited; a correction is a new receipt whose `parent_hash` names the line it answers.
2. Add `ledger/receipt.py` (or a small shell tool) that validates a line against the schema and appends it. Python work stays in the project venv per house rule.

This satisfies decision 4 (`Everything leaves a trace`) before any hardware is touched.

### Phase 1 — First space and processor documents (no image required)

1. Create `spaces/hello-kyrios.md`:
   - Name: `hello-kyrios`
   - Port: `7037`
   - Key policy: write allowed only from the `lirvol` host key; read allowed from any fleet node key.
   - Purpose: v0.0.1 proof-of-port.
2. Create `spaces/cve-raw.md` as a design document only:
   - Port: `7036`
   - Key policy: ingestor may write; enrichers may write an ADP overlay; readers may read CNA originals but not the reserved shadow.
   - This carries the CVE mapping forward without pretending to run the feed.
3. Create `processors/hello-kyrios.md`:
   - A short Velora program that declares one input, one output, and one invariant.
   - A deterministic shell wrapper runs it, captures `stdout`, and writes a receipt to the ledger.
   - This satisfies decision 3 (`Processes are documents`) in skeleton form.

### Phase 2 — Operator gate: build-target decision

The `-os/README.md` foundation table leaves two valid target bases:

```28:33:-os/README.md
| Target base | Debian 12 **Bookworm** arm64 — Buildroot (default) or debootstrap-arm64 + Pi firmware/boot | ... |
| Fidelity rule | Bookworm builds Bookworm ... |
```

The operator chooses one:

- **Option A — debootstrap + Pi firmware/boot** (recommended for v0.0.1): fastest honest path to a bootable Debian arm64 rootfs; lets the project lean on apt for the first image; Buildroot can be the v0.0.2 minimization target.
- **Option B — Buildroot**: more controlled, smaller image, but takes longer to configure and validate; better once the space/processor shapes are frozen.

The agent files the decision as an appended note in this document, then writes the matching build script.

### Phase 3 — Build the v0.0.1 image

If Option A:
- Create `build/bootstrap-lirvol.sh`: uses `mmdebstrap` to build an arm64 Bookworm rootfs, installs the Pi firmware/boot partition, enables `hifiberry-digi` overlay, sets hostname `lirvol`, installs the fleet SSH key, copies `ledger/`, `spaces/`, and `processors/` into `/opt/kyrios/`.
- Outputs: `build/kyrios-v0.0.1-lirvol.img`, `build/kyrios-v0.0.1-lirvol.img.sha256`, and a build receipt.

If Option B:
- Create `build/boards/lirvol/` defconfig and device-tree overlay list under `build/`.
- Outputs: the same image + checksum + receipt.

Either way, the image is **not** flashed in this phase.

### Phase 4 — Operator gate: flash and first boot

Per `HANDOFF.md`, the operator gates the first image:

```23:29:-os/HANDOFF.md
1. **Report the bench state.** ...
2. **Confirm the toolchain ...**
3. **Do not build an image yet.** The operator gates that. Bones first, bench verified, then he says go.
```

When the operator says go:
1. Flash `build/kyrios-v0.0.1-lirvol.img` to a spare SD.
2. Boot `lirvol` from SD; the NVMe remains blank for now.
3. Run the catalog's first-boot checks from the bench:
   - `cat /proc/device-tree/model`
   - `free -h`
   - `lsblk`
   - `vcgencmd get_throttled`
4. Partition and format the NVMe with LUKS + ext4 (operator-granted script under `tools/`; keys held off-card).
5. Move `/opt/kyrios/ledger/` to the NVMe mount and bind-mount it.
6. Run `processors/hello-kyrios` and verify a receipt line appears in the ledger.

### Phase 5 — Version boundary and v0.0.2 opening

v0.0.1 closes when:
- `lirvol` boots the image.
- SSH via fleet key works from the bench.
- The NVMe holds an append-only ledger.
- `hello-kyrios` leaves a trace that validates through `ledger/receipt.py`.

v0.0.2 opens with:
- Ingesting the CVE feed into `cve-raw` (per `REPORT_KYRIOS_CVE_20261115.md`).
- Bringing `valseir` online as supervisor/face.
- Designing the Pico watchdog heartbeat.

## 6. Gates and receipts

| # | Gate | Who holds it | Unblocks |
|---|---|---|---|
| 1 | Build target: Buildroot vs debootstrap | the operator | Phase 3 build script |
| 2 | Run image-build script (needs root for loop mounts / flash) | the operator | image artifact |
| 3 | Flash SD and boot `lirvol` | the operator | Phase 4 first-boot receipts |
| 4 | LUKS/format NVMe script (root) | the operator | ledger on NVMe |

Each gate crossing produces a dated receipt appended to `docs/GAMEPLAN_KYRIOS_v0.0.1_20261116.md` or a separate `docs/FIRST_BOOT_KYRIOS_20261116.md`, never an edit to the original line.

## 7. Deliverables of v0.0.1

- This plan: `GAMEPLAN_KYRIOS_v0.0.1_20261116.md`
- Callsign declaration: `LONSAIVEL_REFLECTION_20261116.md`
- Updated ledger: `org_chart.jsonl`
- Trace format: `ledger/trace-format.md`
- Receipt tool: `ledger/receipt.py`
- Space documents: `spaces/hello-kyrios.md`, `spaces/cve-raw.md`
- Processor document: `processors/hello-kyrios.md`
- Build script or defconfig: `build/bootstrap-lirvol.sh` or `build/boards/lirvol/`
- Image artifact: `build/kyrios-v0.0.1-lirvol.img` + `.sha256`
- First-boot receipts: `docs/FIRST_BOOT_KYRIOS_20261116.md`

## 8. House rules kept in view

- No host `sudo` from the agent. Any step that needs root is a one-line, idempotent script under `tools/` that the operator runs.
- Traces are append-only. A correction is a new line, not an edit.
- Python stays in the project venv (`.venv/`); dependencies live in `requirements.txt`.
- No agentic loops are built without the three conditions written down first: deterministic step, schema-constrained output, measurement in the same round.

---

```
LONSAIVEL 🗺️
A game plan for κύριος v0.0.1: path first, signs at every gate, ending as a document the operator can correct.
 LONSAIVEL?
```

# For Will — what κύριος and ὄνυχος are

Will — this is the short version of a system you've only seen pieces of,
written because you've been asking about the repo. Your CVE tracker is
one of the things it was designed around, so the last third of this is
yours. Everything claimed as *built* below has a receipt in the repo;
everything else is labelled design. Believe the receipts.

---

## TL;DR

**κύριος** is an operating system for a small fleet of Raspberry Pis,
built around one idea: **everything is a document in a space; every
space is reached only through a port under a key; the programs that
process documents are themselves documents; and every arrival, grant,
and refusal leaves an append-only trace.** It is currently being built
bare-metal, in C with AArch64 assembly, on the Pis themselves.

**ὄνυχος** is the same architecture aimed at compiled things instead of
records: build artifacts arrive at a space's port, and CI becomes
AST-shaped graphs and diffs over those artifacts — builds you can
interrogate instead of log files you have to believe.

Your osprey watches the CVE flood's rate. κύριος is the attempt to give
the flood a memory.

---

## The names (and why yours is nearby)

| Name | Greek | Literal sense | What it names |
|---|---|---|---|
| **κύριος** (*kýrios*) | κύριος | "of the authority" | The OS. Authority is a *collection of works* — as legal authority is a collection of texts. The OS is the port authority over its documents and processes: it grants and refuses access to hardware and spaces, and traces both. Not a personified "Lord" — authority itself. (A breath away from "curious" in English, which the operator does not consider an accident worth correcting.) |
| **ὄνυχος** (*onychos*) | ὄνυξ, ὄνυχος | "of the talon / claw" — whence *onyx*, the stone named for a fingernail | The planned CI-facing system: it grips compiled artifacts and does not let go until they've been diffed. It is expected to appear later as a fleet node's name in read-only audit mode (you'll meet it in `spaces/hello-kyrios.md`'s key policy). |
| **Velora** | — | — | The language the documents and processes are written in. Velora is the tongue; κύριος is the OS. They are not the same word, and the repo will correct anyone who merges them. |

And the similarity you may have already noticed: **ὄνυχος and osprey
are the same image** — a talon built for gripping prey out of a stream.
`cve.0xosprey.com` parsed (operator's reading, recorded in the repo):
**0x** (the hex prefix that tells the machine what kind of thing
follows) + **OS** + **prey**, inside *osprey*, the hawk. The resemblance
is convergent, not copied — the Greek side arrived from the claw-word,
yours from the bird. Two talons, same flood.

## Where the two systems came from

Both names carry a project they were designed for:

- **ὄνυχος was named for FullMetalPacket** — an intended *port-space CI*:
  instead of a build server emitting logs, the compiler's artifacts
  themselves arrive at a space, as documents, through its port. A build
  becomes a graph of things (AST-like structure, compiled units,
  linkages) that can be *diffed* between builds: not "the build passed,
  here's 4,000 lines of scrollback," but "these nodes of the graph
  changed, under this key, producing these artifacts, and here is the
  trace." CI as evidence handling.
- **κύριος was named for PortalVision** — an epistemic knowledge system:
  a system whose job is not merely to store claims but to know *on what
  authority* each claim rests, what produced it, and what it replaced.
  PortalVision had already audited FullMetalPacket; κύριος is that
  epistemic discipline turned into an operating system. When something
  occurs, **the kernel updates** — arrivals, timer events, hardware —
  and every update is receipted with its lineage, because in an
  epistemic system "it changed" without "why, when, and on whose
  authority" is not an answer.

So the loop is: ἡ ὄνυχος grips artifacts; κύριος remembers what the
grip found, and can prove it.

---

## The architecture

```
                    ┌──────────────────── FLEET ────────────────────┐
  you ── key ──▶ PORT 7036 ──▶ SPACE cve-raw ──▶ documents (CNA)
                    │              │ key A ≠ key B (no silent refusal)
  enricher ─ key ─▶ PORT 7035 ──▶ SPACE cve-adp ──▶ overlay documents
                    │              │
                    └── every arrival ──▶ LEDGER (append-only, hash-chained)
                                               │ decay over time
                                               ▼
                                     EVICTION STORAGE (external,
                                     with a receipt left behind)
  processes = documents on the same shelf (ingest-delta, kev-diff, …)
  kernel = the port authority: it grants, refuses, and traces; it does
           not author. Authors author. (Do not confuse the two.)
```

### The six load-bearing ideas

1. **Everything is a document in a space.** CVE records, AST graphs,
   poems, policies, receipts — documents. A space is a named room with a
   **size** (a real quota, enforced, not prose) and one way in:
2. **A port and a key.** Nothing reads or writes a space except through
   its TCP port, presenting a key whose fingerprint lands in the ledger.
   Ports talk TCP. (HTTP, if an application wants it, is spoken by a
   *process inside a space* — it is a layer, never the port itself.)
3. **Processes are documents.** The ingestor, the differ, the enricher
   are files on the same shelf as what they process — in Velora where a
   runtime exists, in C at the metal today. "Why did this record
   change?" is answered by *opening the process that changed it*, not
   by trusting a changelog.
4. **Set access to hardware.** Who may touch which device, space, or
   port is declared in advance as capability — grants *and refusals*,
   both legible, both traced. At bare metal this stops being a policy
   document and becomes page tables and driver permissions.
5. **Everything leaves a trace.** The ledger is append-only and
   hash-chained: a correction is a new receipt laid over the old, never
   an edit. The thread of receipts pulled for one case is a **clew** —
   jurisdiction, applied. "What did it say before, on whose authority?"
   is a cheap query here, not an archaeology expedition.
6. **Things decay to eviction storage.** Hot documents live on the
   NVMe; as they age they are copied to external (eviction) storage and
   an eviction receipt is appended — the original line is never edited
   to say so. The ledger always knows where a thing *went*. Decay is a
   lifecycle, not a deletion with good PR.

### The ren story (why the language shapes the kernel)

Velora's word for an algorithm is **`ren`** — F8, STORY, *memory told*.
The comparison that forced it:

```c
for (x = 0; x < 9; x++)   // C: start, termination, and step
                          // fused into one load-bearing header
```

Three declarations for one movement, structurally coupled: change the
stride, go backwards, restart midway — each means restructuring the
header. Velora keeps them **separate and dynamic**:

```
vel ren thim [arr] thir   # scope declared ONCE: the world, first→edge
ren vel sek               # step: advance        (a decision)
ren vel sek fim           # step: advance by 3   (still just a step)
ren vel ora thim          # step: return to start (same machinery)
```

The scope is the world; the story chooses how to move through it —
forwards, backwards, by strides, conditionally, returning to `thim`
(the first, by definition; there is no position 0, the absent place).
An algorithm that is a *story* can be retold, replayed, run backwards.
That is not ornamentation: a kernel whose processes are stories is a
kernel whose computations are natively replayable — which is what an
audit trail wishes it were. Sources: `conceptual_language/
VELORA_PROGRAMMING.md` §15–16 and `REN_REFLECTION_20261105.md` in the
wider workspace.

---

## How the CVEs folded in

Your two reports are in the repo now (`docs/reports/`):

- `REPORT_CVELISTV5_0XOSPREY_20261115.md` — what cvelistV5 and
  cve.0xosprey.com measurably are: **425,880 JSON records, 7.6 GB,
  updated roughly every 7 minutes**; CNA originals vs CISA-ADP
  enrichment (KEV, SSVC, vulnrichment); the 2024 bulk date repair
  (~27,000 records) and the retention cut it forced.
- `REPORT_KYRIOS_CVE_20261115.md` — the mapping onto the architecture.
  Its core claim, one paragraph: **the CVE ecosystem's problems are
  provenance problems at stream speed** — what changed, when, on whose
  authority, what did it say before. Those are the four questions
  κύριος was already designed to answer for documents in general.

Concretely, in κύριος terms:

- The feed is a **place, not a pipe**: `cve-raw`, port **7036** — each
  delta application is an arrival, receipted. (Space document:
  `spaces/cve-raw.md`.)
- **Reserved vs published stops being a view toggle.** The shadow
  inventory is a *separate space with a different key* — a room not
  everyone can enter.
- **CNA original vs ADP enrichment is a capability, not a convention.**
  The enricher process may write the overlay space; it may not alter
  the CNA original, because its key doesn't open that port. The 2024
  date repair, replayed here, would be a *process with write access
  arriving at a port*, leaving the old values — and itself — in the
  trace, instead of a bulk rewrite.
- The tracker face (your two charts, rate over the flood) is the
  display node's application, reading the spaces locally. Osprey
  above, κύριος below: the eye and the ledger.

ὄνυχος closes the same loop on the offensive/defensive tooling side:
the scanner, the tracker, the builds that produce them are themselves
compiled artifacts, arriving at a port, getting diffed as graphs,
leaving traces a pentester can stand behind in a report.

## What is actually built (receipts, not pitch)

- **v0.0.1 (Linux-based prototype) ran end-to-end** on `lirvol`, the
  Pi 5: booted its image, ran the `hello-kyrios` processor-document,
  left a valid receipt on the NVMe ledger. (`docs/
  FIRST_BOOT_KYRIOS_20261116.md`.)
- **Rung 2 (a space answering at its TCP port)** — grant/refuse/size/
  quota, hash-chained receipts — proven as a reference implementation
  on the bench *and* on the Pi 5. It now serves as the behavioural
  oracle. (`docs/witness/rung-2-20261116.md`.)
- **The turn you're reading about**: on the operator's ruling, the real
  OS is **bare-metal C + AArch64 assembly**. Rung B1 passed on
  2026-11-16 on `valseir` (Pi 4): our own 8,632-byte kernel —
  hand-written boot assembly, mailbox framebuffer — put
  **HELLO-KYRIOS** on its screen with no Linux underneath. Photograph
  filed. (`docs/witness/rung-b0-b1-20261116.md`,
  `docs/BARE_METAL_LADDER_KYRIOS_20261116.md`.)
- The fleet: `lirvol` (Pi 5, 8 GB, 256 GB NVMe, S/PDIF audio board —
  measured, not photographed-and-guessed) and `valseir` (Pi 4, 4 GB,
  screen) — `fleet/README.md`.

What is *not* built: a TCP stack at the metal (it is rung B8, and it is
allowed to take as long as it takes — it means writing the Ethernet
driver and ARP→TCP ourselves), EL0 processes, the CVE space running
live. The ladder document is public in the repo precisely so the order
of work can be argued with.

## How to read the repo in 20 minutes

1. `README.md` — the four decisions and the authority/authorship line.
2. This file.
3. `docs/reports/REPORT_KYRIOS_CVE_20261115.md` — your problem, mapped.
4. `docs/BARE_METAL_LADDER_KYRIOS_20261116.md` — where it's going.
5. `kernel/src/` — 200 lines that boot. The whole claim, auditable.

Questions, challenges, and "you can't write a TCP stack" are all
welcome — the last one is scheduled as rung B8 and the operator has
heard it from oracles before.

## Correction of record (2026-11-16, 22:51 — the operator, verbatim sense)

**ὄνυχος is a machine, not a system.** Even with modifications, it is
the auditor: it is not the system, and it carries **no state**. Machines
do a job and they output. ὄνυχος very specifically does just that —
audits κύριος's traces, gathers its receipts transiently, writes the
report, deletes the receipts, frees the memory. Wherever this letter
calls ὄνυχος a "CI-facing system" or a "planned system," read it under
this correction: the *system* is κύριος; ὄνυχος is the machine
(`valseir`, running Kali today, stock) that audits it and reports.
That is also why it may hold no ledger of its own: a machine that kept
state would become a second authority, and there can't be two.

One more load-bearing fact in the same correction: **ὄνυχος carries
the parent process, and it must be there to witness κύριος boot.** The
auditor machine comes up first; the system does not boot unwatched.
(It is stock Kali tonight and the parent process is not yet built —
the carrying is the design's requirement, not a claim about tonight.)

— filed 2026-11-16, from the bench (satan-sums), for Will Duncan,
via the operator. Receipts in-repo; corrections append-only.

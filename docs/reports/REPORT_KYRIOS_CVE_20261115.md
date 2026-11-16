# Report — κύριος against the CVE problem

**Filed:** 2026-11-15 · **Companion:** `REPORT_CVELISTV5_0XOSPREY_20261115.md`
**Subject:** κύριος (the OS, formerly referred to as Velora OS in early drafts; Velora remains the language) and how its architecture could benefit the work that `cvelistV5` / `cve.0xosprey.com` represent.
**Status of κύριος itself:** design stage. Dev bench decided (Debian 12 Bookworm partition; Bookworm arm64 target; Buildroot or debootstrap+Pi firmware per `REPORT_LINUX_DISTRO_VELORA_OS_20261114.md` §7). Nothing in this report claims a built system; it maps decisions already on record onto a measured problem.

## 1. The problem, restated in κύριος's terms

The CVE list is 425,880 documents that change every seven minutes, whose corrections rewrite history (27,000 dates repaired at once), whose consumers must know *what changed, when, and on whose authority*, and whose size (7.6 GB) punishes every naive copy. The mainstream answer is a bigger database and an API key. κύριος's answer is its four load-bearing decisions, each already made for other reasons:

1. **Spaces with a port and key.**
2. **Set access to hardware.**
3. **Processors are documents.**
4. **Everything leaves a trace.**

## 2. The mapping

### Spaces with a port and key → the feed becomes a place, not a pipe

In the Spaces Between protocol (measured, Nov 2026), a space is reached at an address with a key (SPACE_A: port 7036, key A; SPACE_B: port 7035, control). A κύριος CVE space works the same way: the local mirror of the list lives *in a named space*, and nothing reads or writes it except through its port, under its key. Benefit over `git clone` + cron: the clone model gives everyone root over a directory and hopes; the space model gives the feed a **door with a ledger** — every delta application is an arrival at the port, receipted. The reserved/published distinction becomes two spaces with different keys: the shadow inventory (reserved) is not merely a view toggle, it is a room not everyone can enter.

### Set access to hardware → enrichment becomes a capability, not a convention

The tracker's two honest layers — raw CNA record vs CISA-ADP enrichment (KEV, SSVC) — are currently conventions in one file. Under κύριος, access to hardware and spaces is *set*: this process may read the CVE space but not write it; that enricher may write the ADP space but not alter the CNA original; the Pi on the wall may hold the KEV space and nothing else. The 2024 date-correction event is the argument: when 27,000 records' dates are wrong, in the current model the fix silently rewrites files. In a capability-set space, the repair is a *process with write access arriving at a port*, and the old values don't vanish — see §4.

### Processors are documents → the pipeline is inspectable by reading it

κύριος's processors are documents (Velora programs — the same ontological class as the documents they manage). A CVE pipeline on κύριος is therefore a shelf of documents anyone with space access can open: `ingest-delta`, `apply-date-normalisation`, `kev-diff`, `triage-by-ssvc`. Each is versioned like the poem files are versioned, each execution is itself a document-producing act. Benefit: the pentester's question "why did this record change?" is answered by *reading the processor that touched it*, not by trusting a vendor's changelog. When Will's tracker says "Counts follow the selected period and date," on κύριος the counting document is on the shelf next to the counts.

### Everything leaves a trace → date normalisation stops being a rewrite

This is the sharpest fit. cvelistV5's known wound is *history repair*: dates wrong in bulk, fixed in bulk, downstream metrics poisoned, the deltaLog's retention cut to survive the file size. κύριος's append-only discipline (the same one that runs the GGSD ledger, the BLANK_SPACE channel, and every receipt in this project) makes repair a **new trace laid over the old**, never an edit: record-as-published, record-as-corrected, the correcting process, the date of correction — all present, all queryable. `daughter.created_at=20200721` does not overwrite `daughter.created_at`; the poem already taught this OS why not. A CVE space on κύριος could answer, for any of 425,880 records, "what did you used to say?" — the question the current git history can technically answer and practically nobody asks because it costs too much. Traces make it cheap.

## 3. Concrete shape (one fleet, from the catalog)

- **Pi 5 + NVMe node** (`raspberry_pi_images/CATALOG.md` unit B): holds the CVE space. 7.6 GB is nothing to NVMe; hourly delta spaces arrive at the port under key; baseline rebuilds are new spaces, not mutations.
- **Display Pi** (unit C): the tracker, local — 0xOSprey-style rate charts served from the space, offline-capable, no phone-home.
- **Pico watchdog**: if the ingest process dies, the trace shows the gap; the watchdog restarts the document-processor, and the restart is also traced.
- **Kali live USB** (current snapshot, per distro report): inspection of the fleet *without tripping updates on the inspected systems* — the forensic-mode concession, applied to security tooling itself.

## 4. What κύριος does not fix (honesty section)

- **Volume judgement.** Traces make the flood auditable, not smaller. KEV-style prioritisation is still a human/policy act; κύριος can hold the policy as a document, it cannot have the opinion.
- **Upstream truth.** If a CNA publishes a wrong record, κύριος preserves the wrongness *and* its correction beautifully. Provenance is not accuracy.
- **Key custody (corrected 2026-11-15, operator's correction accepted).** An earlier draft of this line claimed "physical access is root" on a Pi fleet. That is not true and is corrected here: Debian 12 does not grant root to everyone — accounts, sudo policy, and file permissions are enforced by the running OS exactly as on any Debian host, and a Pi is not an exception. The actual residual risk is narrower and different: an *unencrypted, removable* SD card can be taken out and read offline by another machine, bypassing the running OS's permissions entirely. That is a media-encryption question (LUKS at rest, keys held off-card), not an OS-access flaw, and it applies to any removable-media system. Spaces holding their keys off the card they govern is the κύριος-shaped answer; it is stated here as the design requirement rather than assumed away.
- **Build status.** This is a design mapping onto decided foundations (Bookworm bench, Pi fleet, space protocol measured in another experiment). The OS does not exist yet. The claim is *fit*, not *product*.

## 5. Verdict

The CVE ecosystem's deepest problems are not search problems; they are **provenance problems at stream speed** — what changed, who changed it, what did it say before, who was allowed to touch it. Those are the four questions κύριος was already designed to answer for documents and processes in general: spaces with ports and keys, set hardware access, processors you can read, traces you cannot quietly edit. Will's hawk watches the flood's rate. κύριος would give the flood a memory. The two compose: osprey above, κύριος below — the eye and the ledger.

— HALMANSEIR, 2026-11-15

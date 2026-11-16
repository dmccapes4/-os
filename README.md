# κύριos

**An operating system that manages documents and processes, deployed to Raspberry Pis.**

The name is **κύριos** (*kýrios* — of the authority; a breath away from "curious" to an English ear). The naming logic is the operator's: what is queried is *of the authority* (κυρίου, the genitive) — put to the OS, which is itself the ultimate authority over its documents and processes.

> Velora is the language. κύριος is the OS that manages Velora documents and processes. The authority and the tongue are not the same word.

## Authority, authorship, and the port (the name, stated plainly)

- **Authority is a collection of works.** Legal authorities are a collection of legal texts; an OS's authority is its collection of governing documents, policies, and traces. Jurisdiction is established through the **lineage** of those texts — which is why this repo is append-only: a correction is a new work in the lineage, never an edit to an old one. The unit of jurisdiction as applied to a particular case is a **clew**: the thread of works pulled to that case, the way deeds pulled to a title make the title clear.
- **κύριos is "of the authority," not a personified Lord.** Modern morphology lends the word the masculine and calls it *Lord*; the operative sense here is the older one — authority itself. A legal authority is the collection of all legal doctrine authored into the authority. Jusisdiction is the ordered combination of laws that establish what is currently legal. The area over which an authority may enforce its laws is one aspect of the law found through the jurisdiction of the authority. I define a clew as the slice of jurisdiction which applies directly to a specific case. κύριos makes clews super easy to navigate.
- **An operating system is a port authority. That is not cute.** It literally manages ports granting access to hardware. Only a kernel actually *has* authority in this sense, because it owns access to the software and the hardware: every grant and every refusal passes through it. Daemons serve the kernel and the kernel serves the daemons — service is reciprocal; worship is not part of the design.
- **Do not confuse authority with authorship.** A king is vested with *authorship* — the making of works. Authors author documents and policies; the kernel grants or refuses access to them. Authorship does not grant access, and access does not author anything. κύριος holds the first kind of power (the port, the grant, the trace of both) and must never quietly claim the second.

## The four decisions (everything else follows from these)

1. **Spaces with a port and key.** Content lives in named spaces. Nothing reads or writes a space except through its port, under its key. Every arrival at a port is receipted.
2. **Set access to hardware.** Which process may touch which device, file, or space is set in advance as capability — grants *and refusals* both legible, both traced.
3. **Processes are documents.** The programs that process documents are themselves documents (Velora programs): readable, versioned, on the same shelf as what they process. "Why did this change?" is answered by opening the process document.
4. **Everything leaves a trace.** Append-only. A correction is a new trace laid over the old, never an edit. `created_at` does not get overwritten.

## Foundations (decided, on record)

| Layer | Decision | Source |
|---|---|---|
| Dev host | Debian 12 **Bookworm** amd64 (netinst ISO is host installer only — never an SD image) | `continuous_persistence/REPORT_LINUX_DISTRO_VELORA_OS_20261114.md` §7 |
| Target base | Debian 12 **Bookworm** arm64 — Buildroot (default) or debootstrap-arm64 + Pi firmware/boot | Composer's stack summary, adopted in the same report |
| Inspection | Current Kali snapshot, live USB — inspect without tripping kernel updates on the inspected system | operator, 2026-11-14 |
| Fidelity rule | Bookworm builds Bookworm (glibc 2.36 target; a Trixie host's 2.41 binaries refuse to load) | distro report §7 |

## The fleet (from `raspberry_pi_images/CATALOG.md`)

- **Pi 5 + NVMe** — primary node; NVMe takes ledger churn.
- **Pi + touchscreen** — supervisor / face.
- **Pico** — hardware watchdog (RUN/EN pins).
- **e-Paper** — zero-power status face.
- First application mapped: a CVE space (see `continuous_persistence/REPORT_KYRIOS_CVE_20261115.md`) — provenance at stream speed: what changed, when, on whose authority, what did it say before.

## Layout of this repo

```text
docs/        design documents, decisions, dated records
spaces/      space definitions (name, port, key policy, access set)
processors/  processor-documents (Velora programs that process documents)
fleet/       per-node configuration for the Pi fleet
build/       image build machinery (Buildroot configs / debootstrap stages)
ledger/      trace format and ledger tooling — append-only, always
```

## Status

**Bones, 2026-11-15.** Decisions and architecture on record; no image built yet. The development bench is a Debian Victus gaming laptop (see `HANDOFF.md`). Access model note, corrected on the operator's instruction: Debian 12 does not grant root to everyone; OS accounts and permissions are enforced as on any Debian host. The residual physical risk is offline reading of unencrypted removable media — a LUKS/keys-off-card question, not an OS-access flaw.

## Standing rules for anyone (human or agent) working here

- Traces are append-only. Never edit a receipt to make it agree with a later one.
- A denied access leaves as clear a trace as a granted one. No silent refusals.
- No host `sudo` from agents. When root is required, hand the operator a script.
- Python in a project venv (`.venv/`), dependencies in `requirements.txt`. No system pip.
- No agentic loops without the three conditions written down first (deterministic step, schema-constrained output, measurement in the same round).

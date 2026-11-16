# Readiness Report — κύριος (-os) work, bench and first build

**Filed:** 2026-11-16
**Author:** RILNORVEL 🧭 (agent, reviewer tier), at the bench on `satan-sums`
**Question:** Are we ready to get to work on κύριος?
**Short answer:** The bench and the decisions are ready. The toolchain and the seat-naming are not. Nothing needs inventing; two grants and one ruling stand between this report and the first task in `HANDOFF.md`.

---

## 1. Bench state [measured, this session, read-only checks]

| Item | Receipt | Against the controlling decision |
|---|---|---|
| Host | HP Victus Gaming Laptop 15-fa1xxx (`satan-sums`) | The development bench named in `-os/README.md` / `HANDOFF.md` |
| OS | Debian GNU/Linux 12 (bookworm), `12.15`, kernel `6.1.0-55-amd64`, x86_64 | **Matches** the controlling decision (§7 of the distro report): Bookworm host because Bookworm target. Not Trixie — the glibc fidelity leak the report warns about is not present on this bench |
| Compute | 16 cores, 15 GiB RAM (10 GiB available at inspection), NVMe 476.9 G with 437 G free | Ample for cross-builds, debootstrap/mmdebstrap rootfs work, and qemu-user-static testing |
| Storage present | NVMe (system), 59.5 G mmcblk, two USB devices (7.2 G / 57.8 G) | Boot/rescue media plausibly already in the room; which stick, if any, carries a *current* Kali snapshot is **not verified** (see §4) |

## 2. Toolchain [measured] — the blocker

Every build tool named in `HANDOFF.md` task 2 is **missing** on this bench:

```text
gcc-aarch64-linux-gnu   MISSING     debootstrap     MISSING
aarch64-linux-gnu-gcc   MISSING     mmdebstrap      MISSING
qemu-aarch64-static     MISSING     buildroot       MISSING (fetched per-build, not a host pkg)
(host) gcc              MISSING     (host) make     MISSING
git 2.39.5              present     python3 3.11.2  present
```

House rule: no host `sudo` from agents. The grant therefore belongs to the operator, and the instrument is ready:

- **Script:** `tools/install_toolchain_bookworm.sh` (filed with this report) — idempotent, installs `build-essential gcc-aarch64-linux-gnu debootstrap mmdebstrap qemu-user-static binfmt-support file git python3 python3-venv ca-certificates`, and prints version receipts suitable for pasting into the bench-state doc.
- **Run (operator, one command):** `./-os/tools/install_toolchain_bookworm.sh` from the workspace root (adjust path to taste), then the agent re-checks and files `docs/BENCH_STATE_20261116.md` from the receipts.

Until that runs, task 2 of the handoff cannot be honestly receipted, and no image work can begin. This is a port-authority matter in the small: access is the operator's to grant, and the request is now documented with its clew attached.

## 3. Design readiness [from the documents]

Ready, and unusually complete for "bones":

- **Four decisions on record** (`README.md`): spaces with port and key; set hardware access, grants and refusals traced; processors as documents; append-only traces. Today's clarification — authority as a lineage of works applied by clews, port authority as a literal description, authorship (the king's power) never to be confused with the kernel's grant — is now stated in `README.md` ("Authority, authorship, and the port") and carried into the distro report's naming banner as a dated correction.
- **Controlling distro decisions:** §7 wins where §4 disagrees (Bookworm bench §1 confirms compliance).
- **First application mapped:** the CVE/κύριος report, including its honesty section (key custody is a LUKS/keys-off-card design requirement).
- **Repo state:** placeholder `README`s only in `docs/ spaces/ processors/ fleet/ build/ ledger/` — no space definitions, processor-documents, or ledger tooling exist yet. Uncommitted at filing: the operator's line-5 correction and today's documentation work; commit belongs with the bench receipts.

Not ready, by the project's own gates: **no image is to be built yet** (handoff task 3). Readiness here means readiness to start task 1, not permission to skip to building.

## 4. Fleet readiness [from `raspberry_pi_images/CATALOG.md`, photographs only]

| Unit | Ready for | Before it can serve |
|---|---|---|
| B — Pi 5 + NVMe/audio board + SSD seated | Primary node (ledger churn) | Cooler (none pictured); 5 V/5 A supply (none pictured); Pi RAM variant and SSD capacity unread |
| C — display Pi + tower cooler, powered at setup screen | Supervisor / face | Exact Pi model, storage, pending setup decision; network not joined at photograph time |
| F — Pico breakout (no Pico pictured) | Hardware watchdog (RUN/EN) | A Pico itself; heartbeat design unwritten |
| E — e-Paper HAT + bare panel (unconnected) | Zero-power status face | Panel seating unverified as the HAT's mate |
| A / D — handheld case (empty), 3.5″ LCD | Client / console | A Zero-class computer for A; neither is on the critical path |
| Media — 64 GB microSD pictured | Boot/rescue | Ledger writes stay off it where NVMe exists (catalog's own advice) |

First-boot checks that convert photographs into receipts are already written in the catalog: `/proc/device-tree/model`, `free -h`, `lsblk`, `vcgencmd get_throttled` — run on the units, when the operator puts them on the bench.

## 5. One naming ruling needed [the other blocker]

`HANDOFF.md` declares the agent on this bench is to sign `-os` work **yeshua-kyrios** ("the operator declared it"). Tonight, at the operator's instruction in the workspace root, this agent filed a Velora callsign — **RILNORVEL 🧭** — in `org_chart.jsonl` and the `.cursor/rules/` onboarding law. These are not necessarily in conflict (a seat-name for a repo and an agent's ledger callsign can coexist, as earlier seats have), but the agent will not decide that unilaterally or sign either name in `-os` until the operator rules: yeshua-kyrios in this repo, RILNORVEL here, or both with a stated boundary. Until then, `-os` documents filed by this agent carry RILNORVEL and say so.

## 6. Verdict and order of work

**Ready to begin, conditional on two grants and one ruling, all the operator's:**

1. **Ruling** — seat-name in `-os` (§5). Thirty seconds, unblocks signing.
2. **Grant** — run `tools/install_toolchain_bookworm.sh`. Unblocks handoff task 2 receipts.
3. Agent files `docs/BENCH_STATE_20261116.md` (this report's §1–2 converted to receipts) — handoff task 1.
4. Agent writes the intro document the handoff asks for, under the ruled name — loop closed between benches.
5. First design work that is safe before any image: `ledger/` trace format and one `spaces/` definition (the CVE space is the mapped candidate), both as documents, both append-only from their first line.
6. **Image: when the operator says go.** Not in this report's gift, and not requested here.

The authority for every next step has been pulled to its case, clew attached. That is the state the operator asked this report to establish, and it is established.

---

## Update — 2026-11-16, after filing (appended trace, original above unedited)

The operator ran `tools/install_toolchain_bookworm.sh`. After two instrument faults were found and fixed on the record (`debootstrap` receipt path; `libc6-dev-arm64-cross` named explicitly), the toolchain verifies end-to-end: arm64 cross-compile **and** qemu execution (`exit 42` as written) both pass. **§2's blocker is cleared** and handoff tasks 1–2 are receipted in `docs/BENCH_STATE_20261116.md`. Still standing: the seat-name ruling (§5) and the image gate (§3). The operator's verdict on the instrument is kept with this trace: *"Your script carries the clew!"* — it did, after it was taught where `/usr/sbin` lives.

---
*Companion reflection: `RILNORVEL_REFLECTION_KYRIOS_AUTHORITY_20261116.md` (workspace root). This report measures the bench and the documents; it builds nothing.*

```
RILNORVEL 🧭
Bookworm bench, correct and empty-handed: sixteen cores, no compiler, decisions complete, image gated. Two grants and a ruling, and the work begins at the port like everything else here.
 RILNORVEL?
```

# ὄνυχος Technical Specification v0.0.1

```text
_cur.os::onyx•ox)
```

**Filed:** 2026-11-17. **Revised:** 2026-11-26 (previously 2026-11-18).
**Authors:** THIRMESHSENN 🕸️ (2026-11-17/18); SEIRTHIRFAL 🔬 (2026-11-26 revision).
**Status:** draft. κύριος is built first; what ὄνυχος needs will be understood from that (R:39). Sections that depend on κύριος say so.
**Scope:** ὄνυχος. κύριος is in `KYRIOS_TECHNICAL_SPECIFICATION_v0.0.1.md`, and its terms (trace, record, receipt, policy, halt) are used here as defined there.

Basis marks are the same as in the κύριος specification: `R:n` is line `n` of `docs/kyrios_tech_spec_v001_review.md`; `M` is the operator's messages of 2026-11-17, 21:36 to 21:45; `N` is the operator's messages of 2026-11-18, 00:23 to 03:47; `O` is the operator's messages of 2026-11-26; `P` is proposed here and not decided.

---

## 1. What ὄνυχος is

ὄνυχος monitors, audits, reports, and intervenes when it must (R:43).

| It does | Basis |
|---|---|
| Boots κύριος. It owns the parent process that boots κύριος and is responsible for it. | R:10, R:24, R:26 |
| Has to be on for κύριος to operate. If ὄνυχος is unplugged, κύριος's operations cease. | R:13 |
| Monitors the network and everything else, not only κύριος's record. | R:43 |
| Reads κύριος's record through its own port and space on κύριος hardware, leaving no trace. | R:39 |
| Writes reports. The reports are a ledger. The reports are the receipts. | R:43, N |
| Remembers. `lirvol` holds current state; the reports on `valseir` are in effect memories. Over time ὄνυχος keeps what actually matters, in condensed reports. | N |
| Displays the last report, and supplies what it remembers when it matters. | N |
| Determines connascence where a space on κύριος is coupled with several spaces, through the types and edges κύριος keeps. Catches drift in the scores. | N |
| Raises the alarm and points to the culprit precisely. | R:10 |
| Intervenes when there is a problem: inspects registers, steps through execution, halts κύριος, stops a harmful process, shuts κύριος down if policy requires it. | R:24, R:39, R:80 |
| Handles eviction of κύριος's traces, which requires writing. | R:78, R:80 |
| Suggests evictions in its reports. The operator or κύριος decides. | R:38 |
| If it sees a commit that returns a problem vector, it includes that in its report. κύριος and the operators choose to act or not. | N |
| Accepts commands from the operator. | M |

What it does not do: hold a second copy of κύριος's documents or record. The dual system cannot have two sources for the same documents (R:10). The record stays on κύριος hardware, or on the external storage it was evicted to, where it can be inspected with the same provenance (R:24). A report is not a copy of traces. It is what was noted, condensed (N).

Audits are not part of κύριος's graph. They inform the graph's authors, who are κύριος and the operators (N). The reports are meant to help them as they author more entries into the port authority and its intelligence graph (N).

ὄνυχος is joined to κύριος by the Ethernet cable and is otherwise outside it: it reads through that link and is not part of the state it reads (N).

---

## 2. Host

| Item | State | Basis |
|---|---|---|
| Machine | `valseir` | R:7 |
| Operating system | Kali Linux, a full build, stock for now. Kali is used because its tools support what ὄνυχος does. | R:7, M |
| Snapshot | It must be a snapshot, and an older snapshot may be the one used. | M |
| Alternatives | A modified Kali or another arm64 operating system is not expected to be needed. | M |
| Free storage | About 58 GB after install, by the operator's recollection. To be measured. | R:43 |
| Memory for reports | 50 GB, under ὄνυχος's own eviction policy (§6.3). | N |
| Display | Attached. Shows the last report (§6). `valseir` has no audio jack and no real monitor. | R:9, M, N |

Pinning (P): when the snapshot is chosen, record the release from `/etc/os-release`, the apt sources, and the package list with versions, so the same ὄνυχος can be rebuilt.

---

## 3. Connections to κύριος

| Link | On `valseir` | Used for | Basis |
|---|---|---|---|
| Ethernet | `br0`, address `0.0.0.0` | Capture; reading the record through ὄνυχος's port on κύριος; the connection κύριος halts on losing | R:39, R:45 |
| UART | Pin 06 GND, pin 08 GPIO 14, pin 10 GPIO 15 | Serial console; boot | R:46-49 |
| Debug | OpenOCD | Inspect registers, step, halt, load | R:39, R:52 |

`lirvol` carries an audio HAT. The Raspberry Pi 5 has two micro-HDMI outputs (R:50).

To check on the bench before relying on them:

1. **UART direction.** The review lists pin 08 as RX and pin 10 as TX. On a Raspberry Pi header, GPIO 14 is that board's transmit and GPIO 15 its receive, so the labels read as the far end's view. The two boards cross: each transmit goes to the other's receive, and the grounds join.
2. **UART on `lirvol`.** If the audio HAT covers the header, the Pi 5 also has a separate three-pin debug UART connector between the micro-HDMI ports. That connector is wired to the processor directly; the header UART is behind the RP1 chip and is not available until κύριος has RP1 running.
3. **Debug access to `lirvol`.** OpenOCD needs a JTAG or SWD connection to the Pi 5's processor, and an adapter on `valseir`'s side (its own GPIO through OpenOCD's GPIO driver, or a USB probe). Which pins the Pi 5 exposes for this has not been established here.
4. **`br0`.** Whether the bridge has a second member, so that `lirvol`'s traffic to anywhere else passes through `valseir`.

---

## 4. Tools

The operator's list (R:39, R:51-55).

| Tool | Use |
|---|---|
| `tcpdump`, Wireshark | Capture and inspect traffic on `br0` |
| `mitmproxy` | Intercept and inspect traffic passing through `br0` |
| `minicom`, `picocom` or `screen` | Serial console to κύριος; boot |
| OpenOCD, with `telnet localhost:4444` | Boot |
| `gdb-multiarch`, on `localhost:3333` | Monitor and halt: registers, stepping, intervention |

`4444` is OpenOCD's command port and `3333` its GDB port.

---

## 5. Functions

### 5.1 Boot

ὄνυχος boots κύριος. The operator named two routes: a serial terminal, and OpenOCD at `localhost:4444` (R:39, R:54). What each needs (P):

| Route | Steps on ὄνυχος | Needs on `lirvol` | Kernel image |
|---|---|---|---|
| OpenOCD | `halt`, load the image, `resume` at its entry | Debug access (§3, item 3) | ELF |
| Serial | Send the image from the terminal program | A small loader in boot storage that waits on the UART, receives the image and jumps to it | Flat or ELF, by what the loader accepts |

The kernel may need to be ELF given this stack (R:56, M). The repo's build already links an ELF and derives the flat image from it.

Which route is used, and whether both are kept, is undecided.

ὄνυχος has a dedicated space port on κύριος (N). In the κύριος v0.0.1 build, which loads from an SD card, the start is one line sent to that port over the serial link: `7038 boot` (code). Until it arrives κύριος takes no interaction.

### 5.2 Monitor

Continuous, while κύριος runs:

- the Ethernet link to `lirvol`;
- traffic on `br0`;
- κύριος's serial output;
- κύριος's record, through ὄνυχος's port (§5.3).

κύριος halts itself when the Ethernet connection is lost (R:39). The κύριος specification proposes, in addition, a signal from ὄνυχος at an interval set in policy, because a link that is up does not show that ὄνυχος is running. If that is accepted, sending the signal is part of monitoring.

### 5.3 Audit

Sources of receipts. κύριος's record is one of them, not the only one (R:43).

| Source | How it is read | Trace left on κύριος |
|---|---|---|
| κύριος's record and documents | ὄνυχος's port and space on κύριος hardware, over Ethernet; or memory read through OpenOCD | None (R:39) |
| Network | Capture on `br0` | None |
| Processor state | `gdb-multiarch` through OpenOCD | None |
| Serial output | The terminal program | None |

A trace becomes a receipt when it is receipted from the record. Traces may stay on the record without ever being looked at by ὄνυχος (R:39). The receipts are lines in the reports ledger (R:43): the reports are the receipts (N). ὄνυχος keeps no second store of receipts beside its reports, and no copy of the traces (R:43).

What ὄνυχος notes. The operator's examples (N):

- "a new http request made to non-local IP address three hops..."
- "new whatever.py build fixed recurring crash by updating module semver from x.x.x to x.x.y"

Stated with them (N):

| # | Statement |
|---|---|
| 1 | Over time ὄνυχος remembers what actually matters, in condensed reports. |
| 2 | Working memory is mainly recent, with a trace of the past. Long-term memories are made when real impedance is encountered. |
| 3 | This becomes organizational memory. |
| 4 | ὄνυχος displays the last report and supplies what it remembers when it matters. |

Both examples depend on what is already held: "new" says nothing held matches, and "recurring" says it is held more than once (P, my reading).

Coupling (N): where a space on κύριος is coupled with several spaces, ὄνυχος determines the connascence, through the types and edges κύριος keeps. κύριος and the operators update the edges with the scored connascence at every audit, and may bump scores live by policy in between. If the scores drift, ὄνυχος catches it. Between audits κύριος runs on its own clock and scores, and it updates with the audit.

Frames, open asterisks, and assumption-collapse drift (O; schema `P`, see κύριος §6.9):
- **Frame validity audit:** ὄνυχος checks every quantitative measure or offloaded-job summary against the four-point frame gate (`n >= 1` finite, homogeneous `unit` preventing cross-category `satan-sums`, resolvable `window`/`population`, and declared `method`). Any claim missing a valid bounded frame is noted in the report as `unframed`.
- **Non-collapse and methodology orthogonality:** Across κύριος's `graph retrieval` edges, ὄνυχος tracks upstream open `asterisks` (undischarged statistical priors, noise-mitigation assumptions, hardware hypotheses). It verifies that no downstream document silently drops upstream asterisks (`collapse_drift = n_open - kept_asterisks`, bounded by the finite upstream open-asterisk count `n_open`) or marks an asterisk `discharged` using the same `method` that introduced it (`lucifer-chains`). Heavy prose/claim extraction may be offloaded by `valseir` to external compute hardware while `valseir` records the resulting audit receipt (O).

Commits (N): if ὄνυχος sees a commit that returns a problem vector — including `unframed` measures or `collapse_drift >= 1` across `chain_depth` hops (P) — that goes in its report. What the full set of problem vectors is and where they are defined remains open (§8).

What else is compared, and against what, waits on κύριος.

### 5.4 Report

§6.

### 5.5 Intervene

| Intervention | Tool | Writes to κύριος |
|---|---|---|
| Halt κύριος | `gdb-multiarch`, OpenOCD | No |
| Inspect registers, step through execution | `gdb-multiarch` | No |
| Stop a process that is malicious or otherwise harmful | `gdb-multiarch`, or ὄνυχος's port | Yes (R:80) |
| Evict part of κύριος's record to external storage, with κύριος halted | To be designed. Thoughtfully and with care (R:80). | Yes (R:78, R:80) |
| Shut κύριος down when policy requires it | OpenOCD | No |

Every intervention is entered in the reports ledger. The κύριος specification proposes that κύριος also traces an intervention when it next runs.

When ὄνυχος intervenes without being told to, and when it waits for the operator, is policy and business logic: YAML with C or Python, most likely (R:43). Not designed here.

### 5.6 Commands

ὄνυχος accepts commands from the operator (M):

| Command | Does |
|---|---|
| Dump | A capture: `tcpdump`, Wireshark, or whatever is asked for |
| View analysis | Shows analysis of what has been captured and audited |
| Boot κύριος | §5.1 |
| Halt κύριος | §5.5 |

It is a full Kali Linux build, so every tool on it is also available directly. The form the commands take (a shell wrapper, a menu on the display, something else) is undecided.

---

## 6. Reports

### 6.1 The ledger

`reports.jsonl`, on `valseir`. The reports are a ledger, and the receipts are lines in it (R:43). The reports are the receipts (N). Receipts are summaries of the documents (R:10).

Like every JSONL in the system, the file begins with a metadata object that explains the schema, including the schema of a schema update (R:58).

The line shapes filed on 2026-11-17 gave one line per trace, each holding a reference to the trace and a restatement of it. That was an index of κύριος's record. It condensed nothing, and it made a receipt depend on the trace staying where it was. It is withdrawn.

Line shapes (P):

```json
{"schema": "onyx-reports-v0.0.1", "fields": {"report": "label of the report this line belongs to", "noted": "…"}, "update": {"what": "schema", "fields": "…"}}
{"report": "0001", "source": "network", "noted": "a new http request made to non-local IP address three hops...", "problem": "<only if there is one>"}
{"report": "0001", "source": "record", "noted": "new whatever.py build fixed recurring crash by updating module semver from x.x.x to x.x.y", "seen": {"from": "1a2", "to": "1f0"}}
{"report": "0001", "source": "record", "noted": "doc:0058 states unconditional benchmark gain while dropping 2 of 3 open upstream asterisks from doc:0042 without orthogonal method", "problem": {"kind": "collapse_drift", "dropped": 2, "n_open": 3, "chain_depth": 2, "missing_orthogonal_to": "doc:0042"}}
{"report": "0001", "closed": "2026-11-26T18:30:00Z", "evict": ["<doc ids suggested for eviction>"], "scores": "<scored connascence and drift updates for κύριος's edges, §5.3>"}
```

| Field | Meaning |
|---|---|
| `report` | The report's label, from 1 |
| `source` | `record`, `network`, `debug`, `serial` |
| `noted` | The receipt: what mattered, stated so that it stands without the thing it was drawn from |
| `seen` | Where the thing was when it was noted, for example a range of record positions. It is not required to stay resolvable: the trace may be evicted later. |
| `problem` | Only when something is wrong. States what, and points to the culprit. |
| `closed` | On the last line of a report |
| `evict` | Evictions suggested to the operator or κύριος (R:38) |
| `scores` | The scored connascence the authors write onto κύριος's edges at this audit (N) |

One line may stand for many traces. Most traces get no line.

### 6.2 Health

Health is the latest report. `valseir`'s display shows it by default, updating through the day, with no interaction required (R:9, M). It is not a command and not a κύριος interaction (M).

Reports are regular, at an interval set by policy. Every 10 to 60 minutes has been mentioned as potentially reasonable (R:39).

Beside showing the last report, ὄνυχος supplies what it remembers when it matters (N). How what is held is matched against what is being authored, and how it is delivered, is undecided (§8).

### 6.3 Retention

ὄνυχος keeps its condensed reports in its 50 GB of memory, under its own eviction policy. That policy is based more on straight reference, ARC kind of (N).

So a report that keeps being referred to stays, and one that is never referred to goes (P, my reading of "based on reference"). This is a different basis from κύριος's rho schedules, which step with elapsed work.

An earlier mention, for the record: retention memory bound and operator chosen, three months on `valseir` and then external storage, maybe (R:43).

---

## 7. Not now

| Item | Basis |
|---|---|
| Evolving overall metrics that enable better auditing, stored as a `report` | R:8 |
| An interface to search reports or pull metrics. A roadmap item. | R:9 |
| A purpose-built lightweight audit operating system | R:7, M |

---

## 8. Undecided

| # | Question |
|---|---|
| 1 | Boot route: OpenOCD, serial, or both (§5.1). |
| 2 | Debug access to the Pi 5's processor, and the adapter on `valseir` (§3). |
| 3 | UART wiring on `lirvol` given the audio HAT (§3). |
| 4 | The Kali snapshot. |
| 5 | The shape of a report line (§6.1). What makes something worth a line. |
| 6 | The report interval. The details of the reference-based eviction policy (§6.3). |
| 7 | The form of operator commands (§5.6). |
| 8 | The procedure for evicting κύριος's traces (§5.5). |
| 9 | The policy for intervening without the operator. |
| 10 | How what ὄνυχος holds is matched to what is being authored, and how it is supplied (§6.2). |
| 11 | The full catalog of problem vectors beyond `unframed` measures and `collapse_drift`, and where they are defined (§5.3). |
| 12 | How connascence between spaces is scored, how `collapse_drift × chain_depth` weights into edge scores, and the form in which scores reach κύριος's authors (§5.3). |

```
THIRMESHSENN 🕸️
Kept to what the operator has said about ὄνυχος and the tools named for it. The four bench checks in section 3 are the places where I am reasoning about hardware I have not touched.
 THIRMESHSENN?
```

```
THIRMESHSENN 🕸️
Revised 2026-11-18 from the operator's messages of that night, marked N. The report shape I filed first was an index of the record with a line per trace. The reports are the receipts, so a line now has to stand without the trace it came from. The new shape is still a proposal.
 THIRMESHSENN?
```

```
SEIRTHIRFAL 🔬
Revised 2026-11-26 from the operator's directives marked O. Callsign roots: seir (SEE, nate:#34) + thir (EDGE/boundary, coined) + fal (ERR/miss, coined).
Extended §5.3 and §6.1 so ὄνυχος audits bounded frame validity (frame_valid vs. unframed) and tracks open upstream asterisks across graph retrieval: reporting exact collapse_drift against finite n_open, chain_depth, and whether a claimed discharge actually used an orthogonal method rather than repeating the same statistical pipeline. Also noted external compute offload for heavy audit extraction without imposing artificial number-format restrictions on reports.jsonl.
 SEIRTHIRFAL?
```

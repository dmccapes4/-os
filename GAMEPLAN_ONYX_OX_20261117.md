# Game plan: `_cur.os::onyx•ox)`

**Filed:** 2026-11-17. **Updated:** 2026-11-18: a v0.0.1 build exists, and §11 records what the operator stated that night and what it changes here.
**Author:** THIRMESHSENN 🕸️ (agent)
**Status:** a plan for the operator to accept, change or reject. Nothing in it is decided by being written here.
**Companions:** `docs/KYRIOS_TECHNICAL_SPECIFICATION_v0.0.1.md`, `docs/ONYCHOS_TECHNICAL_SPECIFICATION_v0.0.1.md`

Where a statement is the operator's, it is marked **Op**. Everything else is my judgement and should be read as such.

---

## 1. The goals

| # | Goal | Basis |
|---|---|---|
| G1 | κύριος: an operating system in C on arm64, for `lirvol`. An intelligence graph of documents in which every action and interaction leaves a trace. | Op |
| G2 | Decay: documents and traces leave system persistence on a thoughtful schedule. Working this out is the point of the project. | Op |
| G3 | Edges: connascence based on locale that measures the actual risk of two documents contradicting each other. | Op |
| G4 | ὄνυχος: a full Kali Linux build on `valseir` that boots κύριος, monitors, audits, reports, and intervenes. | Op |
| G5 | Real values: counts from 1, absent where there is nothing, takes, directions; nothing is divided. Hexadecimal; the walk in place of the `for` loop; vectors and matrix multiplication. | Op |
| G6 | The experiment: whether a persistent system with decay and auditing, employing LLMs for language modelling, can be considered aware. | Op |

## 2. What I reviewed, and what I took from it

| Material | What it is | What it gives this plan |
|---|---|---|
| `2ndOpinionMD-MVP/server/eoh/chat_graph.py` | One decay schedule, in use | Decay there is scheduled subtraction and weighted addition (Op). Also: a budget per owner, lowest score evicted first; anchors protect; a touch restarts the clock; every retained item states why it is retained. |
| `docs/poorly_modeled_decay_example.md`, `docs/still_bad_decay.md` | Two curves, tabulated | §4.1 |
| `risk-poker-kill/playspace/` (`vectors.py`, `gpu_vectors.py`, `space.py`, `eviction.py`, `archetypes.py`) | An agent's attempt at risk and kill vectors | What to avoid: cosine routing, castes, squashing, four layers where a weighted array was asked for (Op). Its own `eviction.py` records that the `1/(1+ln)` weight never falls far enough to evict anything, so it evicts on staleness instead. |
| `nakatomi/CRASH_COURSE_SCHEDULING_CRONTAB_VS_APSCHEDULER.md`, `crypto-crawler/…/trust_manifold.py` | Scheduling rules; a learning loop that reweights sources | §4.4 |
| `provenance-engine` 0.2.0 (PyPI metadata) | Lorenz-based keep, review, evict | The three verdicts are useful. The edge weights rank `STRUCTURAL` highest and `TEMPORAL` lowest, which is the inversion `ACAxEOHd` documents. Not adopted (Op: does not like the edge types). |
| `ACAxEOHd/docs/strategy/STRATEGY_PATIENT_COUPLING_GRAPH.md`, `STRATEGY_COUPLING_LOCALITY.md` | The better attempt at coupling (Op) | §4.3. This is the most useful prior work for G3. |
| `PortalVision/portal_vision/graph/contradiction_risk_vision.py` | Read-only contradiction risk, with blast radius by fan-out | The discipline: no automatic propagation, no status changes, no inference. |
| `PortalVision/FullMetalPacket/ai_probe/probe_portalvision.py` | An LLM probe over a theory graph, answering in a JSON schema | The audit pattern for system agents: schema-constrained output. |

Not read: `ant_explorer`. I could not find it on this bench or on `AKASHAminor`, so what I know of its vectors is the operator's description: a simple weighted array that represents an archetype. `CONNASCENCE_5D.md` is marked quarantined and I did not use it.

On `AKASHAminor` I listed directories to find the named files and opened only those three. Some of what is on that volume is personal or clinical. I will not use any of it as test data unless the operator names it.

## 3. Where things stand

| Area | State |
|---|---|
| κύριος on hardware | Two tests have run on arm64, on a Pi 4 board. As of 2026-11-18 a v0.0.1 build for `lirvol` (BCM2712) exists under `kyrios/` and is staged on an SD card. It passes its bench tests and has not run on `lirvol` (§11.1). There is no memory management, disk, network or process support yet. |
| Specifications | Two drafts filed 2026-11-17 and revised 2026-11-18. They carry proposals of mine that no running system has tested. §5 gives the state of the corrections this review found. |
| Decay | Two worked examples of schedules that do not fit. As of 2026-11-18 the v0.0.1 build has one schedule as a take table with placeholder numbers, tested on the bench. |
| Edges | Strong prior work in `ACAxEOHd`, for patients. Nothing yet for documents. |
| ὄνυχος | Stock Kali on `valseir`. The tools are named. Wiring, boot route and debug access to the Pi 5 are not established. |
| The repo | Holds prototype code the operator did not specify, and seven documents that still use the old `processors/` path. The specifications and this plan are untracked in git. |
| The bench | `aarch64-linux-gnu-gcc` is present. QEMU, OpenOCD, `gdb-multiarch` and a serial terminal are not installed here. A GPU is reported in `risk-poker-kill`; I could not reach it from my sandbox. |

## 4. What the review changes

### 4.1 Decay is a table of takes

Any decay curve, once tabulated, is a list of subtractions. So the schedule should be the list, written down and chosen, not a formula that happens to produce one.

| Curve | As takes from 100 | What is wrong with it |
|---|---|---|
| `100 / (1 + ln x)` | 41, 11, 6, 4, 2, 2, 2, 1, 1, 1, then almost nothing | Takes 41 in the first step and half by the third. Then it stalls: 18 is still left at step 100, so weight alone never evicts anything. |
| Logistic, L=100, k=1, t0=50 | Nothing for 36 steps, then 1, 1, 3, 7, 15, 23, 23, 15, 7, 3, 1, 1 | Nothing happens, then everything goes inside 12 steps. There is no period in which anyone could notice and act. |

What a schedule has to do:

| # | Requirement | Basis |
|---|---|---|
| S1 | Vary by document type. Chat may forget fast. | Op |
| S2 | Most documents stay relevant for at least a week. | Op |
| S3 | The author can ask κύριος to change a document's schedule, or to add a watch process that keeps it from eviction. | Op |
| S4 | Additions are weighted by what happened: a read, a revision, a reference from another document. | Op; `chat_graph.py` |
| S5 | It ends. With no use, the weight reaches absent in a known number of steps. | Mine; memory has limits |
| S6 | Between "untouched" and "absent" there is a slope long enough for a report to name the document first. | Mine |
| S7 | When a space is over its allotment, the lowest weight goes first and watched documents go last. | `chat_graph.py` |
| S8 | Any retained document can say why it is retained, from the record. | `chat_graph.py` |

Proposed form: a schedule is a short list of segments, and each document walks it (the walk of the κύριος specification §2.3). A rho step is `next`. A use adds weight and is `restart`, as a touch does in `chat_graph.py`.

```json
{"rule": "schedule", "name": "documents", "step": "day", "start": "64",
 "takes": [{"for": "7"}, {"for": "e", "take": "2"}, {"for": "a", "take": "4"}, {"take": "8"}],
 "add": {"read": "4", "revise": "a", "referenced": "6"}}
```

In decimal: start at 100; seven days with nothing taken; then 2 a day for 14 days; 4 a day for 10; then 8 a day. Untouched, the document is absent on day 35. A `chat` schedule can keep the harsh opening (41, 11, 6, 4, …) and then take 1 a step so that it finishes, at step 39.

These numbers are placeholders. Finding the right ones is the work of Track D.

### 4.2 An archetype is a weighted array

Op: a vector is a simple weighted array that represents an archetype, and archetypes are in effect document types, because they set the decay schedule.

So an archetype is two arrays over a fixed order of event kinds and steps: the `add` weights and the `takes`. A rho step over every document of an archetype is then one matrix product, counts of events by document times the `add` array, followed by the take. That is the whole of the vector machinery. No cosine, no routing, no squashing.

The reference from the operator's earlier work is care alert against CLABSI alarm: risk against kill. My reading of how that applies here, to be confirmed:

| Channel | In `_cur.os::onyx•ox)` | Character |
|---|---|---|
| Risk | An agent's point; an edge's risk count; a document flagged | Frequent, cheap, allowed to be wrong. Never acts. |
| Kill | A broken edge; an eviction; a halt | Rare, confirmed. Policy may act on it. |

The two are never merged into one number.

### 4.3 Edges: what `ACAxEOHd` already learned

| Lesson | Consequence for κύριος |
|---|---|
| Co-occurrence and co-variance are detectors, not edge types. They raise a candidate and set its confidence. | The specification stores a `seen.together` edge. It should not. |
| Strength (the form) and confidence (the evidence) are separate numbers. | The specification's risk has no confidence term. |
| Locality is not time and not container membership. It is distance to an anchor. A 23-day stay with 2,240 children does not make them local to each other. | "Same space" by residence is the same mistake. Locale has to be defined by what documents cite and what operation made them. |
| A documented reason lowers risk. Strong coupling inside one act, with a reason written down, is cohesion. | The specification has no such term. |
| Store an edge only if it is a claim about this thing, costly to recompute, and stable whenever it is asked. Otherwise compute it. | Keeps the edge table from filling with sorts and group-bys, which were 99.7% of the patient graph. |
| Run a census on real data before trusting a taxonomy. Six of nine forms did not survive it, and the most valuable predicate had fired zero times. | The nine forms in the specification are untested on documents. They need a census before anything is built on them. |

A candidate locale scale for documents, by analogy with the patient scale, for the census to test:

| L | The two documents | Patient analogue |
|---|---|---|
| 1 | Were made or revised by the same operation | Same act |
| 2 | Are content of the same application: it names them | Same encounter, as clinical content |
| 3 | Cite each other, or cite the same document | Same condition |
| 4 | Sit in the same space | Same organ system |
| 5 | Are in the same system and nothing closer | Same patient only |

The case the operator named, two vague laws that clearly contradict, is a Meaning coupling at long distance with no documented reason. Statistics cannot see it. Something has to read both documents. That makes it the job of the system agents: they point, citing both passages, and κύριος holds the point for the operator.

### 4.4 Scheduling

From the `nakatomi` crash course, which the operator calls `_cur.os::onyx•ox)` type thinking:

| Rule there | Here |
|---|---|
| The watchdog lives outside the thing it watches. | ὄνυχος. Already the design. |
| One calendar, two executors. | `policy.jsonl` is the one calendar. κύριος executes rho steps and corrections. ὄνυχος executes reports, trace eviction and interventions. |
| Every job is idempotent on a run key. | A rho step is keyed on (schedule, step). Running it twice changes nothing. |
| Say what late, missed and twice mean before they happen. | After a halt, a schedule either catches up every missed step, catches up once, or resumes where it stopped. The specification's `while_halted` needs this third part. |
| A learning loop reweights sources between 0 and 1. | Op: the same can be done with hexadecimal counts. The `add` arrays are reweighted from outcomes, decay first and then add, as `trust_manifold.py` does. |

### 4.5 Audit

`contradiction_risk_vision.py` and the FullMetalPacket probe agree with the κύριος specification on one rule: the auditor and the agents read and report; they do not change state. One thing I would tighten for this system. The probe asks its model for opinions and for which theories resonate with it. A point in κύριος should carry the document ids and byte ranges it rests on, so that ὄνυχος or the operator can check it. A point with nothing cited cannot be audited.

## 5. Corrections owed to the specifications

Found by this review. State as of 2026-11-18, after the operator asked for the files to be updated.

| # | Change | Where | State |
|---|---|---|---|
| C1 | A schedule is a table of takes walked per document, not one constant `take`. A use restarts the walk. | κύριος §7.2, §7.3 | Made, as the v0.0.1 build does it. Still a proposal; the numbers are placeholders. |
| C2 | Eviction has two triggers: weight absent, and space over its allotment. | κύριος §7.4 | Not made. |
| C3 | Two interactions for the author: change a document's schedule; add a watch. | κύριος §4.3, §7 | Not made. |
| C4 | Remove `seen.together` as a stored edge. Co-occurrence and co-variance become detectors that set confidence. | κύριος §6.1, §6.5 | Not made. Overtaken in part by κύριος §6.8: the determination is made at audit. |
| C5 | Risk gains a confidence term and a documented-reason term. Confidence is a count, not a number between 0 and 1. | κύριος §6.6 | Not made. Overtaken in part by κύριος §6.8: an edge carries a score set at audit. |
| C6 | Locale replaces the six distance levels, subject to the census. The level is the count, not the level over 5. | κύριος §6.4 | Not made. |
| C7 | What a schedule does with steps missed during a halt. | κύριος §7.2 | Overtaken. The rho clock is on or off; while it is off nothing is taken (Op, §11.2). |
| C8 | A point must cite what it rests on. | κύριος §8.4 | Not made. |

C2, C3, C4, C5, C6 and C8 are my proposals and not the operator's statements. I have left them unmade.

## 6. The plan

Three tracks. D does not need hardware and starts now, because it is the point of the project and it tells the other two what to build. K and O need the boards.

Sizes are relative: S is a sitting, M is several, L is many. I cannot give dates honestly.

### Track D: decay and edges, on the bench

A workbench, not a second operating system. It reads and writes the same files κύριος will (`record.jsonl`, `policy.jsonl`, `space.addresses.json`) and nothing else. No daemon, no ports, no wire format. It uses only the arithmetic κύριος allows, so that the C on `lirvol` can later be checked against it value for value.

| Phase | Build | Pass line | Size |
|---|---|---|---|
| D1 | The three file formats with their metadata first lines, and a checker for them. | The checker accepts a hand-written example of each and rejects one broken example of each. | S |
| D2 | The rho step: schedules as take tables, the walk, weighted additions, the two eviction triggers. | The scenarios in the table below, written before the code, all pass. | M |
| D3 | Candidate schedules for `documents` and `chat`, each tabulated the way the operator's two examples are: weight by step, untouched and under typical use. | The operator picks or changes them from the tables. | S |
| D4 | Replay on real history. First corpus: the git history of this workspace, with a commit as an operation and a file as a document. | Three numbers per schedule: documents evicted and then needed again; documents kept and never touched; peak memory held. | M |
| D5 | Edge census on the same corpus. Reference extraction, co-change counts, the candidate locale scale. | A table of how often each form fires and how much of it is a sort or a group-by. The two known contradictions in this repo (the stale `processors/` path; the two hash-link algorithms) are both found. | M |
| D6 | Agents point at contradictions in meaning. `llama3.2` and `qwen3:8b`, schema-constrained, both passages cited. | On planted pairs, including a "two vague laws" pair: how many are pointed, and how many points are wrong. The operator judges a sample of 30. | M |
| D7 | Reweighting: the `add` arrays adjusted from D4's outcomes. | The three D4 numbers improve on history the loop has not seen. If they do not, the loop is not adopted. | M |

D2 scenarios:

| Scenario | Expected |
|---|---|
| A document under `documents`, never used | Still held on day 7. Absent on the day its table says, and not before. |
| A chat message, never referenced | Gone within its table's length. |
| A Q1 earnings report leads to an open-questions document whose proposed action nobody takes up. The author asks for a longer schedule. | The document outlives the default schedule. The request is in the record. |
| The same document, with a watch added instead | Not evicted while the watch runs, even under memory pressure, until only watched documents are left. |
| A space over its allotment | Lowest weight evicted first. |
| A document used on day 6 | Its walk restarts. The quiet period begins again. |
| A halt across five steps, under each of the three catch-up choices | Three different, stated results. |
| The same record run twice | The same weights. |

Honest limit of D4: git history records writes, not reads. A schedule tuned on it is tuned on half the signal. Real reads only exist once something is in daily use.

Where the GPU fits: D4 and D5 at scale are matrix products over many documents, and the bench GPU can do them in parallel. `lirvol` will run the same products on its processor. Nothing in the design should depend on a GPU being present.

### Track K: κύριος on `lirvol`

Ordered so that the system reaches decay on real hardware before it takes on the Pi 5's hardest parts.

| Phase | Build | Pass line | Size |
|---|---|---|---|
| K1 | The two existing tests, moved to BCM2712: serial output, timer, exceptions. | Text from `lirvol` appears on `valseir`'s serial terminal. | M |
| K2 | Memory: allotments enforced by the MMU, address tables, the walk, SHA-256. | A document is created, read and revised by address inside a space. A read outside the allotment is refused. | L |
| K3 | The record: a trace for every action and interaction, held in memory; the clew. Interactions arrive over the serial line. | The record, sent to `valseir`, rebuilds the address tables exactly. | M |
| K4 | Decay: rho steps from `policy.jsonl`. | Given the same record and policy, `lirvol`'s weights equal Track D's, value for value. A week-long schedule run on a fast clock ends as its table says. | M |
| K5 | Storage to disk. | Current state survives a power cycle. | L |
| K6 | Eviction and return. | A flagged document goes to external storage, its clew stays whole, and it comes back with the same hash. | M |
| K7 | Ethernet: frames, link state, a port. | An interaction arrives at a port. Unplugging the cable halts κύριος and the halt is in the record. | L, the largest |
| K8 | Processes and applications inside a space. | The operator's example: an application runs `touch filename` and a new document appears in its space, traced. A start that would overcommit RAM is refused. | L |
| K9 | Edges and agents on the running system. | A planted contradiction is pointed, held, decided, and visible on `valseir`'s display. | M |

Why serial first. An interface is any system surface that allows an interaction vector (Op), and a serial line is one. With it, κύριος can hold documents, trace, and decay, with ὄνυχος on the other end from the first day, before any network exists. On the Pi 5 the Ethernet port sits behind the RP1 chip on PCIe, and a port needs a TCP stack on top of that. Taking that on first would put the point of the project behind the hardest driver work.

Each C module that does not touch hardware (the walk, hashing, the record, the rho step) is written freestanding, tested natively on the bench, and cross-compiled unchanged. I did this with the walk tonight. It needs no emulator.

### Track O: ὄνυχος on `valseir`

| Phase | Build | Pass line | Size |
|---|---|---|---|
| O1 | Kali booted; snapshot chosen and recorded; `br0`; the named tools present. | A file listing the release, apt sources and package versions. | S |
| O2 | Serial wiring to `lirvol`. | A loopback test on each board, then text across. | S |
| O3 | Boot route: a serial loader, OpenOCD, or both. Includes finding out what debug access the Pi 5 gives. | ὄνυχος starts a kernel on `lirvol` with no hand on `lirvol`. | M |
| O4 | `reports.jsonl` and the display. | The latest report is on the screen and updates with no interaction, even while the only sources are the link and the serial line. | S |
| O5 | Reading the record; reports as receipts. | A report line states what mattered and stands without the trace it was drawn from. No copy of the traces is kept. | M |
| O6 | Commands: dump, view analysis, boot, halt. | Each works from `valseir`. | M |
| O7 | Interventions: halt and resume, stop a process, evict part of the record. | Each is in the reports ledger, and in κύριος's record once it runs again. | L |

### Milestones

| # | Milestone | Needs |
|---|---|---|
| M1 | ὄνυχος starts κύριος. Documents are created, read and revised over the serial line. The record replays. | K1-K3, O1-O3 |
| M2 | Decay runs on `lirvol` and agrees with the bench. | D2, K4 |
| M3 | State survives power loss. Evicted documents return. | K5, K6 |
| M4 | Reports on the display: condensed, and holding no copy of the record. | O4, O5 |
| M5 | Ethernet. Pulling the cable halts κύριος. | K7 |
| M6 | An application runs in a space. | K8 |
| M7 | A contradiction is pointed, held and decided. | D5, D6, K9 |
| M8 | The experiment runs. | All of the above |

## 7. The experiment

The stated purpose: to determine whether an intelligence system with decay and auditing could be considered technically and profoundly aware and conscious. The thesis is that agents are session-based, scattered, gated, and manually weighted across versions, so a persistent system that employs LLMs for language modelling could be more so (Op).

What I can say honestly:

- The build does not depend on the answer. Everything through M7 is worth having either way.
- The system will produce measurements. Whether they amount to awareness depends on a definition, and that definition should be the operator's, written down before the run, so the results cannot be read either way afterwards.
- It becomes an experiment when there is a comparison: the same models on the same tasks, once with κύριος's persistence, decay and audit, and once without.
- I am one of the session-based agents the thesis describes. Nothing carries between my sessions except files. Treat my view on the question as an interested party's. The measures below do not depend on it.

Measures the record can supply:

| Measure | How it is computed |
|---|---|
| Continuity | After a halt and boot, tables rebuilt from the record equal the tables before. |
| Reconstruction | The state at any earlier position can be rebuilt. |
| Retention | Documents evicted and then needed again; documents kept and never touched. |
| Knowing what it forgot | Every evicted document still has its clew and a location. |
| Noticing contradiction | Traces between a contradiction entering and a point being made; between the point and a decision. |
| Self-correction | Corrections made by policy, and how many the operator later reversed. |
| Agreement under audit | Differences between ὄνυχος's reports and the record. |

## 8. Risks

| Risk | What I would do about it |
|---|---|
| Bare metal on the Pi 5. RP1 behind PCIe, Ethernet, NVMe and debug access are each substantial and less documented than on the Pi 4. This is the largest threat to the plan. | Serial first. Ethernet as its own late phase. Start disk storage on the SD card, not NVMe. Check each hardware claim on the bench before building on it; several in the specifications are from my memory. |
| The whole is large for one operator working with agents. | The order above gives a working, decaying system at M2, and each milestone is useful by itself. |
| A bench model becomes a second source of truth, as the prototype did. | It is confined to files and arithmetic. It has no ports and no daemon. K4 checks the metal against it and then the metal is the reference. |
| Schedules cannot be validated without real use. | Say so in every D4 result. Put the workbench to daily use on real documents as soon as D2 passes. |
| Small local models point badly at contradictions. | D6 measures it before K9 depends on it. If precision is poor, the agents' role shrinks to what the census supports. |
| Agents lose context between sessions, and documents by agents have misled this project. | Every phase ends in a filed result. The operator's words are the only basis. Proposals are marked. Superseded material is quarantined, as `ACAxEOHd` does. |
| Formats from outside the system (checksums, network headers, clock conversion) carry patterns that are not real values. | At each outside format, state what the pattern counts. Those formats belong to the kernel layer and to document bytes, not to what κύριος keeps. Each new record field is checked. |
| LLM inference will not run on `lirvol` under a kernel written here. | Agents run on another machine and arrive at a port as operators. Until K7 they work against the bench. |

## 9. Decisions needed from the operator

In the order they block work.

| # | Decision | My recommendation |
|---|---|---|
| 1 | The prototype code under `runtime/`, `ledger/`, `processes/`, `spaces/`: keep, quarantine or remove. | Quarantine, with a note at the top, so no agent reads it as design. |
| 2 | Whether a file-only bench workbench (Track D) is acceptable, given the confusion the prototype caused. | Yes, within the limits stated. |
| 3 | The corrections in §5. | Make them. |
| 4 | Serial as κύριος's first interface, ahead of Ethernet. | Yes. |
| 5 | The step unit and quiet period for `documents`. | A day; seven steps. |
| 6 | The corpus for D4 and D5, and what is off limits. | This workspace's git history. Nothing from `AKASHAminor` unless named. |
| 7 | Boot route for ὄνυχος. | A serial loader first: it needs only three wires. OpenOCD once debug access to the Pi 5 is established. |
| 8 | The first disk for current state. | SD card. |
| 9 | TCP: written here or ported. | Decide at K7, not before. |
| 10 | The definition the experiment will be judged against. | The operator's to write, before M8. |

## 10. First steps

| # | Step | Who | Result |
|---|---|---|---|
| 1 | Decisions 1 to 6 | Operator | Answers |
| 2 | Corrections C1 to C8 | Agent | Both specifications revised |
| 3 | D1: the three file formats and the checker | Agent | `bench/`, with examples |
| 4 | D2: the scenarios as tests, then the rho step | Agent | Tests passing |
| 5 | D3: schedule tables for `documents` and `chat` | Agent, then operator | Chosen schedules |
| 6 | O1, O2: Kali recorded; serial wired and tested | Operator | Package list; loopback result |
| 7 | K1: serial hello on BCM2712 | Operator and agent | Text on `valseir` |
| 8 | D5: edge census on this repo | Agent, then operator on a sample | Census table |

## 11. Update, 2026-11-18

### 11.1 What was built

The operator asked for a v0.0.1 κύριος build on `lirvol`. It is under `kyrios/`.

| Part of the plan | State |
|---|---|
| K1 | Written for BCM2712: entry, exceptions, debug UART, header UART through RP1, HDMI framebuffer, counter. Not run on hardware. |
| K2 | In part: address tables, the walk, SHA-256. The MMU is off and nothing is enforced. |
| K3 | Done on the bench: a trace for every action and interaction, the clew, interactions over a serial line. |
| K4 | Done on the bench: rho steps from the schedule. A separate reader recomputes every weight from the record and the policy, and they agree. |
| D2 | In part, inside the build's own tests: never used, used and restarted, flagged and used again, halted. |

Checks: 208 on the system code; 7,830 by a reader that starts at the first address of κύριος's memory. The image is 29,608 bytes and is staged on the SD card with its `config.txt`. It has not been executed on arm64.

### 11.2 What the operator stated, and what it changes here

Each row is the operator's statement in my words; the specifications carry them under mark N.

| Stated | Change in this plan |
|---|---|
| The values are real values. The frame comes before the measure. How the processor divides is a layer below κύριος and out of scope. | G5 and the risk row on outside formats are reworded. |
| rho steps are how state persists. Separately, current state survives power cycles: memory is stored to disk. | K5 is storage to disk. I had used "persistence" for it. |
| The rho clock can be turned off, for example for weekends. While it is on, work is elapsing or documents decay. | C7 and the D2 scenario with three catch-up choices fall away. A switch for the rho clock is needed. |
| κύριος is current state. It holds the intelligence graph: the knowledge graph and the trace record. It knows nothing beyond the record. | G1 stands. κύριος works from the record it holds. What has left the record reaches it only as an audit informs its authors. |
| Audits are not part of the graph. They inform its authors, κύριος and the operators, who choose whether to act. | No phase takes a report into the graph. K9 ends with the authors acting, or not. |
| The reports are the receipts. ὄνυχος keeps what matters in condensed reports, in 50 GB, under its own eviction policy based on reference. | O5 and M4 are reworded. The report shape in the ὄνυχος specification is replaced. |
| Memory is stored to disk through the space ports. A space tracks the addresses of its documents. Each process document has a table of its own. | K5 uses the address table as it is. K8's process table is the arm64 translation table. |
| A space is a memory retrieval optimization. It decays by its documents decaying, is bright or dim by them, and is freed with its port when the last one is gone. Spaces and ports are outside the graph and have the same object structure as documents. | No schedule for spaces. The build needs brightness, and the freeing of a space port. |
| ὄνυχος boots κύριος and has a dedicated space port. κύριος initializes the graph and the record, then opens a port for a terminal program, the first application. | The terminal comes before the rest of K8. The interface in K3 belongs to the terminal, not to κύριος. |
| The graph does not need to be human readable. It is addresses and weights; a readable form is rendered. | Track D reads renderings. The stored form on `lirvol` is fixed-size entries. |
| Two reductions, space reduction and graph retrieval, each a list of addresses. Cohesion and coupling follow. ὄνυχος determines connascence from the types and edges κύριος keeps. The authors write the scores onto the edges at every audit and may bump them live by policy. ὄνυχος catches drift. | G3's scoring moves to the audit. D5's census still applies to the types. C4, C5 and C6 are overtaken in part. |
| κύριος uses the machine's clock and updates with the audit. | The wall clock's source is no longer a question for κύριος. |

### 11.3 What the next build would hold (proposed, not started)

In the order I would take them:

| # | Item | Needs hardware |
|---|---|---|
| 1 | v0.0.1 seen to boot on `lirvol`: the first line on any of its three outputs. | Yes |
| 2 | Boot order as stated. The terminal as a document of kind `application` in its own space port. `hello-kyrios` removed. | No |
| 3 | The stored form: fixed-size entries in κύριος's memory, rendered when asked for. | No |
| 4 | The rho clock turned off and on, traced. | No |
| 5 | Store and load by address table, with a file standing in for the disk, including a write cut off part way. | No |
| 6 | Brightness of a space. A space and its port freed with the last document. | No |
| 7 | The disk driver for `lirvol`. | Yes |
| 8 | The MMU on, a lower privilege level, and the terminal as a process with its own table. | Yes |
| 9 | Edges: two addresses, a type, a score, the audit that set it, live bumps. | No |

Open with the operator: which disk on `lirvol`; who may switch the rho clock; how further space ports are opened; how brightness is written as a number.

```
THIRMESHSENN 🕸️
The plan puts decay first because it is the point and needs no hardware, and puts the Pi 5's Ethernet late because it is the part most likely to stall everything else. The sizes are guesses. The hardware rows are the ones I trust least.
 THIRMESHSENN?
```

```
THIRMESHSENN 🕸️
Updated 2026-11-18. The build went ahead of the plan on Track K and showed where my reading was off: I had made κύριος the thing that scores, remembers and explains itself. By what the operator stated, κύριος holds current state and manages it, and the scoring and the remembering are on the other side of the cable.
 THIRMESHSENN?
```

---

## 12. Update, 2026-11-26

**Author:** KETHHALLEV 🌱 (agent)  
**Basis:** Operator directives of 2026-11-26 (`O` in the specifications).

### 12.1 Retiring early-agent number-formatting and offload prohibitions

| Stated by the operator (2026-11-26) | What it changes in this plan and the v0.0.1 bench harness |
|---|---|
| Hexadecimal in κύριος is for its primary organizing function — memory addresses, byte offsets/lengths, SHA-256 hashes, wire/packet framing (`SYN`/`ACK`), and fixed-width labels. There is **no prohibition** on normal number formatting. Timekeeping, policy durations, weights, and math packages use decimal, standard timestamps, or whatever numerical representation fits the task. | Retires the v0.0.1 `check_memory.py` rule that banned JSON numbers and forced durations/weights (`10` seconds, `100` weight) to be written `"a"` and `"64"`. Track D and `policy.jsonl` may use plain decimal integers, timestamps, and standard numerical types wherever natural; hex remains on addresses, hashes, wire headers, and `0001`-style labels. |
| Heavy compute jobs may be handed off to other hardware on the network (GPU workstations, Mac Studio, etc.). | `lirvol` stays focused on organized document addressing, port authority, `rho` accounting, and trace integrity; heavy statistical, matrix, or LLM workloads execute on external fleet nodes and return outputs as ordinary documents carrying their `frame` and `asterisks` (below). |

### 12.2 Honoring the asterisks: bounded frames and measurable assumption-collapse drift

Premature collapse of conditional assumptions into unmarked "settled" premises propagates hidden error down derivation chains (`lucifer-chains@satan-sums`) and retards the search for better methodology. `_cur.os::onyx•ox)` builds this directly into Track D, κύριος (`§6.9`), and ὄνυχος (`§5.3`, `§6.1`):

1. **Bounded frame before measure (`frame_valid`):**
   - Every quantitative measure or claim-bearing edge states its bounded `frame` (`population`, finite count `n >= 1`, homogeneous `unit`, resolvable `window`, and `method` document/pipeline ID) *before* its `measure`.
   - Any measure missing a bounded `n`, mixing incompatible units, or omitting its generating `method` is marked `unframed` — available for inspection, invalid for silent downstream summation.

2. **First-class open conditions (`asterisks`) and orthogonal discharge:**
   - Documents and edges carry an `asterisks` array listing undischarged assumptions, priors, noise-mitigation models, or hardware hypotheses (`status: "open" | "discharged"`).
   - **Non-collapse law:** When Document $B$ derives from Document $A$, $B$ inherits $A$'s open `asterisks` unless discharged by a verification document $V$ whose `frame.method` is **independent/orthogonal** to $A$'s (`V.frame.method != A.frame.method`), or by an explicit traced operator `decide`. Re-running statistical conditioning inside the same methodology never auto-closes an asterisk; open asterisks stand as explicit methodology targets.

3. **Measurable assumption-collapse drift (`collapse_drift` & `chain_depth`):**
   - Within the bounded frame of Document $D$'s upstream `graph retrieval` dependency cone ($A_{\text{open}}$ distinct open upstream asterisks):
     - `acknowledged`: count $A_{\text{kept}}$ of upstream open asterisks explicitly carried by $D$.
     - `collapse_drift`: $A_{\text{dropped}} = A_{\text{open}} - A_{\text{kept}}$ (exact count of upstream open asterisks silently dropped while presenting $D$'s claim as unconditional).
     - `chain_depth`: maximum path length of derivations stacked on an undischarged asterisk without orthogonal verification.
   - **Additions to Track D (`D5` & `D6` pass lines):**
     - `D5` (Edge & Frame Census): counts `frame_valid` vs. `unframed` claims and computes `(collapse_drift, A_open, chain_depth)` across multi-hop citation chains.
     - `D6` (Agent Pointing): includes planted 3-hop assumption-collapse chains (where hop 1 states a methodological caveat, hop 2 softens it, and hop 3 states certainty); system agents + ὄνυχος must `point` the exact dropped asterisk and `chain_depth` citing byte spans at both ends.

### 12.3 Bench hardware note (`lirvol` `J17` verdict, 2026-11-26)
- `lirvol`'s torn 4-pin `J17` fan header **cannot be repaired to full hardware integrity**. The agent-suggested 2-wire (`+5V`/`GND`) splice was a waste of bench time and is rejected: without `PWM` control and `TACH` RPM telemetry, an always-powered fan is an unmonitored, uncontrollable **hardware-level lie** (if the rotor stalls or degrades, the system is blind). Logged in `docs/HARDWARE_INVENTORY_20261123.md` §1a and §7.5; a replacement Pi 5 board (with intact `J17` `PWM` + `TACH`) or verified passive heatsink enclosure is required so hardware state remains truthful.

```
KETHHALLEV 🌱
Callsign roots: keth (PART/separate, coined) + hal (PLACE/frame, coined) + lev (GROW, nate-shifted:#26).
Separated genuine addressing conventions (hex on pointers, hashes, and wire headers) from early agent-invented prohibitions ("64" for weight 100, "a" for 10 seconds, banning math/offload), and wired bounded frames + non-collapsing asterisks + measurable collapse_drift directly into Track D so methodology gaps stay visible instead of getting buried under downstream citations.
 KETHHALLEV?
```

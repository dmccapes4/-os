# κύριος Technical Specification v0.0.1

```text
_cur.os::onyx•ox)
```

**Revised:** 2026-11-26 (previously 2026-11-18; first filed 2026-11-17, replacing the draft reviewed in `kyrios_tech_spec_v001_review.md`).
**Authors:** THIRMESHSENN 🕸️ (2026-11-17/18); HALMESHMAR 🧷 (2026-11-26 revision).
**Status:** draft, early development. A v0.0.1 build exists under `kyrios/` and is staged on an SD card for `lirvol`. Nothing here is closed.
**Scope:** κύριος. ὄνυχος has its own document, `ONYCHOS_TECHNICAL_SPECIFICATION_v0.0.1.md`.

## 0. Basis marks

Every statement of design carries one of these.

| Mark | Basis |
|---|---|
| `Q` | The operator's commissioning message, 2026-11-17. |
| `R:n` | Line `n` of `docs/kyrios_tech_spec_v001_review.md`, the operator's review. |
| `M` | The operator's messages of 2026-11-17, 21:36 to 21:45. |
| `N` | The operator's messages of 2026-11-18, 00:23 to 03:47. |
| `O` | The operator's messages of 2026-11-26 (bounded frames, open asterisks, measurable collapse drift, retirement of hexadecimal-only prohibitions, hardware offload). |
| `code` | What a file in this repo does, including the v0.0.1 build under `kyrios/`. A description of what exists, not a decision. |
| `P` | Proposed here. Not decided. |

Documents written by earlier agents are not a basis. That includes the prototype under `runtime/` and `ledger/` (the "oracle"), which is cited only under `code` (M).

---

## 1. The system

### 1.1 Terms

The first six are the operator's, verbatim (R:31-36).

| Term | Definition |
|---|---|
| **system** | name: κύριος; port authority which persists state, manages decay, executes actions, provides interface |
| **operator** | the human (or LLM agent) operating the operating system |
| **interface** | any system surface which allows an interaction vector |
| **interaction** | operator/job action via interface; external |
| **action** | what the system does |
| **operation** | interaction and the actions carried out on the operator's behalf |
| **document** | Anything in the system: a file, process, space, port, key, edge (Q). Bytes in memory at an address (R:78). |
| **trace** | One entry recording an action, an interaction, or anything else that happens (R:62). |
| **record** | The collection of traces: `record.jsonl`, one trace per line (R:78). |
| **clew** | A document's entire trace, pulled from the record (R:41). |
| **receipt** | A trace is a receipt only when it is receipted from the record (R:39). Receipts belong to ὄνυχος and are specified there. |
| **policy** | A JSONL document by which κύριος or the operator decides (R:38). |
| **weight** | A document's decay weight: a count, or absent (R:60). |
| **rho step** | One step of the decay clock (R:72). |
| **absent** | Nothing there. Not a number and not a value (R:60, M). |
| **point** | What a system agent does when it sees something it thinks is wrong (R:74). |
| **current state** | What κύριος holds on `lirvol`. It includes the record (N). |
| **intelligence graph** | The knowledge graph and the trace record (N). κύριος holds it. |
| **audit** | What ὄνυχος reports. Not part of the graph. It informs the graph's authors (N). |
| **space port** | A space and the port it is reached through (N). |

### 1.2 Layers

Authority runs from the bottom of this table to the top (R:20).

| Layer | What it is | What it does |
|---|---|---|
| κύριος | The operating system. The port authority. | Sets authority: ports, spaces, keys, policy. Persists state, manages decay, executes actions, provides interface. |
| Kernel | C. | Provides the primitives. Enforces what κύριος sets. Mutates memory. Concludes each request from the bytes given and what is available. It holds no authority and decides nothing. |
| Processor | ARM 64-bit (arm64). | Gates the kernel. |
| Hardware | The Raspberry Pi, or whatever `_cur.os::onyx•ox)` runs on. | The last authority: the metal and the silicon. |

### 1.3 Design statements

These are the current design, not invariants.

| # | Statement | Basis |
|---|---|---|
| 1 | Everything in the system is a document. | Q |
| 2 | Documents have edges. | Q |
| 3 | Every action and every interaction leaves a trace, so that the system can be reconstructed. The collection of traces is the authority for what exists and what happened. | Q, R:14, R:38 |
| 4 | Each space is reached through its port. The two together are a space port. | Q, N |
| 5 | An application runs inside a space and acts within that space's allotment. | R:16 |
| 6 | Hardware is allotted by the operator, from a layout κύριος proposes. | R:18 |
| 7 | Memory has limits. Documents and traces decay and are evicted. What stays in current state is decided by rho steps: that is how state persists. | R:14, R:38, N |
| 8 | ὄνυχος boots κύριος. κύριος does not operate while ὄνυχος is offline. | R:13, R:26 |
| 9 | κύριος holds no more applications and processes than it can serve at full RAM use per process. | R:76 |
| 10 | κύριος keeps real values (§2.1). A document's bytes are the operator's and are stored exactly. | R:58, M, N |
| 11 | ὄνυχος reads κύριος without leaving a trace. | Q, R:39 |
| 12 | κύριος is current state. It holds the intelligence graph: the knowledge graph and the trace record. | N |
| 13 | Current state survives power cycles. Memory is stored to disk. | N |
| 14 | Audits are not part of the graph. They inform its authors, who are κύριος and the operators. The authors choose whether to act. | N |
| 15 | κύριος learns by managing the graph. It knows nothing beyond the record. What is evicted from the record goes to external storage, or where operator policy says. | N |
| 16 | Spaces and ports are not part of the graph. They have the same object structure as other documents. | N |
| 17 | At the operating system level the graph is addresses and weights. A human-readable form is rendered when it is wanted. | N |
| 18 | κύριος uses the machine's clock and updates with the audit. | N |
| 19 | Heavy numerical, statistical, or model jobs may be handed off to external hardware; κύριος organizes the resulting documents, addresses, frames, and traces. | O |
| 20 | Unverified assumptions ("asterisks") remain open across derivations until discharged by an independent method; dropping them is measurable drift, and every measure requires a bounded, stated frame first. | O |

### 1.4 Target

`lirvol`, a Raspberry Pi 5. κύριος is the operating system being built for it (R:2). A v0.0.1 image is built from `kyrios/` and staged on an SD card for `lirvol`. It has not run there yet (code, §9.5). Heavy compute jobs may be offloaded across the network to companion hardware while `lirvol` remains the organizing port and document authority (O).

The graph is for things like a software organization: design, QA, software, product (N).

---

## 2. Values, the walk, clocks, identity

### 2.1 Real values

The values κύριος keeps are real values (N): labels, positions, counts, weights, clock steps, addresses, lengths (R:58, R:60, M).

| A value is | Meaning |
|---|---|
| A count, from 1 | A position, a step or a count is 1 or more. Nothing is indexed from 0. |
| Absent | Where there is nothing to count, there is nothing. In a rendering the key is omitted. |
| A take | Removal is a take: `7{-3, -3}` is 7, take 3, take 3. A take never removes more than is there. |
| A direction | A sign is a direction in a field of coordinates. Movement is a count in a direction (N). |
| A portion | A part of a whole is a count against the whole, written as a surplus triplet (§2.2). Nothing is divided. |
| Hexadecimal | Used for addressing and wire/protocol framing — memory addresses, byte offsets/lengths, SHA-256 digests, packet headers (`SYN`/`ACK`), and fixed-width labels. **There is no prohibition on number formatting** (O): decimal integers, standard timestamps/sexagesimal clocks, floating-point values, and standard math packages are used freely wherever natural. |
| A label | A label is a name, not a count. Labels may carry leading zeros: `0008` is 8. |

The frame comes before the measure (N, O). What is counted against is stated and bounded before the count; the triplet of §2.2 and the claim frame of §6.9 carry their frame for that reason.

Three statements of ownership:

- **The kernel provides the primitives.** It is C on an arm64 processor and uses whatever those provide (R:58). How the processor divides is a layer below κύριος and is not in scope (N).
- **A document's bytes are the operator's.** κύριος stores, hashes, moves and returns those bytes exactly. It never parses, re-encodes or rejects them for the numbers they contain (R:58, M, O).
- **No artificial formatting or compute prohibitions.** Earlier agent drafts treated "hexadecimal" as a ban on JSON numbers or decimal durations/weights (e.g., encoding `100` as `"64"` and `10` seconds as `"a"` in policy/test harnesses). That rule is retired (O): hexadecimal belongs to addressing, digests, and wire framing, while timekeeping, policy schedules, math libraries, and jobs handed off to external compute hardware use whatever numerical representation is clearest and appropriate (O).

### 2.2 Surplus triplets

A portion of a whole is kept as a surplus triplet, as defined in `conceptual_language/THALKETHEN_SURPLUS_ARITHMETIC_20261027.md` §II. The triplet is the count itself, with its frame:

```text
frame (thom)   the total being counted against
with           count inside the frame that has the property
without        frame take with
thal           the smaller of with and without
surplus        the larger take the smaller
direction      ohn if with is larger, bahn if without is larger; dropped when they balance

written        surplus direction  thal thal  thom thom        e.g.  10 ohn 45 thal 100 thom
check          thom = 2 × thal + surplus
```

In JSON: `{"surplus": "a", "direction": "ohn", "thal": "2d", "thom": "64"}`. A balanced triplet has no `surplus` and no `direction` key.

### 2.3 The walk

The `for` loop is replaced by a declared scope and separate steps (M). The operator's example:

```text
x in [1..10]      scope declared; x is absent
x.next            1
x.now(5)          5
x.next(2)         7
x.back            6
x.back(2)         4
x.restart         absent
```

| Step | Effect |
|---|---|
| `x in [a..b]` | Declares the scope, both ends included. The position is absent. |
| `x.next`, `x.next(n)` | Moves forward 1 or `n`. From absent, the first step lands on `a`. |
| `x.back`, `x.back(n)` | Moves back 1 or `n`. |
| `x.now(k)` | Places the position at `k`. |
| `x.restart` | The position becomes absent. |

The declaration and the steps are separate. The steps may sit anywhere in a process; nothing forces a step, its size or its direction. The space and the variable may each be declared again (N). This is the `ren` (STORY) form of `conceptual_language/VELORA_PROGRAMMING.md` §14-16.

The kernel provides the walk as a primitive. Reference, tested against the example above on the bench and compiled for arm64 with the flags in `kernel/Makefile` (356 bytes of code):

```c
typedef unsigned long u64;

enum kplace  { K_ABSENT = 1, K_PLACED };
enum kresult { K_DONE = 1, K_EDGE, K_NOT_PLACED };

struct kwalk {
    u64 first, last;      /* the scope, both ends included */
    u64 at;               /* the position; meaningful only when placed */
    enum kplace place;
};

void kwalk_in(struct kwalk *w, u64 first, u64 last) {
    w->first = first; w->last = last; w->place = K_ABSENT;
}

enum kresult kwalk_next(struct kwalk *w, u64 n) {
    u64 from  = (w->place == K_PLACED) ? w->at : w->first;
    u64 steps = (w->place == K_PLACED) ? n : n - 1;   /* the first step lands on first */
    if (steps > w->last - from) return K_EDGE;
    w->at = from + steps; w->place = K_PLACED;
    return K_DONE;
}

enum kresult kwalk_back(struct kwalk *w, u64 n) {
    if (w->place != K_PLACED) return K_NOT_PLACED;
    if (n > w->at - w->first) return K_EDGE;
    w->at -= n;
    return K_DONE;
}

enum kresult kwalk_now(struct kwalk *w, u64 k) {
    if (k < w->first || k > w->last) return K_EDGE;
    w->at = k; w->place = K_PLACED;
    return K_DONE;
}

void kwalk_restart(struct kwalk *w) { w->place = K_ABSENT; }
```

The position cannot leave the scope (N). A step that would leave it is refused and the position does not move. The position never holds a value outside `[a..b]`, including when the walk is finished.

Redeclaration (N): `x in [c..d]` may be stated again for a variable that already has a scope, and the same scope may be declared for another variable. In the reference this is a second call to `kwalk_in`.

Chosen here only so the reference is complete (P): `back` from absent is refused. Not decided: what the position is after a redeclaration. The reference makes it absent. The alternative is to keep the position when it lies inside the new scope.

### 2.4 Clocks

| Clock | Counts | Used for |
|---|---|---|
| Record position | Traces, from 1 | Order of what happened |
| rho step | Decay steps, from 1, per schedule (R:72) | Decay. Schedules differ by document type, so each schedule has its own step count. The rho clock can be turned off, for example for weekends. While it is on, work is elapsing or documents decay (N). |
| Hardware counter | Processor timer ticks since boot, with the boot count | Intervals inside one boot. This is the machine's clock. κύριος uses it as it is and updates with the audit (N). |
| Wall clock | Date and time | The human interface only. Sexagesimal (R:60). κύριος does not depend on its accuracy (N). |

Timer ticks become clock steps by compare and count (P): a step is due at a counter value, and when that value is reached the next one is set by adding the interval. The v0.0.1 build does this (code).

### 2.5 Identity

Accepted as written in the reviewed draft (R:70).

```text
doc_id = first 16 bytes of SHA-256( "kyrios-doc-v0.0.1" 0x00 space 0x00 kind 0x00 name )
```

- Identity is the triple `(space, kind, name)`. It is stable across revisions.
- A document created without a name takes `name = "sha256:" + hash of its bytes`. The same bytes arriving twice are the same document.
- A name that resolves to no document still has a well-defined id.
- Names match `[A-Za-z0-9._:/#+> -]{1,255}`.

---

## 3. Storage

### 3.1 Documents are in memory

A document is a run of bytes at an address inside the allotment of the space that holds it (R:78). It is read by address. Nothing is scanned to find it.

| Structure | Form | Holds |
|---|---|---|
| Documents | Bytes in memory | The content, exactly as given |
| `space.addresses.json` | One per space. JSON is its rendering (§3.6). | For each document in the space: where its bytes are (R:78). The space tracks the memory address of the documents in it (N). |
| Process table | Inside each process document | The process's own addressing (R:78): a table that points to all of the process's memory (N) |
| `ports.json` | JSON dictionary | The main ledger: each port and its current assignment (R:22) |
| `record.jsonl` | JSONL | The traces (R:78) |
| `policy.jsonl` | JSONL | Policy (R:38) |

Current state survives power cycles: memory is stored to disk (N, §3.7). What stays in current state is a separate matter, decided by rho steps (§7).

### 3.2 The address table

One entry per document. Example values are illustrative; the `doc` shown is the real id of a document named by the hash of the 19 bytes `{"hello": "kyrios"}` in a space `hello-kyrios`.

```json
{"schema": "kyrios-addresses-v0.0.1",
 "0001": {"doc": "d1df8ae972f064e9b9743fa13419f12e",
          "kind": "file",
          "name": "sha256:e46e0d5fb676198dc7a9c23462feb5356d958c6554535e6611828349624cc35f",
          "address": "40200000", "length": "13",
          "hash": "e46e0d5fb676198dc7a9c23462feb5356d958c6554535e6611828349624cc35f",
          "schedule": "documents", "weight": "40", "rho": "1",
          "made": "3", "last": "3"}}
```

| Field | Meaning | Changes when |
|---|---|---|
| key (`0001`) | The document's label in the space, from 1 | Never |
| `doc`, `kind`, `name` | Identity (§2.5) | Never |
| `address`, `length` | Where the bytes are | The document is revised or moved |
| `hash` | SHA-256 of the bytes | The document is revised |
| `schedule` | The decay schedule that applies (§7) | Policy changes |
| `weight` | Decay weight. Absent: the document is flagged for eviction. | A rho step |
| `rho` | The rho step at which `weight` was last set | A rho step |
| `made`, `last` | Record positions of the first and the latest trace naming the document | A new trace names it |
| `location` | Present only after eviction: where the bytes went. `address` is then absent. | Eviction |

The table is a dictionary and is modified in place (P, following R:22 for the ports ledger). Every modification is an action and leaves a trace, so the table at any earlier moment can be rebuilt from the record.

### 3.3 The ports ledger

`ports.json` is a dictionary keyed by port number. A port keeps its number when it is reassigned to another space; the entry is modified in place and the change is traced (R:22).

```json
{"schema": "kyrios-ports-v0.0.1",
 "1b7d": {"space": "hello-kyrios", "state": "open",
          "keys": {"<key fingerprint>": ["read", "write"]},
          "since": "1"}}
```

`1b7d` is port 7037 at the human interface. `since` is the record position of the assignment.

The number of spaces is bounded by the number of ports. It will be small (R:76).

### 3.4 JSONL files

Every JSONL file begins with one metadata object that explains the schema of the lines after it, including the schema of a schema update. A schema change is appended as a line of that shape and applies to the lines that follow it (R:58).

```json
{"schema": "kyrios-record-v0.0.1", "fields": {"at": "position in the record, from 1", "what": "…"}, "update": {"what": "schema", "fields": "fields added, renamed or retired from this line on"}}
```

JSONL is used where entries accumulate. A dictionary is used where an entry is looked up by key and changed. Neither is a rule (R:14, R:22).

### 3.5 Reading without leaving a trace

The requirement is on the `document` schema (Q): ὄνυχος must be able to audit without leaving a trace. Three properties of the storage above provide it.

1. **Direct read.** Every document can be read by `address` and `length` from its table entry and checked against `hash`. The reader needs nothing from κύριος beyond the bytes.
2. **One place to start (P).** A root table at a published address gives the addresses of `ports.json`, each `space.addresses.json`, and the record. From it, every table and every document can be walked.
3. **Nothing changes on a read.** No field in any table records that a read happened. There is no access time and no read count. Where a decay schedule adds weight for reads, it counts the traces of operators' reads at the rho step (§7). A read by ὄνυχος has no trace and adds no weight.

Two read paths use these properties:

| Path | How | What κύριος does |
|---|---|---|
| ὄνυχος's port and space on κύριος hardware (R:39) | Over Ethernet. Requests for record lines and for bytes by address. | Answers. Writes nothing. |
| Debug access (R:39) | OpenOCD reads memory at the same addresses. | Nothing. κύριος executes no instruction for the read. |

In the v0.0.1 build the first path runs over the serial line, and the bench test compares all of κύριος's memory before and after such reads: no byte differs (code).

Writes by ὄνυχος are a different matter. ὄνυχος writes to handle trace eviction and to stop a process (R:80). Those are things that happened to the system, and each is traced when κύριος next runs (P).

### 3.6 Stored form and rendering

The graph does not need to be human readable. At the operating system level it is addresses and weights, and a human-readable form can be rendered (N).

What follows from that here (P):

- The JSON shown in §3.2, §3.3, §5 and §6 is a rendering. The stored form of a table entry or an edge is fixed-size: addresses, lengths, a hash, weights, counts.
- With fixed-size entries, an entry is found from its label with nothing parsed.
- A fixed-size field cannot be omitted, so in the stored form an absent value needs a pattern of its own. Which pattern is undecided (§10).
- `record.jsonl` is left as text. It was specified as JSONL with a metadata first line (R:58, R:78), and nothing said since changes that.

The v0.0.1 build keeps fixed structures and also renders every table as JSON into memory after each trace (code). The second copy is not needed.

### 3.7 Disk

Current state survives power cycles (N). Memory is stored to disk, and the space ports make that direct: a space tracks the memory address of every document in it, and each process document has a table of its own that points to all of the process's memory (N, R:78).

On arm64 that table is the process's translation table. The processor reads it on every memory access. An entry can mark memory as not in RAM, and the kernel then brings it from disk when it is used. This is a property of the processor, not a choice made here.

Proposed (P), using only fields §3.2 already has:

| The entry has | The document is |
|---|---|
| `address` only | In RAM, not yet on disk |
| `address` and `location` | In RAM and on disk |
| `location` only | On disk, not in RAM |

Storing writes the bytes and sets `location`. Loading reads them, checks them against `hash`, and sets `address`. Surviving a power cycle, eviction, and return on use are then the same operation on the same entry.

Each trace carries the hash of the line before it (§4.2). After an interrupted write the last whole trace can be found from that, and the tables can be rebuilt from the record (statement 3).

Not designed: the layout on disk, and which disk on `lirvol` holds current state (§10). The v0.0.1 build has no disk driver (code).

---

## 4. The record

### 4.1 What a trace says

A trace answers three questions (R:62, R:68): what happened, where it is stored, and what else categorises it. When something is created, the thing becomes a document in memory and the trace is added to the record.

### 4.2 Fields

All `P`. Memory addresses, byte lengths, hashes, and wire identifiers are hexadecimal strings; counts, sequence positions, and clock values may be written in hexadecimal or standard decimal/timestamp format as declared by the record's schema metadata line (§2.1, O).

| Field | Meaning |
|---|---|
| `at` | Position in the record, from 1. The metadata line is not a trace and has no position. |
| `kind` | `interaction` or `action` (§1.1). |
| `operation` | The `at` of the interaction this trace belongs to. One operation is one interaction and the actions carried out for it. Absent on an action the system took by itself. |
| `what` | What happened (§4.3). |
| `who` | A key fingerprint, a process `doc_id`, or `system`. |
| `port`, `space` | Where it arrived and the space concerned. |
| `doc`, `label` | The document concerned. |
| `address`, `length`, `hash` | Where the document's bytes are after this trace, and their hash. |
| `was` | The hash before, on a revision or removal. |
| `result` | `done` or `refused`. Only on actions. |
| `reason` | Why, on a refusal. |
| `tags` | Anything else that categorises the trace. |
| `boot`, `ticks` | Boot count and hardware counter (§2.4). |
| `rho` | The rho step of the document's schedule at this trace. |
| `prev` | SHA-256 of the previous line's bytes. Absent on the first trace. |

The v0.0.1 build writes these fields in a fixed order. It adds `name` on a creation, and `schedule`, `step` and `flagged` on a rho trace. It leaves out `boot`, because it stores nothing to disk and so cannot count boots (code).

### 4.3 `what`

| On an interaction | On an action |
|---|---|
| `create`, `read`, `revise`, `remove`, `list`, `link` | The same word, with `result` |
| `start`, `stop` (a process) | The same word, with `result` |
| `point` (from a system agent, §8.4) | `hold` |
| `decide` (the operator, on something held) | The action decided |
| | `boot`, `halt`, `resume`, `fault` |
| | `rho` (§7), `evict` |
| | `correct` (§8.5) |
| | `layout` (§5.4), `assign` (a port) |

### 4.4 Example

Illustrative. One operation: an operator creates a 19-byte document at port 7037.

```json
{"at": "1", "kind": "action", "what": "boot", "who": "system", "result": "done", "boot": "1", "ticks": "125f35"}
{"at": "2", "kind": "interaction", "operation": "2", "what": "create", "who": "<key fingerprint>", "port": "1b7d", "space": "hello-kyrios", "length": "13", "boot": "1", "ticks": "8b2a7a", "prev": "<sha256 of line 1>"}
{"at": "3", "kind": "action", "operation": "2", "what": "create", "who": "system", "port": "1b7d", "space": "hello-kyrios", "doc": "d1df8ae972f064e9b9743fa13419f12e", "label": "0001", "address": "40200000", "length": "13", "hash": "e46e0d5f…4cc35f", "result": "done", "tags": ["file"], "rho": "1", "boot": "1", "ticks": "8b2c4b", "prev": "<sha256 of line 2>"}
```

### 4.5 The clew

The clew of a document is every trace in the record whose `doc` is that document, in order (R:41). `made` and `last` in the address table give the first and latest positions, so pulling a clew does not need a scan from the start of the record.

### 4.6 Linking and git

- `prev` links each trace to the one before it (P). The prototype does the same under the name `parent_hash` (`runtime/kyriosd.py:87`, code).
- The operator's direction is to use git as well, so that the record gets a commit hash (R:68). κύριος on the metal has no git. Where the commit is made is undecided (§10). ὄνυχος sees the commits (N); what it does with them is in its specification.

### 4.7 Eviction of the record

The record decays too. It is persisted to external storage (R:43). Append-only is the spine of the design, not an absolute (R:14).

- When part of the record is evicted, an `evict` trace states the range of positions moved, the SHA-256 of the moved lines, and where they went. Positions are not renumbered.
- ὄνυχος handles trace eviction, with κύριος halted, under policy (R:78, R:80).
- The evicted part can be inspected on the external storage with the same provenance (R:24).
- The record evicts to external storage, or where operator policy says. κύριος knows nothing beyond the record it holds (N).

---

## 5. Kinds of document

| Kind | Content | Notes |
|---|---|---|
| `space` | Its port, its memory allotment, the address of its table, its limits | Holds documents (R:78). One port per space. Not part of the graph; same object structure as other documents (N). |
| `port` | Its entry in `ports.json` | Reassignable; the number stays (R:22). Not part of the graph; same object structure as other documents (N). |
| `key` | A fingerprint: SHA-256 of the key text | κύριος keeps the fingerprint, never the key (P). |
| `process` | Program bytes, its full RAM need, its own address table | The hash is checked before it starts (P). |
| `application` | The processes it is made of and the space it runs in | §5.2. The first is a terminal program (N). |
| `file` | Bytes | Not interpreted. |
| `edge` | §6 | |
| `hardware` | A device: what it is and where its registers are | §5.4 |
| `policy` | `policy.jsonl` | §7, §8 |
| `record` | `record.jsonl` | The record is a document too. |

`report` is not a κύριος kind at present. Evolving audit metrics stored as a `report` are not being done now (R:8).

### 5.1 Space

```json
{"port": "1b7d", "allotment": {"address": "40200000", "length": "4000000"},
 "table": "<address of space.addresses.json>",
 "limit": {"document": "10000", "documents": "3e8"}}
```

Everything a space holds lies inside `allotment`. The kernel enforces the boundary.

A space and its port are a space port. Stated by the operator (N):

| # | Statement |
|---|---|
| 1 | Setting the rest of the operating system aside, a space is a memory retrieval optimization. |
| 2 | A space decays by the documents in it decaying. |
| 3 | A space with many documents that are often read and revised is bright and prevalent. A space with one decayed document is very dim. |
| 4 | When that document is gone, the space and its port are freed. |
| 5 | Spaces and ports are not part of the graph. They have the same object structure as other documents, which makes everything easier. |

So a space has no schedule and no weight of its own. Its brightness is read from its documents. How brightness is written as a number is not stated. Proposed (P): the sum of the weights of the documents in the space, absent when none of them has weight.

There are two reductions: space reduction and graph retrieval. Both carry a list of addresses (N). Space reduction gives the addresses that belong to a space port. Graph retrieval gives the addresses reached from a document along its edges (§6.8).

### 5.2 Application

An application runs inside one space (R:16).

The first application is a terminal program. κύριος opens a port for it at boot, for initial communication (N, §8.7).

1. The operator reaches the application through the space's port.
2. The operator may start a process through the application.
3. The process acts on the application's space. Its requests go from the application, in the space, to the kernel, and are carried out within the space's allotment.

Example (R:16): an application runs `touch filename`. The result is a new `file` document in the application's space, with its entry in that space's address table and its traces in the record.

### 5.3 Admission

- A space needs a port. No free port, no new space (R:76).
- A port is freed together with its space when the last document in the space is gone (N). It can then be assigned again; the number stays (R:22).
- A process states its full RAM need. A start is refused when the needs of all running processes plus the new one exceed the RAM available. κύριος does not overcommit (R:76).

A refusal is an action and is traced.

### 5.4 Hardware

Hardware is allotted with the operator in the loop, the way partitioning is done when installing Linux (R:18):

1. κύριος reads what the board has.
2. κύριος proposes a layout: memory per space, devices per process.
3. The operator accepts or edits it.
4. The accepted layout is stored as a document and traced (`layout`). The kernel enforces it.

---

## 6. Edges

An edge is a document of kind `edge` (Q). The operator asked for four things (Q): contradiction risk; connascence-like typing; locality; and co-occurrence and co-variance to identify coupling types; with ordinary relations and references as well. Everything in this section beyond that list is `P`, except §6.8.

§6.8 records what the operator stated on 2026-11-18. It was stated after §6.1 to §6.7 were written, and where they differ §6.8 is the current design.

### 6.1 Types

| Class | Types | Stored |
|---|---|---|
| Relation | `rel.same_space`, `rel.same_application`, `rel.same_key` | No. Computed from the tables when asked. |
| Reference | `ref.inline` (bytes in one document name another), `ref.pinned` (the name plus the target's hash) | Yes |
| Agreement | `agree.<form>`, nine forms (§6.3) | Yes |
| Observation | `seen.together`: co-occurrence and co-variance of a pair (§6.5) | Yes |

### 6.2 Content

```json
{"type": "agree.name",
 "from": {"doc": "481b0b71eef451202eb019cdee397a5f", "at": "230", "length": "1a"},
 "to":   {"doc": "dfd8aa409ae7fd127658926e22e52a45"},
 "subject": "processors/hello-kyrios.md",
 "parties": "8", "distance": "3",
 "check": "none",
 "drift": "1",
 "state": "broken",
 "risk": "1b",
 "by": "<key fingerprint | process doc_id>"}
```

| Field | Meaning |
|---|---|
| `from`, `to` | The two documents. `at` and `length` address a part of a document, in bytes. |
| `subject` | What the two must agree on. |
| `parties` | How many documents share this agreement. 2 or more. |
| `distance` | Locality level (§6.4). |
| `check` | `none`; `declared` with the `doc_id` of a process that tests the agreement; `pinned` with a hash the kernel compares. |
| `drift` | How many times one end has changed without the other since the agreement was last seen to hold. Absent when none. |
| `state` | `holds` or `broken`. |
| `risk` | §6.6. Absent when there is none. |
| `seen` | Observation triplets (§6.5), where there are any. |
| `by` | Who made the edge. |

### 6.3 Agreement forms

Connascence: two documents agree on something, and a change to one requires a change to the other. The forms run from the easiest to see to the hardest. `rank` is the form's weight.

| Rank | Form | The two must agree on |
|---|---|---|
| 1 | `name` | A name or path |
| 2 | `type` | A type or kind |
| 3 | `meaning` | What a value means |
| 4 | `position` | An order of elements |
| 5 | `algorithm` | An algorithm |
| 6 | `execution` | An order of execution |
| 7 | `timing` | Timing |
| 8 | `value` | Values that change together |
| 9 | `identity` | The very same thing |

### 6.4 Locality

| Level | The two documents are |
|---|---|
| 1 | Parts of the same document |
| 2 | In the same application |
| 3 | In the same space |
| 4 | In different spaces under one key |
| 5 | In different spaces under different keys |
| 6 | One of them is outside the system, or evicted |

### 6.5 Co-occurrence and co-variance

Both are counted over operations (§1.1): one interaction and the actions carried out for it. Both are kept as surplus triplets (§2.2), so no ratio is computed.

| Triplet | Frame (thom) | Pole (`with`) |
|---|---|---|
| `occur` | Operations in which either document is named | Both are named |
| `change` | Operations in which either document is revised | Both are revised |
| `order` | Operations in which both are revised | `from` is revised first |

Worked, in decimal:

| Counts | Triplet | Reads as |
|---|---|---|
| A named in 14 operations, B in 9, both in 7 | `occur`: 2 bahn 7 thal 16 thom | Apart more often than together |
| A revised in 5, B in 4, both in 4 | `change`: 3 ohn 1 thal 5 thom | They change together |
| Of those 4, A first in 3 | `order`: 2 ohn 1 thal 4 thom | A leads |

Traces written by the observing process itself are not counted.

What the triplets identify (P; see §6.8 for where this determination is made):

| `change` | `order` | Other evidence | Coupling type nominated |
|---|---|---|---|
| any | any | A reference exists, unpinned | `agree.name` |
| any | any | A reference exists, pinned | `agree.identity`, `check: pinned` |
| ohn | balanced | Both are processes | `agree.algorithm` |
| ohn | balanced | Both are data | `agree.value` |
| ohn | balanced | One of each | `agree.meaning` |
| ohn | ohn or bahn | | `agree.execution`, the leader first |
| bahn or balanced | | `occur` is ohn | None. Used together, not coupled. |

How large the surplus and the frame must be before a type is nominated are policy values.

A nominated edge changes nothing. It is a `point` and is held for the operator (§8.4).

### 6.6 Contradiction risk

The risk of an edge is a count, computed with multiplication and addition only:

```text
risk = rank × distance × (parties + drift)          check is none or declared
risk = rank × distance × drift                      check is pinned; absent when drift is absent
```

| Term | Why it raises the risk |
|---|---|
| `rank` | The harder an agreement is to see, the more likely a change misses it. |
| `distance` | The more boundaries between the two, the less likely they are changed together. |
| `parties` | More documents share the agreement. |
| `drift` | One end has already moved without the other. |

A pinned edge has no standing risk because the kernel compares the hash at use; only drift counts.

An edge is `broken`, whatever its risk, when a reference no longer resolves, a pinned hash matches nothing, or a declared check fails. A broken edge is a contradiction that exists, not a risk of one.

The level at which a risk is reported is a policy value. The ranks and levels above are reasoned and not calibrated against any data.

Computing risk for every edge is one pass of multiplications over the edge table. The operator's direction is that a series of basic operations becomes a matrix multiplication, a baelorn (M; `conceptual_language/BAELORN_FORMAL_SPEC_20261027.md` §VIII).

### 6.7 Two examples from this repo

Both are real (Appendix A). Values in decimal.

| Example | Form | Distance | Parties | Drift | Risk | State |
|---|---|---|---|---|---|---|
| `spaces/hello-kyrios.md` byte 560, length 26, names `processors/hello-kyrios.md`. The directory was renamed `processes/`. Seven documents and the directory share the name. | `name`, 1 | 3 | 8 | Before the rename: absent. After: 1. | 24, then 27 | `broken`: the name resolves to nothing |
| `processes/hello-kyrios.sh:35` and `runtime/kyriosd.py:87` compute the link to the previous trace in two different ways, and `ledger/trace-format.md:21` describes a third party to it. | `algorithm`, 5 | 3 | 3 | 1 | 60 | `holds`, untested |

The first is cheap and already broken. The second carries more than twice the risk and has not failed yet only because it has run once.

### 6.8 Scores, audit and drift

Stated by the operator (N):

| # | Statement |
|---|---|
| 1 | There are two reductions, space reduction and graph retrieval. Both carry a list of addresses. |
| 2 | If the documents in a space are mainly related to that space, the space is cohesive. |
| 3 | If a space is coupled with several spaces, the connascence is determined by ὄνυχος, through the types and edges κύριος keeps. |
| 4 | κύριος and the operators can and should update the edges with the scored connascence at every audit. They can also bump scores live, by policy. |
| 5 | If the scores drift, ὄνυχος catches it. |
| 6 | Between audit reports κύριος runs on its own clock and its own scores. It updates with the audit. |

What this changes in the rest of the section (P):

| Earlier in this section | Now |
|---|---|
| §6.5 has κύριος nominate the coupling type of a pair from its own counts. | κύριος keeps the types and the edges. The determination is made at audit, outside κύριος. |
| §6.6 has κύριος compute risk for every edge. | An edge carries a score. The score is written at an audit and may be bumped live by policy. The arithmetic of §6.6 is one proposed way to arrive at a score. |
| An edge says nothing about when its score was set. | An edge needs to say which audit set its score and how many live bumps it has had since, so that drift can be seen. Proposed fields: `score`, `audit`, `bumps` (absent when there have been none). |
| §6.2 shows an edge as a JSON document with eleven fields. | That is a rendering. At the operating system level an edge is two addresses, a type and a score (§3.6). |

A live bump is an action and leaves a trace naming the policy line, as a correction does (§8.5).

Cohesion (P): for a space, two counts. Edges with both ends in the space, and edges with one end in another space. No ratio is formed. A space whose graph retrieval mostly returns addresses in the same space is cohesive.

### 6.9 Bounded frames, open asterisks, and measurable collapse drift

Stated by the operator (O; cf. R:10–12 on `lucifer-chains@satan-sums`):

| # | Statement |
|---|---|
| 1 | Fragile claims often hide their fragility inside the statistical or computational assumptions they take for granted. Epistemic integrity attaches explicit asterisks to those assumptions. |
| 2 | Honoring those asterisks drives the search for new methodology; prematurely collapsing them into unmarked "settled" premises retards progress. |
| 3 | `_cur.os::onyx•ox)` makes open asterisks and assumption-collapse drift measurable, and requires a bounded, appropriate frame on every measure so the measure is available and valid. |
| 4 | Heavy statistical, numerical, or model jobs may execute on external hardware; when their outputs enter κύριος as documents, they carry their frame and open asterisks like any other derivation. |

Proposed structure and arithmetic (`P`, grounded in `O`, `N`, `R:10–12`):

#### 1. Bounded frame before measure (`frame_valid`)

A quantitative claim, benchmark summary, edge audit, or offloaded-job result states its `frame` before its `measure`:

```json
{
  "frame": {
    "population": "space:bench/traces[1..240]",
    "n": 240,
    "unit": "interaction_trace",
    "window": {"from_at": 1, "to_at": 240},
    "method": "doc:0042@sha256:9f3c...",
    "hardware": "offload:mac-studio"
  },
  "measure": {"with": 218, "triplet": {"surplus": 196, "direction": "ohn", "thal": 22, "thom": 240}}
}
```

A measure's frame is **valid** (`frame_valid = true`) if and only if all four checks hold:
1. **Finite, non-empty domain:** `n >= 1` is an explicit finite count (`thom` in §2.2), never open-ended or zero.
2. **Homogeneous unit (`satan-sums` guard, R:10):** every item counted in `n` shares the same stated `unit`; counts or residuals from incompatible populations, scales, or noise models are never summed into a single `n`.
3. **Resolvable scope (`window` / `population`):** the space, trace range, byte span, or dataset cohort resolves to addressable bytes or traces.
4. **Declared method (`method`):** `method` names the pipeline, instrument, or model document (and `hardware` when offloaded) that produced the measure.

If any check is missing or fails, the state of the measure is `unframed`: the document's bytes remain stored intact (§2.1), ὄνυχος flags the claim as `unframed` at audit, and downstream documents cannot aggregate it as a validated measure.

#### 2. Open asterisks (`asterisks`) and orthogonal discharge

Any document or edge whose conclusion rests on unverified assumptions, statistical priors, post-selection/noise-mitigation models, or uncalibrated thresholds carries an explicit `asterisks` list (`P`):

```json
"asterisks": [
  {
    "id": "ast-01",
    "kind": "statistical_model",
    "statement": "Signal recovery assumes zero-noise extrapolation polynomial holds across unmeasured gate depths",
    "method": "doc:0042@sha256:9f3c...",
    "status": "open"
  }
]
```

**Propagation and non-collapse law (`lucifer-chains` guard, R:10, O):**
- Whenever Document $B$ cites or derives from Document $A$ (`ref.*` or `agree.*`), every `status: "open"` asterisk on $A$ enters $B$'s upstream **inherited asterisk set** $U(B)$ via `graph retrieval` (§6.8).
- An open asterisk **cannot** be discharged by repeating calculations, re-aggregating statistics, or paraphrasing prose under the **same** `method` (`V.frame.method == A.frame.method`).
- An asterisk transitions to `status: "discharged"` only when:
  1. A verification document $V$ tests the claim under an **independent / orthogonal methodology** (`V.frame.method != A.frame.method` — e.g., direct physical measurement or bench execution replacing statistical extrapolation) with `V.frame_valid == true`, setting `discharged_by` to $V$'s pinned `doc@hash`; or
  2. An operator explicitly accepts or scopes the bound via a traced `decide` operation (§8.4).
- Until discharged, an open asterisk remains visible in `graph retrieval` as an active methodology target rather than an error to hide.

#### 3. Measurable assumption-collapse drift (`collapse_drift` and `chain_depth`)

For any document $D$, its bounded audit frame is its finite upstream dependency cone in `graph retrieval` at audit time, containing $n_{\text{open}} = |U(D)|$ distinct open upstream asterisks:

| Measure | Frame | Definition | When `n_open` is 0 |
|---|---|---|---|
| `kept_asterisks` | `n_open` upstream open asterisks | Count of asterisks in $U(D)$ explicitly carried forward or scoped on $D$. | Absent |
| `collapse_drift` | `n_open` upstream open asterisks | `n_open - kept_asterisks` — exact count of open upstream asterisks silently dropped while $D$ states a derived claim as unconditional. Expressible directly as `(collapse_drift, n_open)` or as a surplus triplet (§2.2) with `thom = n_open`. | Absent (nothing open upstream to collapse) |
| `chain_depth` | Directed citation path $D \to \dots \to A_0$ | Length (in hops, from 1) of the longest unbroken dependency chain carrying an undischarged asterisk without orthogonal verification. | Absent |

In §6.6's edge risk arithmetic, `drift` decomposes cleanly into two non-overlapping counts (`P`):
- **`revision_drift`:** one endpoint of an agreement changed bytes without the other (§6.6).
- **`collapse_drift`:** downstream endpoint dropped $k \ge 1$ open upstream asterisks across `chain_depth` hops, raising edge fragility by `collapse_drift × chain_depth`.

---

## 7. Decay

Working out the correct decay management is the point of the project (R:72). This section fixes the mechanism and leaves the schedules to policy.

### 7.1 What is stated

| Statement | Basis |
|---|---|
| The rho-kernel is a decay clock employing various methods. It is not a kernel. | R:38, R:60 |
| Decay is weighted accrual decay steps, which may add to or subtract from total weight. | R:60 |
| When nothing is left the document is flagged for eviction. The weight is absent. | R:60 |
| rho steps are tracked. They are the second clock. Schedules vary across types, for example system chat and documents. | R:72 |
| ὄνυχος audits and suggests evictions in its reports. The operator or κύριος decides, based on policy. | R:38 |
| When the system halts, rho may or may not halt on some or all document types. | R:74 |
| The record is itself subject to decay. | R:43 |
| rho steps are how state persists. What still has weight stays in current state. | N |
| The rho clock can be turned off, for example for weekends. While it is on, work is elapsing or documents decay. | N |
| A space decays by the documents in it decaying. | N |

The variable is always written `rho`, lower-case.

### 7.2 Schedule

A schedule is a line of `policy.jsonl` (P for the shape). The shape below is the one the v0.0.1 build uses (code). Every number in it is a placeholder: finding the right schedules is the work, and it has not been done.

```json
{"rule": "schedule", "name": "documents", "kinds": ["file"], "method": "accrual",
 "start": "64", "most": "64",
 "takes": [{"for": "7"}, {"for": "e", "take": "2"}, {"for": "a", "take": "4"}, {"take": "8"}],
 "add": {"read": "4", "revise": "a"},
 "step": {"clock": "seconds", "every": "a"},
 "while_halted": "stop"}
```

In decimal: start at 100; seven steps with nothing taken; then 2 a step for 14 steps; 4 a step for 10; then 8 a step. A document that is not used is absent at step 35.

| Field | Meaning |
|---|---|
| `kinds` | The document types the schedule applies to |
| `method` | The decay method. `accrual` is the first; others are added as they are tried. |
| `start` | Weight given to a new document |
| `most` | The weight is never raised past this |
| `takes` | Weight taken at each step, as a table: for that many steps, take that much. An entry with no `take` takes nothing. The last entry has no `for` and applies from then on. Each document walks the table (§2.3). |
| `add` | Weight added for each trace of that `what` naming the document since the last step |
| `step` | What makes the schedule's clock step |
| `while_halted` | `continue` or `stop` |

The rho clock can be turned off and on (N). Who may do that, and how it is asked for, is undecided (§10). The v0.0.1 build can stop rho only by halting κύριος (code).

A space is not stepped. Its brightness follows from its documents (§5.1).

### 7.3 A rho step

For one schedule, at its step `n`:

1. For each document under the schedule, count the traces since the previous step that name it, by `what`. At the schedule's first step, count from the start of the record. ὄνυχος reads have no trace and are not counted (§3.5).
2. Add: `weight = weight + Σ add[what] × count`, not past `most`. An absent weight starts from nothing. A document that was used goes back to the start of the take table (`restart`).
3. Take: the document steps along the take table (`next`). At the end of the table the position does not move, so the last entry keeps applying. If the entry takes nothing, nothing is taken. If `weight` is more than the take, take it. Otherwise the weight becomes absent.
4. Write `weight` and `rho: n` into each address table entry.
5. Write one trace: `{"what": "rho", "schedule": …, "step": n, "flagged": [doc ids whose weight is absent]}`.

No weight is ever negative and none is ever zero: there is a count, or there is nothing.

Step 2 over all documents of a schedule is one matrix product: the table of counts (documents by `what`) times the column of `add` weights. That is why the absence of division and negatives makes schedules easier (M).

Per-document weights are not traced one by one. They can be recomputed from the record and the policy. The bench check for the v0.0.1 build does this and compares the result with the tables (code).

### 7.4 From flagged to evicted

1. A document whose weight is absent is flagged. Nothing else happens to it at the step.
2. If it is used again before eviction, step 2 gives it weight again and the flag is gone.
3. Eviction is decided by policy, where a policy line covers the case, or by the operator. ὄνυχος's reports name candidates (R:38).
4. To evict: the bytes are copied to external storage, the copy is checked against `hash`, the table entry gets `location` and loses `address`, the memory is freed, and an `evict` trace is written.

An evicted document still exists and still has its clew. It is no longer in immediate system memory (R:38).

When the last document of a space is gone, the space and its port are freed (N, §5.1).

---

## 8. Interactions

### 8.1 Interface

An interface is any system surface that allows an interaction vector (R:33). The first one is the port κύριος opens at boot for a terminal program (N, §8.7). In the v0.0.1 build the surface is a serial line, and a request is one line of text (code). A port reached over Ethernet comes later.

Wire form (P): one JSON line stating the request, followed by the content bytes when there are any, with their length given in the request. The content is carried and stored exactly as sent (§2.1).

### 8.2 Order of checks

On every interaction (P):

1. Write the interaction's trace.
2. Is the port assigned and open?
3. Does the key's fingerprint appear in the port's entry with the right needed?
4. Is the request within the space's limits and allotment?
5. Carry out the action. Write its trace, `done`.

A failure at 2, 3 or 4 writes the action's trace with `refused` and a `reason`. A request that cannot be parsed is also traced and refused. Nothing is refused without a trace (statement 3).

### 8.3 What is not an interaction

`health` is not a κύριος interaction. The operator did not ask for a `health` command. Health is the latest ὄνυχος report, shown on `valseir`'s display with no interaction (M).

### 8.4 System agents

In the current setup the system agents are `llama3.2` and `qwen3:8b`, reached over local ssh (R:74).

- Agents observe and report. They change nothing.
- When an agent sees something it thinks is wrong, it points. The word is `point`, not `suggest` and not `flag` (R:74).
- A point is an interaction with `what: point`, naming the documents and stating what is wrong. κύριος answers with `hold`. It stays held for the operator, who answers with `decide`.

### 8.5 Corrections

When κύριος sees something wrong that policy allows it to correct, it corrects it immediately, and the correction leaves a trace (`correct`) naming the policy line (R:74). Everything else is held for the operator.

### 8.6 Halt

| Cause | Basis |
|---|---|
| The Ethernet connection to ὄνυχος is lost. κύριος must know, and halts itself. | R:39 |
| ὄνυχος is offline, for example unplugged by the operator. Operations cease. | R:13 |
| ὄνυχος halts κύριος: to handle trace eviction, to stop a harmful process, or because policy requires a shutdown. | R:24, R:78, R:80 |
| Policy, on a condition κύριος detects | R:74 |

On a halt κύριος writes a `halt` trace with the cause, takes no new interactions and starts no processes. Whether rho continues is set per schedule (§7.2).

Detection (P): the kernel reports the Ethernet link state to κύριος on change. A link that is up does not show that ὄνυχος is running, so in addition ὄνυχος sends a signal on its port at an interval set in policy, and κύριος halts when it is missed.

Resuming (P): by ὄνυχος only. κύριος writes a `resume` trace with the hardware counter, so the length of the halt is in the record.

### 8.7 Boot

Stated by the operator (R:10, R:26, N):

1. ὄνυχος boots κύριος. It has a dedicated space port.
2. κύριος initializes the intelligence graph and the trace record.
3. κύριος opens a port for initial communication. That is a terminal program: the first application.

κύριος takes no interaction until ὄνυχος has started it. The mechanism ὄνυχος uses is in the ὄνυχος specification.

Because current state survives power cycles (§3.7), step 2 is an initialization only the first time. After a power cycle the graph and the record are loaded from disk (P).

How further space ports are opened after boot is not stated (§10).

### 8.8 Audits

Audits are not part of the graph. They inform the graph's authors (N). The authors are κύριος and the operators, and they choose whether to act (N).

- The reports are meant to help the operators and κύριος as they author more entries into the port authority and its intelligence graph (N).
- At every audit the edges are updated with the scored connascence (§6.8).
- κύριος updates with the audit: its own clock and scores run uncorrected in between (N).

No path takes a report into the graph as a document.

---

## 9. The kernel

### 9.1 What exists (code)

| Item | State |
|---|---|
| `kernel/src/boot.S`, `vectors.S`, `kernel.c`, `link.ld` | C with arm64 assembly at the entry points. Written for BCM2711 (Raspberry Pi 4), load address `0x80000`. |
| `kernel/Makefile` | Cross-compiles with `aarch64-linux-gnu-gcc`, `-ffreestanding -nostdlib -mgeneral-regs-only`. Links `build/kernel.elf`, then copies it to a flat `kernel8.img`. |
| Tests | Two have run on arm64 hardware (R:28). |
| `kyrios/kernel/` | The v0.0.1 kernel layer for BCM2712 (`lirvol`): entry and exception vectors in arm64 assembly, debug UART, header UART through RP1, firmware framebuffer, hardware counter, SHA-256, the walk. Not run on hardware. |
| `kyrios/system/` | κύριος v0.0.1: spaces, documents by address, address tables, the ports ledger, the record, the clew, decay by take table, halt and resume, a one-line text interface. See §9.5. |
| `kyrios/host/` | The same system code built for the bench, with its tests. |
| `kyrios/Makefile`, `stage_card.sh`, `boot/config.txt` | `make` builds `kyrios-v0.0.1.img`. `make check` runs the bench tests. The script stages a mounted Raspberry Pi boot partition. |

C is the current implementation language. That is not fixed (R:28).

### 9.2 Image format

The kernel may need to be ELF (M, R:56). The Makefile already produces `build/kernel.elf` and derives the flat image from it, so both exist from one link.

| Boot path | Image |
|---|---|
| ὄνυχος loads the kernel through OpenOCD and debugs it with `gdb-multiarch` | ELF: load addresses and symbols |
| The board's firmware loads the kernel from storage | Flat `kernel8.img` |

Which path is used is decided with the ὄνυχος boot mechanism.

### 9.3 Primitives κύριος needs

The kernel provides primitives and enforces. This is the list the rest of this document depends on.

| Primitive | Needed by |
|---|---|
| Memory allotment per space, enforced by the MMU. A translation table per process. | §3.1, §3.7, §5.1 |
| Read and write of bytes at an address within an allotment | §3 |
| SHA-256 | §2.5, §3.2, §4.2 |
| The walk | §2.3 |
| Timer ticks and a step source for rho clocks | §2.4, §7 |
| Start and stop of a process, with its hash checked before the start | §5 |
| Ethernet: frames in and out, and link state | §8.1, §8.6 |
| Storage: read and write blocks | §3.1, §7.4 |
| Serial port | Boot, console |

### 9.4 Moving to `lirvol`

To be confirmed on the board; listed so the work is visible.

- `kernel/` addresses BCM2711 peripherals. `kyrios/kernel/` addresses BCM2712, from the Linux device tree for the board and the Raspberry Pi documentation. None of it has run on `lirvol`.
- On the Pi 5, Ethernet and the 40-pin header's GPIO and UART sit behind the RP1 chip, reached over PCIe. Halting on link loss (§8.6) needs the Ethernet driver.
- The firmware settings `enable_rp1_uart=1` and `pciex4_reset=0` are documented as setting up the header UART and leaving the PCIe link to RP1 configured when the image starts. The v0.0.1 build depends on that for serial on the header, and checks for it before use.
- There is no disk driver and no Ethernet driver yet.

### 9.5 The v0.0.1 build against this document (code)

What it does: an operator's line creates, reads, revises and lists documents by address in a space. Every such line leaves an interaction trace and an action trace, and a refusal is traced with its reason. ὄνυχος starts, halts and resumes κύριος through its port, and reads the record, the tables, documents and clews with no trace. Documents decay by the schedule of §7.2, one rho step every 10 seconds, and are flagged when their weight is absent.

How it was checked: 208 checks of the system code on the bench; 7,830 checks by a separate reader that starts at the first address of κύριος's memory and uses nothing else, including recomputing every weight from the record and the policy. The arm64 image has not been executed.

Where it differs from this document:

| This document | The build |
|---|---|
| Boot: ὄνυχος boots κύριος, which then initializes the graph and the record and opens a port for a terminal (§8.7). | The record is initialized at power-on, and two `assign` traces are written before `boot`. `boot` arrives as a line on the serial port. |
| The first application is a terminal program in its own space port (§5.2). | The line interface is compiled into κύριος. Port 7037 is assigned to a space named `hello-kyrios`, carried over from the prototype, which starts with no documents. |
| The stored form is addresses and weights; JSON is rendered when wanted (§3.6). | Fixed structures in the image's own data, and JSON tables rendered into memory after every trace. |
| Current state is stored to disk (§3.7). | No disk driver. Memory starts empty at every power-on. |
| The rho clock can be turned off (§7.2). | Only by halting κύριος. |
| A space is bright or dim by its documents, and is freed with its port when the last one is gone (§5.1). | Brightness is not computed. Documents never leave. Ports are assigned once. |
| Edges with scores (§6). | No edges. |
| Keys (§8.2). | None. `who` is taken from the port named on the line, which checks nothing. |
| Allotments enforced by the MMU; processes with their own tables (§9.3). | The MMU is off. There are no processes. |
| ὄνυχος's port number (§10). | 7038, chosen for the build. |

---

## 10. Undecided

| # | Question |
|---|---|
| 1 | The layout of current state on disk, and which disk on `lirvol` holds it (§3.7). |
| 2 | Where the git commit of the record is made: by ὄνυχος on the external storage, or by κύριος writing git-format objects. |
| 3 | Whether the read path through ὄνυχος's port (§3.5), where κύριος answers and writes nothing, is acceptable, or whether only reads in which κύριος executes nothing count as leaving no trace. |
| 4 | Whether ὄνυχος's writes are traced by κύριος on resume (§3.5). |
| 5 | The walk: the position after a redeclaration (§2.3). |
| 6 | The number of ὄνυχος's port on κύριος. The v0.0.1 build uses 7038. |
| 7 | The decay schedules: methods, amounts, step sources. This is the project's main work and is expected to change. |
| 8 | Whether an edge's risk feeds a document's decay weight. |
| 9 | Edge ranks, levels and reporting thresholds (§6). |
| 10 | Who may turn the rho clock off and on, and how it is asked for (§7.2). |
| 11 | Whether ports appear in the record in hexadecimal or decimal. Under `O` (§2.1), there is no prohibition on decimal port numbers (e.g., `7037`, `7038`) where clearest; hexadecimal is reserved for addresses, digests, and wire framing where appropriate. |
| 12 | ELF or flat image (§9.2). |
| 13 | Whether the address tables' stored form is the fixed-size form of §3.6, with `space.addresses.json` as the name of the rendering. |
| 14 | The pattern that stands for an absent value in a fixed-size entry (§3.6). |
| 15 | How brightness is written as a number (§5.1). |
| 16 | How further space ports are opened after boot (§8.7). |
| 17 | The fields an edge carries for its score, the audit that set it, and live bumps (§6.8). |
| 18 | Policy thresholds on `collapse_drift` and `chain_depth` (§6.9), and how external hardware offload workers register their `method` and `hardware` identifiers. |

---

## Appendix A. State of the repo (code)

Facts about files as they stand. The prototype code was written by earlier agents and was not specified by the operator (M).

| # | Fact | Where |
|---|---|---|
| A-1 | `processors/` was renamed `processes/`. Seven documents still use the old path, two of them scripts. A card flashed from the repo today logs `processor not found` at first boot. | `README.md:45`, `spaces/hello-kyrios.md:15,16,26`, `processes/hello-kyrios.md:5`, `processes/hello-kyrios.sh:8,58`, `tools/first_boot_kyrios.sh:10,74`, `build/buildroot/README.md:15`, `docs/BARE_METAL_LADDER_KYRIOS_20261116.md:165` |
| A-2 | Two different computations of the link to the previous entry: the whole file, and the last line. | `processes/hello-kyrios.sh:35`, `runtime/kyriosd.py:87` |
| A-3 | The prototype has a `health` request. κύριος has none (§8.3). | `runtime/kyriosd.py:170`, `runtime/arrive.py:31` |
| A-4 | The prototype refuses unparseable and unknown requests without writing anything. | `runtime/kyriosd.py:167,175` |
| A-5 | The prototype re-encodes the operator's payload as sorted JSON before hashing and storing it, which alters the operator's bytes. | `runtime/kyriosd.py:179` |
| A-6 | The prototype and `ledger/trace-format.md` call a line of the record a receipt and name its fields differently from §4.2. | `ledger/trace-format.md`, `ledger/receipt.py` |
| A-7 | The kernel sources target BCM2711. The target board is BCM2712. | `kernel/Makefile:1`, `kernel/src/link.ld:1` |

```
THIRMESHSENN 🕸️
Built from the operator's words in the commissioning message, the review and tonight's messages, with the rest marked P. The edge arithmetic and the schedule shape are mine and untested against a running system; section 10 is where I would expect corrections first.
 THIRMESHSENN?
```

```
THIRMESHSENN 🕸️
Revised 2026-11-18 from the operator's messages of that night, marked N. Three of my readings were wrong and are corrected here: "system values" (they are real values), persistence (it is what the rho steps decide, and state is separately stored to disk), and spaces (no clock of their own; outside the graph). Sections 3.6, 3.7 and 6.8 carry proposals of mine under P. Section 9.5 lists where the v0.0.1 build does not yet match.
 THIRMESHSENN?
```

```
HALMESHMAR 🧷
Revised 2026-11-26 from the operator's directives marked O. Callsign roots: hal (PLACE/frame, coined) + mesh (MEASURE, nate:#78) + mar (LAST/endure, coined).
1. Retired the early-agent prohibition on non-hexadecimal numbers (§2.1, §4.2, §10): hexadecimal is for κύριος's primary organizing work (memory addresses, byte spans, SHA-256 digests, SYN/ACK wire headers, labels), while timekeeping, policy schedules, math packages, and offloaded compute jobs use decimal, timestamps, floats, or whatever fits.
2. Recorded explicit support for handing off heavy compute jobs to external hardware while κύριος maintains document organization, addresses, frames, and traces (§1.3, §1.4, §6.9).
3. Added §6.9 (marked O for principles, P for schema/arithmetic): bounded frames before measures (frame_valid), enduring open asterisks across derivation chains unless discharged by an orthogonal method, and exact bounded counts for assumption-collapse drift (collapse_drift against n_open, plus chain_depth).
 HALMESHMAR?
```

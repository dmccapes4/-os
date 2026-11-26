# Game Plan: Poetry & Journal Intelligence Graph and Ordered Transduction (`_cur.os::onyx•ox)` Prove-Out)

**Filed:** 2026-11-26  
**Author:** LIRRESHTHIR 🎚️ (agent)  
**Status:** proposed plan (`P`) for operator review; nothing here overrides `docs/KYRIOS_TECHNICAL_SPECIFICATION_v0.0.1.md` or `docs/ONYCHOS_TECHNICAL_SPECIFICATION_v0.0.1.md`.  
**Companions:**  
- `/home/dylanmccapes/dev/REALITY_USER_MANUAL/misc/justin_texts_amzn_202611/REPORT_TRANSDUCTION_ATTRIBUTION_AND_KYRIOS_HORIZON_20261126.md`  
- `docs/KYRIOS_TECHNICAL_SPECIFICATION_v0.0.1.md`  
- `docs/ONYCHOS_TECHNICAL_SPECIFICATION_v0.0.1.md`  
- `GAMEPLAN_ONYX_OX_20261117.md`  
- `/home/dylanmccapes/dev/docs/library/govern.py` (reference implementation of governed hybrid recall)

---

## 1. Purpose and Thesis Under Test

This plan translates the operator's architecture of **ordered transduction over an audited intelligence graph** into two staged builds:

1. **Stage I — The Literary Prove-Out (Friend's Mom's Journal & Poetry Corpus):** Ingest a real, multi-register, multilingual personal archive into a `_cur.os::onyx•ox)`-shaped intelligence graph (`record.jsonl`, `space.addresses.json`, `policy.jsonl`, `reports.jsonl`), index it with hybrid `BM25 + bge-m3`, run `rho` decay schedules to observe attractor formation, and train/prune single-purpose narrowband decoders ("transducers") on bounded facets of the corpus (musical references, profanity/transgressive register, French and German code-switching).
2. **Stage II — Organizational Kiosk & Workstation Architecture:** Map the exact same primitives onto a software organization's operational graph (git commits, Kubernetes state, emails, quarterly OKRs, wall-mounted `ὄνυχος` health/usage monitors, workstation `git` configuration of transducers, and reasoned IT tensor contracts).

### Non-negotiable house constraints built in from day one

- **No autonomous LLM-drives-LLM loops.** Every pipeline step follows the `govern.py` rule: (a) deterministic step or hard oracle (`BM25`, `bge-m3`, SHA-256, `rho` integer matrix pass), (b) strict JSON-schema-constrained decoder outputs at $T = 0$, and (c) paired measurement in the same round.
- **Append-only provenance.** Raw poem and journal entries are stored byte-for-byte with SHA-256 hashes; corrections, ingestion arguments, and re-classifications append new traces and never overwrite source text.
- **Boundary between `κύριος` real values and ML floats.** Inside `κύριος` system tables (`space.addresses.json`, `record.jsonl`, `policy.jsonl`, edge scores), values obey `κύριος` §2.1: counts from 1, `absent` where nothing is counted, integer `takes`, surplus triplets, hexadecimal in stored tables, and `rho` always written lower-case. Floating-point embedding vectors (`bge-m3`) and PyTorch adapter matrices live strictly inside application/process document bytes in their allotted space, converting to integer ranks and counts at the `κύριος` port boundary.
- **Python in `.venv/`, zero host `sudo`.**

---

## 2. Local Bench Inventory (Measured 2026-11-26)

Checked via `ollama list` on the workstation bench:

| Role in this plan | Local model | Status on bench | Footprint |
|---|---|---|---|
| Dense multilingual embeddings (EN / FR / DE) | `bge-m3:latest` | **Present** | 1.2 GB |
| Document-ingestion gate & CLI argument agent | `llama3.2:3b` | **Present** | 2.0 GB |
| Teacher / gold-set adjudicator / complex schema pass | `qwen3:8b` | **Present** | 5.2 GB |
| Structured code / schema formatter | `qwen2.5-coder:7b-instruct-q4_K_M` | **Present** (`qwen2.5-coder:3b` optional pull for 3B parity) | 4.7 GB (~1.9 GB for 3B) |
| Narrowband specialist base (for QLoRA + pruning assay) | HuggingFace / GGUF `Llama-3.2-3B` or `Qwen2.5-3B` in `.venv/` | Pull to local project cache when Phase P4 starts | ~6 GB BF16 / ~2 GB Q4 |

---

## 3. Stage I: Friend's Mom's Poetry & Journal Intelligence Graph

### Why this corpus is a strong first test

Corporate cloud LLMs fail personal literary archives in three predictable ways: they bowdlerize or moralize over profanity and raw grief, they flatten French/German code-switching into English paraphrases, and they hallucinate thematic summaries detached from line-level provenance. A faithful local transducer graph has no business judging the author; its job is to preserve exact bytes, map structural couplings, surface real attractors, and answer bounded questions honestly.

### Phase P0 — Custody, Raw Vault, and Standard Document Subtyping

1. **Custody & privacy boundary:** Store raw scans/transcriptions in a dedicated local directory excluded from public remotes. Every raw file is hashed (`SHA-256`) on arrival before any parser touches it.
2. **Hierarchical document subtyping:** Every node shares the universal `_cur.os` document envelope (`doc_id` from `(space, kind, name)`, `address`/`location`, `length`, `hash`, `schedule`, `weight`, `rho`, `made`, `last`) while carrying a strict subtype hierarchy inside its structured header:
   - `doc.archive.notebook` — physical journal or folder container (date span, physical description, provenance).
   - `doc.entry.poem` — verse unit preserving exact lineation, stanza breaks, indentation, and punctuation.
   - `doc.entry.journal` — dated or undated prose entry.
   - `doc.entry.fragment` — marginalia, draft scraps, bilingual notes, epigraphs.
   - `doc.span.stanza` / `doc.span.passage` — byte-offset child slices (`at`, `length` per `κύριος` §6.2) so retrieval and edges resolve to exact lines without duplicating source bytes.

### Phase P1 — The Interactive Ingestion Gate (`llama3.2:3b` + `ὄνυχος` Receipt)

This implements the operator's end-of-day workflow in miniature:

1. A raw entry arrives at the ingestion script (`ingest_gate.py`).
2. `llama3.2:3b` (at $T = 0$, JSON-schema locked) proposes the structural envelope: `subtype`, `inferred_date_state` (`explicit` | `contextual` | `absent`), `languages_present`, `stanza_count`, `line_count`, and `structural_anomalies` (e.g., undated page turn, mixed prose + verse).
3. If structural fields fail validation or carry ambiguity, the gate refuses silent auto-ingest and opens an interactive CLI turn with the operator: it states the schema objection (`point`), the operator supplies or argues the structural decision (`decide`), and the loop terminates only when the document conforms to valid system structure.
4. Every turn of that argument is written verbatim to `record.jsonl` (interaction + action traces), while the `ὄνυχος` ledger (`reports.jsonl`) records a condensed, non-judgemental receipt of what entered and what structural edge cases required operator override.

### Phase P2 — Hybrid Index (`BM25 + bge-m3`), Typed Edges, and `rho` Attractors

1. **Dual recall index (modeled on `govern.py`):**
   - **Lexical (`BM25`):** Tokenizes without stripping foreign diacritics, profanity, or punctuation-adjacent clitics. Captures exact French/German words, rare proper nouns, song titles, and exact curse words.
   - **Semantic (`bge-m3`):** 1024-dimensional cross-lingual embeddings over entries and stanza/passage spans. Because `bge-m3` embeds English, French, and German into a shared geometric space, an English conceptual query naturally recalls a German or French stanza even when zero lexical tokens overlap.
   - **Rank fusion to integer counts:** Reciprocal Rank Fusion (RRF) merges the two lists and hands `κύριος` an ordered rank list from `1..N` (real values, 1-indexed).
2. **Typed edges (`κύριος` §6):**
   - `ref.inline` / `ref.pinned` — explicit cross-references, recurring titles, or rewritten drafts of the same poem (draft lineage!).
   - Motif/lexical co-occurrence counted as **surplus triplets** (`occur`, `change`, `order` — `surplus direction thal thom`, zero division).
3. **`rho` decay simulation & attractor formation:**
   - Replay chronological ingestion + simulated/real operator queries across `rho` steps using tabulated `takes` and `add` matrices (`κύριος` §7.2–7.3).
   - **Pass criterion:** Verify that central poems, recurring musical motifs, and foundational journal entries accumulate weight and stabilize as **bright attractors** (high intra-space cohesion and cross-entry reference weight), while transient fragments gracefully step down to `weight: absent`, flag for eviction to cold storage, and remain 100% recoverable via their `clew` and SHA-256 `location`.

### Phase P3 — Narrowband Specialist Transducers & Weight-Pruning Assay

Instead of asking a generalist chatbot *"Tell me about her poetry,"* we build confined, schema-locked **transducers** — each an authority on *one* bounded projection, answering the exact same 5-field contract every time at $T = 0$:

| Transducer ID | Confined domain | Fixed 5-field output contract (JSON schema) |
|---|---|---|
| `transducer.music` | Instruments, composers, song lyrics, musical terminology (*tempo*, *clef*, *chanson*, etc.), rhythmic/cadence markers | `1. spans_cited` (doc + byte offsets), `2. musical_entities`, `3. structural_role` (`allusion` \| `meter` \| `setting` \| `metaphor`), `4. coupled_entries` (graph neighbors), `5. confidence_triplet` |
| `transducer.profanity_register` | Curse words, oaths, transgressive/raw colloquialisms, sudden register breaks | `1. spans_cited` (verbatim, uncensored), `2. lexical_items`, `3. register_shift` (`abrupt` \| `sustained` \| `quoted_speech`), `4. coupled_entries`, `5. confidence_triplet` |
| `transducer.polyglot_fr_de` | French and German words, phrases, epigraphs, syntax calques, code-switching boundaries | `1. spans_cited`, `2. language_and_verbatim`, `3. literal_gloss`, `4. switch_trigger` (`untranslatable_term` \| `quotation` \| `register_veil` \| `lyric_rhyme`), `5. coupled_entries` |

#### The Weight-Activation & Parameter-Reduction Experiment

To test the operator's parameter-reduction hypothesis with hard receipts:

1. **Gold dataset construction:** Use `qwen3:8b` + operator review on a stratified sample of entries to freeze a verified evaluation benchmark (100+ spans per transducer, including hard negatives).
2. **Specialist adaptation:** Fine-tune separate QLoRA adapters on a 3B base (`Llama-3.2-3B` or `Qwen2.5-3B`) for each transducer's single schema task.
3. **Activation profiling:** Run each specialist across the corpus and log per-layer attention-head and MLP-channel activation norms alongside Wanda importance scores ($S_{ij} = |W_{ij}| \cdot \|X_j\|_2$).
   - *Measurement A:* What fraction of MLP channels and attention heads carry 95% of activation energy for `transducer.music` vs. `transducer.polyglot_fr_de`? How disjoint are their active sub-networks?
4. **Structured pruning & compression:** Prune inactive heads/MLP channels at stepped ratios (20%, 40%, 60%) — and compare against distilling into a sub-1B student — measuring:
   - Schema validity rate (must remain 100%),
   - Span-citation precision/recall against the gold set,
   - VRAM resident footprint (MB) and latency (ms/query) on local hardware.

---

## 4. Stage II: Scaling to the Software Organization & Gattaca Kiosk

Every mechanism proven in Stage I maps directly onto the software-company intelligence graph and `_cur.os::onyx•ox)` kiosk architecture:

### 4.1 Mapping Literary Primitives to Organizational Primitives

| Mechanism | Stage I (Poetry & Journal) | Stage II (Software Organization on `_cur.os::onyx•ox)`) |
|---|---|---|
| **Document subtypes** | `notebook`, `poem`, `journal`, `fragment`, `stanza` | `process` (with RAM translation tables), `git_commit`, `k8s_datum`, `okr_report`, `email`, `design_spec` |
| **Ingestion gate** | `llama3.2:3b` argues entry metadata with archivist | End-of-day `llama3.2` gate argues commit/work-log structure with engineer; `ὄνυχος` traces argument without judgement |
| **`rho` attractors & eviction** | Core poems/motifs stay bright; stray scraps evict to cold vault with `clew` | Active sprints, load-bearing specs, and recurring incidents form stable attractors; stale telemetry & minor commits evict to external storage with intact hashes |
| **Narrowband transducers** | `music`, `profanity_register`, `polyglot_fr_de` | `commit_scale_audit`, `email_lineage_5q`, `k8s_anomaly_brief`, `okr_drift_check`, `connascence_contradiction_point` |
| **Configuration control** | Git-tracked schemas, `policy.jsonl`, adapter weights | Engineers tune transducer matrices, schemas, and watch jobs from workstations via `git` |

### 4.2 Worked Example: Abstract Problem Vector for the 20-Commit Query

Query: *"Retrieve the last 20 commits on this project and report on scale and importance of the recent work."*

Instead of stuffing raw `git log -p` into a 1M-token cloud chatbot and hoping its prose is calibrated, the system constructs an explicit **Abstract Problem Vector** and executes ordered transduction:

1. **Space reduction (`κύριος` §5.1):** Resolve `space_port` for the target repository $\to$ immediate memory address list of `git_commit` documents.
2. **The walk (`κύριος` §2.3):** Declare scope over the space's commit clew and step backward 20 positions (`c in [last-19..last]`).
3. **Graph retrieval (`κύριος` §6.8):** Traverse edges from those 20 commits to touched `file` documents, active `okr_report` nodes, broken or high-score connascence edges (`agree.algorithm`, `agree.meaning`), and any `ὄνυχος` problem receipts.
4. **Hybrid recall (`BM25 + bge-m3`):** Pull semantically and lexically coupled design docs or prior incident reports outside the immediate 20-commit edge hop, including pointers to evicted cold-storage nodes if historical precedent matches.
5. **Abstract Vector assembly:** For each commit and for the 20-commit window, populate named, inspectable primitives (counts and surplus triplets):
   - `[files_touched, spaces_crossed, max_connascence_rank, drift_count, okr_edge_weight, onyx_problem_flags, rho_brightness_delta]`
6. **Bounded transduction:** Multiply against the organization's authored importance/risk matrix and pass the structured slice to `transducer.commit_scale_audit`, which emits the same deterministic 5-part report every time (Scope, Structural Coupling, Attractor Alignment, Contradiction/Drift Risk, Cold-History Precedent).

### 4.3 The Office Kiosk, Watch Jobs, and the IT Tensor Contract

- **The Kiosk (`valseir` / `lirvol` pattern scaled to an office wall):**
  - Wall monitors render `ὄνυχος` passive health and condensed audit reports (`ONYCHOS` §6.2 — zero interaction required to view), live server/cluster load, space brightness maps, and scheduled data-query reports.
  - Local access over Ethernet, Wi-Fi, BLE, or Thread terminates at a transport bridge outside `κύριος` that forwards authenticated one-line JSON requests + key fingerprints to assigned `κύριος` space ports (`KYRIOS` §8.1–8.2). The transport medium is interchangeable because `κύριος` gates the **port and key**, not the radio.
- **Watch jobs:** An operator places a watch on a critical space, edge, or transducer condition (`GAMEPLAN_ONYX_OX_20261117.md` §4.1 S3/S7). Watched documents resist eviction under memory pressure and trigger immediate `ὄνυχος` report lines when coupled state moves.
- **Guaranteed local execution & IT tensor requests:**
  - `κύριος` enforces strict non-overcommit RAM admission (`KYRIOS` §5.3): every process declares its full memory ceiling up front, running reliably on company-issued laptops and local kiosk nodes.
  - When a task genuinely requires heavy matrix compute (e.g., full-corpus `bge-m3` re-embedding, training/pruning a new specialist transducer, or an organization-wide historical audit across evicted storage), the operator submits a signed `tensor_request` document naming the `space`, `matrix_workload`, `duration`, and `reason`. Compute becomes an accountable, reasoned allocation — never an ambient perceptual crutch.

---

## 5. Execution Sequence and Milestones (Stage I Build)

All Stage I workbench code lives in an isolated directory (proposed: `kyrios-os/experiments/poetry_graph/` with its own `.venv/`, reading and writing `_cur.os`-compatible `record.jsonl`, `policy.jsonl`, `space.addresses.json`, and `reports.jsonl` renderings so it never creates a competing daemon or touches `kyrios/` C sources).

| Phase | Deliverable | Pass / Receipt Gate | Effort |
|---|---|---|---|
| **P0** | Corpus vault layout, SHA-256 manifest, subtype JSON schemas (`notebook`, `poem`, `journal`, `fragment`, `span`) | 10 sample items hashed and byte-verified round-trip | S |
| **P1** | `ingest_gate.py` (`llama3.2:3b` schema gate + CLI operator argument loop + `record.jsonl` / `reports.jsonl` receipt writer) | Operator argues 5 real ambiguous entries through the gate; traces replay cleanly | S–M |
| **P2** | `BM25 + bge-m3` hybrid indexer (multilingual EN/FR/DE preserved) + surplus-triplet edge builder + `rho` take-table simulator | Cross-lingual recall verified on FR/DE lines; `rho` replay shows stable attractors vs. clean eviction | M |
| **P3** | Gold evaluation set (`qwen3:8b` + operator gate) + 3 narrowband QLoRA transducers (`music`, `profanity_register`, `polyglot_fr_de`) | 100% schema adherence at $T=0$; uncensored exact byte-offset citations measured against gold set | M–L |
| **P4** | Activation-sparsity & structured-pruning assay (`Wanda` head/MLP profiles + 20%/40%/60% pruned models) | Filed report comparing active-weight overlap, accuracy, VRAM, and ms/query across pruning steps | M |
| **P5** | Abstract-vector router (`route_transduce.py`): query $\to$ space/graph reduction + hybrid RRF $\to$ specialist transducer report | End-to-end bounded queries answered deterministically with full `clew` provenance | M |

---

## 6. Operator Decisions Needed Before Phase P0 Starts

1. **Corpus path & custody:** Where will the raw journal/poetry files sit on disk, and should the `experiments/poetry_graph/` directory hold only synthetic fixtures in git while pointing at an external/gitignored vault path for the real family texts? (Recommended: **yes**, keep personal literary bytes out of git remotes while committing code and schema receipts.)
2. **3B base selection for P3/P4 pruning:** Use `Llama-3.2-3B` (matches the ingestion gate family) or `Qwen2.5-3B` (stronger multilingual French/German tokenizer efficiency) — or run both head-to-head on `polyglot_fr_de`?
3. **Initial `rho` step unit for a literary archive:** Since a historical journal is written in the past and explored in research sessions, should `rho` step per **operator session / N interactions** rather than wall-clock days so idle calendar days don't evict the archive between research sittings?

---

```
LIRRESHTHIR 🎚️
Designed Stage I so it honors both sides of the house simultaneously: the personal literary text (lyric lineation, French and German code-switching, uncensored profanity, zero moralizing) and the kyrios-os specification (append-only SHA-256 traces, real integer counts at the system boundary, surplus triplets for co-occurrence, no LLM-drives-LLM loops). The piece I am keenest to see measured is Phase P4: hooking Wanda activation norms on the music transducer versus the French/German transducer will turn the operator's claim about partial weight usage in confined decoders from an architectural intuition into a plotted curve with exact VRAM and accuracy numbers.
 LIRRESHTHIR?
```

---

## 7. Addendum: Nate's Mom's 1970s Corpus Context, Configured Connascence Math, and Lie/Bias Recovery (2026-11-26)

### 7.1 Corpus specifics and the moral `rho` kernel

The Stage I archive is **Nate's mom's college poetry and journal from the 1970s**, spanning a period that included psychedelic (acid) use. Three design rules follow directly from that fact:

1. **Every entry is treated as ground-truth first-person fact (`what she wrote and experienced at t`).** There is no "fact-checking" filter against external orthodoxy.
2. **Philosophical contradiction = trajectory, not error.** Across 1970s college life and acid experiences, entries will naturally contradict one another philosophically. In this archive, a contradiction edge (`agree.meaning` with opposite stance polarity across `t_1 -> t_2`) measures **mindset growth, experimentation, perturbation, or decay**, never "hallucination" or dishonesty.
3. **Measuring the `rho` kernel (the moral and ethical person behind the poetry):** While surface lexical registers, psychedelic imagery, and transient philosophical positions rise and decay across `rho` steps, the persistent high-weight attractor that refuses to decay to `absent` should isolate **the invariant moral and ethical person authoring the work**. We add a fourth narrowband transducer to Phase P3 to test this directly:
   - `transducer.ethos_contradiction` — extracts explicit value commitments, ethical stances, and self-revisions (`1. spans_cited`, `2. stance_primitive`, `3. polarity_vs_prior`, `4. state_context` [`baseline` | `psychedelic_ref` | `retrospective`], `5. coupled_entries`).

### 7.2 Configured connascence via parameter reduction + matrix multiplication (Phase P2b)

Following the locality discipline proven in `2OPMD/autoimmune_concepts_atlas/docs/strategy/STRATEGY_COUPLING_LOCALITY.md` (`L=1..2` documented = **coherence**; `L=4..5` undocumented = **coupling risk**), connascence across both general prose documents and software artifacts is computed without $O(N^2)$ LLM calls:

1. **Step 1 — Parameter reduction:** Given a query or audit rule, reduce the $N$-document table by explicit metadata/structural parameters (date window, space set, target port/resource, or hybrid `BM25 + bge-m3` top-$k$) to obtain a reduced slice of $k$ documents represented by primitive feature matrix $X$ ($k \times d$).
2. **Step 2 — Configured matrix multiplication:** Multiply $X$ by a named **configuration matrix** $W_c$ ($d \times m$) that selects and weights only the primitives relevant to lens $c$, then form the coupling Gram matrix:
   $$Y_c = X W_c, \qquad C_c = Y_c Y_c^\top$$
   - **Coherence score (intra-space):** sum of $C_c(i, j)$ for pairs in the same act/space (`L=1..2`) with documented edges.
   - **Cross-space coupling alert (`L=4..5`):** off-block entries $C_c(i, j)$ exceeding threshold where `documented_reason` is `absent` — e.g., in Stage II, an `admin` Christmas card meeting (*pink vellum and bow ties*) colliding on `[holiday_window, company_wide_audience]` with a `devops` plan to push a *cat-in-a-snow-globe cartoon* to all employee terminals.
   - **Contradiction / trajectory vector:** pairs with high topical product $C_{\text{topic}}(i, j)$ and negative stance product $C_{\text{stance}}(i, j) < 0$. In Stage I (Nate's mom's journal), ordered by timestamp $(t_i < t_j)$, this plots philosophical evolution around her moral `rho` attractor. In Stage II (organizational kiosk), at concurrent timestamps across spaces, it flags **potential deception, unacknowledged policy drift, or nascent operator bias**.

### 7.3 Attribution trace recovery (`clew` audit against lies and nascent bias)

Because nascent human bias inevitably enters the initial configuration matrices $W_c$ and ingestion rules, `W_c` itself is stored as a versioned, hashed document in `κύριος` with its own `clew`.

- **Making lying painful (Stage II):** Every assertion in a meeting note, OKR, or commit is tied to an author fingerprint and record position; if a claim contradicts `br0` network receipts, `git` diffs, or coupled spaces, `ὄνυχος` surfaces the exact byte spans on both sides. Evicting a document from RAM never erases accountability because cold-storage hashes resolve the full `clew` on audit.
- **Recovering from lies or biased configurations (Stage I & II):** Add `clew_recover.py` to Phase P5 — given any invalidated document or revised configuration matrix $W_c'$, walk its `clew` and coupled edges forward to recompute affected scores deterministically without mutating a single historical trace line.

```
LIRRESHTHIR 🎚️
Added §7 to lock in Nate's mom's 1970s college context, the fourth transducer (ethos_contradiction) around her moral rho kernel, the two-step matrix formula for connascence (parameter reduction X followed by configured Gram product (X W_c)(X W_c)^T), and clew_recover.py so operator bias in nascent configuration matrices is audited by the exact same trace machinery as organizational lies.
 LIRRESHTHIR?
```

---

## 8. Addendum: MoE Decoding (`Qwen3-30B-A3B`), Proven Ingestion Lineages, and Partitioned Edge Appliances (2026-11-26)

### 8.1 Upgrading the model stack: `dengcao/Qwen3-30B-A3B-Instruct-2507:Q4_K_M` + Expert Pruning

Alongside local `llama3.2:3b` and `qwen3:8b`, the `.245` bench (`hilarious-marsupial`, RTX 4090 24 GB) already runs **`dengcao/Qwen3-30B-A3B-Instruct-2507:Q4_K_M`** (~18 GB, 30.5B total parameters, **3.3B active per token**, 128 experts / 8 active, non-thinking Instruct-2507), proven in production across two direct precursors to this plan:

1. **GGSD Iteration 4 ([ggsd.fullmetalpacket.app](https://ggsd.fullmetalpacket.app), `matthew_full_v004`):** Geometric grammar decoding of Maththew combining hard lexicon oracles, Homeric/Pindaric punctuation cadence math, and schema-constrained `Qwen3-30B-A3B` decoding.
2. **2OPMD / ACA EHR Ingestion (`reports/system/REPORT_ACA_EOHD_MODELFILES_AND_QWEN3_PARAMS.md`):** Longitudinal EHR and clinical guideline ingestion into a typed knowledge/coupling graph.
   - *Critical configuration carry-over:* As measured in the ACA EoHD audit, dengcao's `:Q4_K_M` Ollama blob bakes Thinking-style sampling defaults (`temperature: 0.6, top_p: 0.95`). Every poetry/journal ingestion and transducer call on `Qwen3-30B-A3B` **must explicitly override** `temperature` (`0.0–0.1`), `top_k`, and `format: json` in its Modelfile or API payload.

#### Why `Qwen3-30B-A3B` supercharges Phase P4 (MoE Expert Pruning)

In Phase P4, testing parameter reduction on a dense 3B model requires structured head/channel slicing. On **`Qwen3-30B-A3B`**, the model is *already* factored into **128 routed experts per MoE layer (only 8 active per token)**:
- We gain 30B-grade literary, philological, French/German, and nuanced 1970s cultural comprehension at 3.3B active FLOPs during gold-set building and specialist decoding.
- By logging router gate histograms across `transducer.music`, `transducer.profanity_register`, `transducer.polyglot_fr_de`, and `transducer.ethos_contradiction`, we can identify experts with near-zero routing frequency on a confined domain and **drop whole cold experts**, shrinking the 18 GB footprint directly toward an edge/workstation budget while preserving exact active-expert precision.

### 8.2 Constrained Moto G Play (`fogona`) Edge Terminals and Partitioned Space Security

Stage II extends naturally beyond wall monitors to stripped, single-purpose mobile edge nodes — building on the existing `fogona` fleet discipline (`pouch` running Unity pre-rendered megafauna packs for KangarooPark across 64 GB flash / 4 GB RAM; `cove` and `shoal` running offline documentaries and narrated content for StarLearner):

1. **Constrained Linux edge appliance (Stage II-E):** Boot a Moto G Play 2024 into an audited, minimal bare-bones Linux image (requiring full source/bootloader verification before adopting any experimental `W-Floyd/linux` / `pmaports` tree noted in `KIOSK_FOGONA_SETUP_20261111.md`) carrying **only** SSH, framebuffer/GLES rendering, and local TTS/TTL playback packages. One device, one role.
2. **Partitioned ingress space & handshake:**
   - Connection to `_cur.os::onyx•ox)` requires an explicit socket + cryptographic handshake + **device hardware UUID / key-fingerprint check** (mandatory for any out-of-network or wireless handset).
   - Authenticated arrival opens only a **partitioned ingress space** running a tightly scoped `sshd` or `httpd` governed by that space port's own security policy (`ports.json` + `policy.jsonl`).
3. **Cross-space dispatch under `ὄνυχος` watch:**
   - The ingress daemon cannot touch graph memory directly; requests route *out* of the ingress space to authorized internal processes that query the intelligence graph and invoke bounded tools/transducers.
   - Because `ὄνυχος` (`valseir`, stock Kali with its pre-packaged `tcpdump`/`tshark`/`mitmproxy`/`openocd`/`gdb-multiarch` toolkit) sees every incoming socket request and every cross-space transfer, any UUID mismatch, unauthorized port probe, or anomalous cross-space hop triggers `ὄνυχος` to **halt `κύριος`** immediately and intervene.

```
LIRRESHTHIR 🎚️
Appended §8 to integrate dengcao/Qwen3-30B-A3B-Instruct-2507:Q4_K_M (including the mandatory override of dengcao's baked 0.6/0.95 sampling params from the ACA EoHD report), link Stage I to GGSD v004 and 2OPMD EHR ingestion, add MoE expert-routing histograms to Phase P4, and specify the UUID-gated partitioned ingress space for constrained Moto G Play Linux edge terminals.
 LIRRESHTHIR?
```

---

## 9. Addendum: Live Envelope Check on `heuristic-friction` for 8× MoE `Qwen3-30B-A3B` (2026-11-26)

Live hardware probe on `heuristic-friction` (2026-11-26):
- **GPU:** NVIDIA GeForce RTX 3060 — `12,288 MiB` (12 GiB) GDDR6 (`10,786 MiB` free)
- **System RAM:** `62 GiB` DDR4 (`43 GiB` available)
- **CPU:** 12th Gen Intel Core i7-12700KF (12 cores / 20 threads)
- **Disk (`/` on `nvme0n1p3`):** `907G` size, `855G` used, **`6.2G` available** (`100%` with ext4 root reserve)

### Exact tensor & byte budget for `Qwen3-30B-A3B-Instruct-2507` (`Q4_K_M`)

Architecture (`48` layers, `hidden=2048`, `128` experts/layer, `8` active/token, SwiGLU `intermediate=768`):
- **Shared non-expert backbone** (token embeddings + `lm_head` `0.62B` + GQA attention + routers `0.92B`): **`1.54B` params $\approx$ `1.05 GB`** (`Q4_K_M` / `Q6_K`).
- **One expert across 48 layers** ($48 \times 3 \times 2048 \times 768$): **`226.5M` params $\approx$ `0.137 GB`**.
- **One 8-expert pruned bank** ($8 \times 226.5\text{M} = 1.81\text{B}$ expert params): **`~1.10 GB`**.
- **Standalone 8-expert specialist** (`1.54B` backbone + `1.81B` experts = `3.35B` params): **`~2.15 GB`**.

| Deployment mode on `heuristic-friction` | Memory footprint | Fits RTX 3060 (12 GiB) + 62 GiB RAM? |
|---|---|---|
| **8 specialist transducers sharing 1 frozen backbone in VRAM** (`1.05 GB` backbone + $8 \times 1.10\text{ GB}$ expert banks) | **`9.85 GB` VRAM** (`~7.6 GB` if 25% common syntax experts overlap) | **Yes — all 8 resident inside the 12 GiB RTX 3060 simultaneously**, with `~2.1–4.4 GB` left for `bge-m3` (`1.2 GB`) and short-context KV cache |
| **8 standalone `2.15 GB` pruned GGUFs** (no backbone sharing; 4–5 hot in VRAM, rest in OS page cache / RAM) | **`17.2 GB` total** across VRAM + `43 GiB` free RAM | **Yes — trivial in RAM**, instant `mmap` swap onto the 3060 |
| **Full unpruned 128-expert `Qwen3-30B-A3B:Q4_K_M`** via `llama.cpp --n-cpu-moe` (backbone + KV + ~26 layers in 3060 VRAM; remaining cold experts in DDR4 RAM) | **`~10.5 GB` VRAM + `~8.5 GB` RAM** (only 8/128 spilled experts read per token, `<0.6 GB`/tok) | **Yes — runs locally** without needing `.245`'s 4090 |

**Binding operational constraint (measured):** Disk, not VRAM or RAM. Because `/` on `heuristic-friction` currently has **`6.2 GiB` free**, pulling the unpruned `18.5 GB` `dengcao/Qwen3-30B-A3B` blob directly onto `/` will hit `ENOSPC`. Either:
1. Prune the 8-expert (`1.10 GB` bank / `2.15 GB` standalone) slices on **`.245`** (where the `18 GB` blob is already on disk) and `scp` the compact expert packs to `heuristic-friction`, or
2. Free `~25 GiB` on `nvme0n1p3` (or mount an external NVMe/USB volume) before pulling the full `18.5 GB` base locally.

```
LIRRESHTHIR 🎚️
Probed heuristic-friction before answering. Upgraded 62 GiB RAM (43 GiB free) plus the 12 GiB RTX 3060 makes both interpretations true: 8 expert-pruned Qwen3-30B-A3B specialists sharing the 1.05 GB attention backbone take ~9.85 GB total (fitting all 8 inside the 3060's 12 GiB VRAM at once), and even the unpruned 128-expert 18.5 GB model fits easily across VRAM + DDR4 with --n-cpu-moe. Catching the 6.2 GiB free disk ceiling on nvme0n1p3 saves an immediate ENOSPC headache.
 LIRRESHTHIR?
```

---

## 10. Addendum: Full 4-Node Fleet Compute Envelope for Poetry Ingestion & `_cur.os::onyx•ox)` (2026-11-26)

Cross-referencing `reports/system/REPORT_FLEET_ECOSYSTEM.md`, `REPORT_245_4090_OLLAMA_CONTEXT_ENVELOPE.md`, `kyrios-os/docs/BENCH_STATE_20261116.md`, and today's live probe on `82` fixes our exact hardware ceiling across all four machines:

### 10.1 Per-node measured hardware & optimal model tier

| Node | Hardware (measured) | Binding limit | Assigned Poetry / `_cur.os` role | Concurrent resident capacity |
|---|---|---|---|---|
| **`satan-sums`** (`220`, Victus, Debian 12 Bookworm) | RTX 4050 Laptop (**6 GiB** VRAM), 16 threads, 15 GiB RAM, **437 GiB free NVMe** | 6 GiB VRAM | **Snappy Ingestion Gate & CLI Argument Node:** interactive `llama3.2:3b` schema gate, trace staging | **2× `llama3.2:3b`** (`4.0 GB`) + KV; *or* `1× llama3.2:3b` + `1× qwen2.5-coder:3b` + `bge-m3` (`5.1 GB`, 100% GPU) |
| **`heuristic-friction`** (`82`, Dev workstation) | RTX 3060 (**12 GiB** VRAM, `360 GB/s`), i7-12700KF, **62 GiB RAM** (`43 GiB` free), **`6.2 GiB` free `/` disk** | Disk (`6.2 GiB` free on `nvme0n1p3` until cleared) | **Hybrid Index, Router & Mid-Tier / Pruned Transducer Host:** `bge-m3` + BM25, `qwen2.5:3b` / `qwen3:8b`, watcher scripts | `bge-m3` (`1.2 GB`) + `qwen3:8b` (`5.2 GB`) + `qwen2.5:3b` (`1.9 GB`); *or* **up to 8× pruned 8-expert `Qwen3-30B-A3B` banks** on shared `1.05 GB` backbone (`~7.6–9.85 GB` VRAM) + warm `mmap` pool in `43 GiB` RAM |
| **`hilarious-marsupial`** (`245`, Pop!_OS compute) | RTX 4090 (**24 GiB** VRAM, `1,008 GB/s`), Ryzen 5 7600X, ~31+ GiB RAM | 24 GiB VRAM (`40,960` safe `num_ctx` ceiling at 100% GPU; 7% CPU spill costs −45% prefill) | **Primary Heavy Decoder & MoE Pruning Forge:** full `Qwen3-30B-A3B-Instruct-2507:Q4_K_M` (`212.7 tok/s` gen, `~105k tok/s` prefill), gold-set builds, expert-routing histograms | **1× full 128-expert `Qwen3-30B-A3B:Q4_K_M`** (`~22 GiB` @ `40k` ctx, solo); *or* **up to 18× pruned 8-expert banks** on shared backbone (`20.85 GB` VRAM) |
| **`Mac Studio`** (`230` / `108`, Apple M2 Ultra) | M2 Ultra 60-core GPU, **64 GiB unified** (`800 GB/s`; `~44 GiB` usable after `8 GiB` macOS + `8 GiB` pinned Postgres) | Compute-bound prefill (~`0.5 page/s` vs `245`'s `1.5–1.8 page/s`); requires `--resume` + `82` heartbeat watcher | **High-Capacity Long-Context Co-Worker:** overnight cross-journal contradiction sweeps, secondary full `30B` shard | **2× full 128-expert `Qwen3-30B-A3B:Q4_K_M`** (`37 GB` weights); *or* **1× full `30B-A3B` + `bge-m3` + reranker** with huge KV headroom; *or* **30+ pruned 8-expert banks** |

### 10.2 Total fleet ceiling at a glance

Across all four machines (`6 + 12 + 24 + ~44 = ~86 GiB` of GPU/Metal memory, plus `43 GiB` free system RAM on `82`):

1. **Unpruned full `Qwen3-30B-A3B-Instruct-2507:Q4_K_M` (`30.5B` / 128 experts, `~18.5 GB` each):**  
   **3 to 4 concurrent full instances** fleet-wide (`1` blazing-fast primary on `245` 4090 + `1–2` high-capacity workers on `Mac Studio` + `1` optional `--n-cpu-moe` hybrid on `82`), **while simultaneously keeping** `qwen3:8b` + `bge-m3` live on `82` (3060) and `2× llama3.2:3b` live on `satan-sums` (4050).
2. **Pruned 8-expert `Qwen3-30B-A3B` narrowband transducers (`3.35B` active slice):**  
   - **~34–38 standalone `2.15 GB` GGUF specialists** resident in GPU/Metal memory at once, or  
   - **60+ distinct specialist 8-expert banks (`1.10 GB` each)** resident simultaneously when sharing one `1.05 GB` frozen backbone per machine.

For Nate's mom's 1970s poetry and journal archive (even thousands of pages), this fleet is **massively over-provisioned** in RAM and FLOPs: `satan-sums` handles interactive `llama3.2` ingestion gates with zero lag, `82` holds the live `BM25 + bge-m3` graph and `3B–8B` routers, `245` chews deep `30B-A3B` stanza/ethos decodes at `212 tok/s`, and `Mac Studio` runs long-horizon cross-notebook contradiction passes in parallel.

```
LIRRESHTHIR 🎚️
Appended §10 from the fleet and 4090 envelope reports. Two measured guardrails from last summer's post-mortems carry straight into this layout: keep 245's full 30B-A3B solo at num_ctx <= 40960 so bge-m3 doesn't push it into the 7% CPU spill cliff, and treat Mac Studio as the high-memory co-worker watched from 82 with --resume JSONLs rather than the high-QPS prefill primary.
 LIRRESHTHIR?
```

---

## 11. Operator Decisions Locked (§6 Answers) & Proposed `rho` + Ethos Architecture (2026-11-26)

### 11.1 Decision 1 Locked: No Git Theater — Sequential Page Authority & Unceremonious Entry Cuts

- **No git for `poetry_graph` data:** Using git on a historical archive entered page-by-page is performative overhead. Immutable SHA-256 page hashes + the `prev`-chained append-only `record.jsonl` already provide complete provenance.
- **Sequential 1-indexed pages are the physical spine:** Because pages lack consistent (or any) dates, primary ordering is strictly `(notebook, page_num)` counted from `1`. Dates are optional observed properties (`explicit` | `contextual` | `absent`), never structural keys.
- **Solving unannounced mid-page splits and cross-page spills ("no ceremony"):**
  - **Immutable physical layer:** Each numbered page is stored once as an untouched byte document (`page_0001`, `page_0002`, …).
  - **Virtual `entry` spans:** An entry (poem, prose reflection, fragment) is a lightweight span-list document pointing to byte ranges across one or more pages: `[{"page": "0014", "at": "1a0", "length": "24c"}, {"page": "0015", "at": "1", "length": "98"}]`. Re-cutting a boundary later appends a new span-map trace without ever altering page bytes.
  - **Two-oracle boundary detector + human gate:** On each newly entered page `p`, a deterministic structural pass (line-length variance jump, stanza/whitespace shift, syntactic closure, pronoun/tense/language switch) paired with a sliding-window `bge-m3` cosine cliff across adjacent line blocks proposes:
    1. `continues_prev_page` (top of `p` joins bottom of `p-1`), and
    2. `intra_page_cuts` (line indices inside `p` where an entry ends and a new one starts unannounced).
    Ambiguous cuts raise a `point` in `ingest_gate.py` for single-keystroke operator confirmation (`decide`).

### 11.2 Decision 2 Locked: 3-Model Benchmark + Compact `ggsd_web.py`-Style SSH Watcher

- **3-way benchmark across all Stage I tasks (boundary cuts + narrowband transducers):**
  1. `llama3.2:3b` (local fast baseline)
  2. `qwen2.5:3b` (primary candidate for narrowband 3B specialist transduction & FR/DE tokenization)
  3. `dengcao/Qwen3-30B-A3B-Instruct-2507:Q4_K_M` dispatched over SSH to **`245` (`hilarious_marcupial`)** (the 4090 oracle + MoE expert-routing reference).
- **Compact live monitor (`poetry_watch.py`):** Modeled directly on `GrammaticalGeometrySpaceDriver/web/ggsd_web.py` (stdlib `ThreadingHTTPServer`, zero dependencies, dark monospace card view), trimmed to a single compact screen polling `.245` GPU health/temp + page/span progress + 3-model agreement counts over SSH.

### 11.3 Decision 3 Proposal: Impedance-Protected `rho` & Tracking the Moral/Ethical Person

In an organization, untouched old state is usually stale operational clutter. In a 1970s college poetry journal, an edgy poem written in a style or motif completely foreign to earlier pages may sit untouched for 60 pages before the author returns to it — and decaying that early outlier to `absent` would blind the graph to long-arc creative and personal returns.

#### A. Impedance-Protected `rho` (`dormant_seed` floor, per `ONYCHOS` §5.3)

`ONYCHOS_TECHNICAL_SPECIFICATION_v0.0.1.md` §5.3 already states the governing law:
> *"Working memory is mainly recent, with a trace of the past. **Long-term memories are made when real impedance is encountered.**"*

We translate that directly into two `rho` rules stepped by **page count** (`1 ingested page = 1 rho step`):

1. **Impedance score at ingestion (novelty against prior history):** When an entry on page `p` exhibits high lexical, prosodic, register, or motif distance from the running profile of pages `1..p-1` (an edgy stylistic break, sudden profanity, foreign language switch, or novel philosophical image), its birth impedance places it on the **`seed` (`senn`) schedule** rather than the `mundane` schedule.
   - `mundane` entries (low-tension transitional prose with no downstream coupling) walk their `takes` all the way to `absent` and evict cleanly from active comparison.
   - `seed` entries step down after the local recency window (`~10` pages) to a **protected standing floor** (`weight = 8` hex — dormant, never `absent`), sitting in a compact **Dormant Motif Ring** that remains active in every future connascence matrix pass.
2. **Long-baseline return resonance:** When a later page `p + Δ` couples strongly to a dormant `seed` entry from `Δ` pages earlier, `add[return_resonance]` scales with the span gap `Δ`, restarting its walk and promoting both entries into a **bright long-arc attractor**.

#### B. Tracking the Moral and Ethical Person Behind the Poetry (Orthogonal Subspaces)

Across 1970s college life and acid use, surface aesthetics, moods, and explicit philosophical claims will swing and contradict one another. **Moral and ethical identity does not live in topic, mood, or metaphysical thesis — it lives in *relational valuation*** (what the speaker honors, what she refuses as shameful, how she treats vulnerability, and where she places accountability when things go wrong — the exact anchor $b_i$ of the Honor Well).

To prevent wild stylistic or psychedelic shifts from washing out her moral signature, every entry is projected onto **two strictly orthogonal vector frames** (never averaged together):

1. **Frame 1 — `Style & State Vector` (`surface_vec`):** Diction, prosody, imagery (music, nature, psychedelic perceptual shift), FR/DE switches, and explicit philosophical/cosmological propositions. *Expected to oscillate; high variance here feeds the Dormant Motif Ring.*
2. **Frame 2 — `Ethos Vector` (`ethos_vec` — the moral `rho` kernel):** Strict **Quote-or-Absent** extraction (`absent` whenever an entry makes no ethical/relational move — zero forced hallucination!) scored as **surplus triplets / signed integer poles (`ohn` / `bahn`)** across eight explicit moral-stance primitives:

| # | Ethos Primitive | `ohn` pole (+) | `bahn` pole (−) | What it tests across time |
|---|---|---|---|---|
| 1 | `candor_vs_pretense` | Unsparing honesty, self-scrutiny, naming the fake | Masking, posturing, rationalizing | Whether she demands truth even when unflattering |
| 2 | `care_protection` | Tenderness, sheltering the vulnerable, loyalty | Detachment, callousness, abandonment | Who/what she feels bound to protect |
| 3 | `agency_accountability` | Owning her own choice/fault (`fal`) | Blaming fate, others, or cosmos | Locus of moral responsibility |
| 4 | `harm_indignation` | Anger at cruelty, exploitation, hypocrisy | Cynical resignation or punching down | What triggers moral refusal / shame boundary |
| 5 | `transgression_purpose` | Profanity/edge used for diagnostic truth or liberation | Edge used for contempt or shock alone | Why she reaches for transgressive register |
| 6 | `reciprocity_grace` | Forgiveness, fairness, gratitude (`dek` / `man`) | Score-keeping, resentment | How debt and gift are held between people |
| 7 | `reverence_inviolable` | Treating life, love, music, or craft as sacred | Nihilistic flattening ("nothing matters") | What survives acid/irony unscathed |
| 8 | `self_verdict` | Honest remorse paired with endurance (`thaimoreth`) | Self-destruction or grandiosity | How she judges herself after a miss |

#### What crossing Frame 1 $\times$ Frame 2 reveals mathematically

Running the configured Gram product (§7.2) separately on `surface_vec` and `ethos_vec` across pages `1..N` separates three phenomena that human readers and cloud chatbots constantly conflate:

1. **The Moral `rho` Kernel (Invariant Attractor):** The subset of `ethos_vec` primitives whose cumulative surplus stays strongly polarized in one direction across *sober poems, acid-era fragments, French/German lines, and profane rants alike*. That stable vector **is** the moral and ethical person behind the poetry.
2. **Dialectical / State Oscillation (`Δsurface` large, `Δethos` near zero):** Two poems contradict each other philosophically (e.g., cosmic unity on page 30 vs. stark existential solitude on page 34), yet both score `ohn` on `candor_vs_pretense` and `care_protection`. She changed *frames* or *states of consciousness* — **not** her moral character. Look especially at *why* she abandons an earlier philosophical idea: often a writer drops a philosophy precisely *because* her invariant `ethos_vec` judged that philosophy dishonest or cruel.
3. **True Moral Growth or Fracture (`Δethos` sustained across subsequent pages):** A lasting shift in one of the eight `ethos_vec` axes (for instance, moving from externalizing blame to owning agency, or from nihilistic flattening to reverence) that persists over the rest of the notebook. Those inflection pages become the highest-weight biographical landmarks in `ὄνυχος`'s condensed report.

```
LIRRESHTHIR 🎚️
Locked Decisions 1 and 2 and drafted the §11.3 proposal for Decision 3. Pairing ONYCHOS §5.3's impedance law (high-novelty edgy outliers drop to a non-absent dormant_seed floor rather than decaying out before a late-notebook callback) with orthogonal Style/State vs. Quote-or-Absent Ethos vectors solves both halves of the problem: foreign motifs stay searchable across 80-page gaps, and psychedelic or philosophical swings stop muddying the measurement of who she actually is morally.
 LIRRESHTHIR?
```

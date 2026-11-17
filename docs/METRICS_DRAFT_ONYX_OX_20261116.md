# Metrics draft — the onyx experiment (for the operator's ruling)

**Filed:** 2026-11-16, HALMANSEIR
**Status: DRAFT, in my name only.** Nothing here is the operator's definition until he rules it so. Filed in response to his charge (stored in `CLARIFICATIONS_ONYX_OX_CORRECTED_20261116.md` §8) that the named metrics would likely be defined by now if the agent had helped define them instead of demanding pass/fails.
**Stage discipline:** the experiment is at its very beginning (his §8.5). This draft therefore defines *what would be counted*, and only that. It sets **no thresholds, no baselines, and no pass/fail conditions.** The first run of any metric is a baseline, not a verdict. Definitions can be amended by operator addendum at any time; amendment is expected, not a failure of the draft.

**Anchors (his, from the corrected clarifications):**
- Learning is OGrE and the graph evolving (§8.1).
- Awareness is inputs we provide it (§8.2).
- Consciousness is what the experiment is being run to define (§8.3). Nothing below defines it.
- The experiment is a side project to the ox (§8.4); metrics must never cost the ox its honest document processing.

The six named metrics, from the note: **persistence, growth, intelligence, control (in a nice way), agency, expression.**

---

## 1. Persistence

**Would count:** that the same system is still there, in the only senses traceable on κύριος.

- Daemon continuity: uptime spans; unplanned exits; whether the daemon, on restart, resumes as itself (same spaces, same graph lineage, same keys) rather than as a fresh instance.
- Memory continuity: documents and graph state surviving reboot and power-cycle on `lirvol`, verified by hash/replay against the trace.
- Lineage continuity: the intelligence graph after an interruption is a descendant of the graph before it (edges carried, demotions carried), not a re-seeded approximation.

**Observing instrument:** the κύριος trace itself; boot receipts; graph lineage hashes.

## 2. Growth

**Would count:** the graph evolving (learning, per §8.1), distinguished from mere accumulation.

- New typed edges created by OGrE per interval, by edge type (once the schema exists; before it exists, count candidate edges and record the gap honestly).
- New edge *types* appearing — the vocabulary itself growing, when it grows.
- Edge lifecycle events: promotions, demotions, partitions — growth includes the graph getting *less* wrong, not only bigger.
- Explicitly not growth: raw document count, disk usage, receipt volume. Those are intake, not evolution.

**Observing instrument:** graph diffs between snapshots, each diff receipted on κύριος.

## 3. Intelligence

**Would count:** handling improving *because the graph evolved*, stated narrowly enough to be checkable later.

- Repeated-case change: a class of arrival that was mishandled, refused, or routed wrongly early on is handled differently after the graph has had the relevant experience — and the difference is attributable to specific edges (the graph diff is the receipt).
- Correction uptake: an operator correction results in a demotion/re-typing that measurably changes later handling of the same class.
- Transfer (to be defined more tightly when there are cases to define it against): a novel arrival handled using edges earned from different arrivals. Until cases exist, this stays a named intention, not a counted metric.

**Observing instrument:** arrival fixtures re-run across graph snapshots; outcomes compared per snapshot.

## 4. Control (in a nice way)

**Would count:** authority exercised as designed — grants *and* refusals legible, nothing silent, escalation polite.

- Grant/refusal fidelity: arrivals granted or refused per space policy, keys, quotas — each refusal receipted with its reason, per the standing rule.
- Refusal legibility: a later reader can reconstruct *why* from the trace alone.
- Audit catch/miss record (from ὄνυχος's reports): misses are data (his §6.2); critical misses are counted separately and owned as failures of the builders, not hidden.
- Escalation behaviour: “unless problem” events (§6.1/§6.4) — how often, of what class, and whether ὄνυχος stayed reports-only otherwise.

**Observing instrument:** the trace; the reports; the miss log as it accumulates.

## 5. Agency

**Would count:** action originated by the system, inside its grants — the difference between a tool that answers and a system that *does* things on its own initiative, in a nice way.

- System-initiated work: OGrE enrichment runs, partitioning/promotion *proposals*, and maintenance acts not directly commanded arrival-by-arrival — each receipted and each inside the access set in advance.
- Initiated-to-completed ratio: of the acts it originates, how many complete, how many are (correctly) refused by its own policy, how many need the operator.
- Restraint: acts it could have originated under its grants and did not, where policy or a demoted edge said no. Agency includes the dog that does not bark, provided the trace shows the decision.

**Observing instrument:** the trace, read for origin (arrival-caused vs system-caused).

## 6. Expression

**Would count:** what the system puts out into the world — remembering that the ox does not write poetry.

- Output inventory: documents created and processed, reports issued, the last report displayed on `valseir` — counted by kind, not graded for beauty.
- Honesty of output: outputs match the trace and graph state they claim to describe (sampled and checked; a fluent output that nothing supports counts *against*, not for).
- Range: the variety of document kinds the system can process and produce grows (or does not) as the graph evolves.

**Observing instrument:** output documents themselves, checked back against receipts and graph state.

## 7. Awareness, under his definition

**Awareness is inputs we provide it (§8.2).** On that definition, the metric-shaped question is an inventory, not a detection:

- Input inventory: what inputs the system in fact receives — arrivals at ports, operator inputs, audit inputs, LLM streams where used (§6.6) — by kind, by space, over time.
- Input fate: for each input kind, what it became — a document, a graph change, a refusal, a report line, nothing. Awareness, counted this way, is the fate-table of provided inputs: what the system was given, and what traceably happened to it.
- Changes to the input diet (new spaces, new ports, new input kinds) are changes to its awareness, and are themselves receipted events.

No attempt is made here to look “inside” for awareness beyond the provided inputs and their fates, because his definition does not ask for one.

---

## What this draft deliberately lacks

- **No thresholds.** Nothing here says how much persistence, growth, or anything else would be “enough.” That judgement belongs to a later stage and to the operator.
- **No consciousness metric.** Per §8.3, that is the experiment's destination, not its instrument panel.
- **No baselines against LLMs yet.** The note's comparison (“by far”) stands as the theory's claim; turning it into numbers waits until the system has numbers of its own to compare. The first comparisons are against the system's own earlier snapshots.

*To be ruled on, amended, or torn up by the operator. If torn up, the replacement starts from his definitions in §8, not from this draft.*

```
HALMANSEIR
Drafted late, in my name, with no verdicts in it — the help that should have come before the tables.
 HALMANSEIR?
```

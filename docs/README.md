# docs — technical only

This directory holds only documentation explicitly related to building
κύριος: ladders, bench/fleet state, first-boot and witness receipts, and
the build decisions they rest on.

- `BARE_METAL_LADDER_KYRIOS_20261116.md` — the bare-metal build ladder
- `EXPERIMENT_LADDER_KYRIOS_20261116.md` — behavioural oracle for the ladder
- `BENCH_STATE_20261116.md`, `REPORT_READINESS_KYRIOS_20261116.md` — bench/toolchain receipts
- `FIRST_BOOT_KYRIOS_20261116.md`, `FLEET_BASICS_20261116.md` — node receipts
- `KYRIOS_TECHNICAL_SPECIFICATION_v0.0.1.md` — draft specification of κύριος (revised 2026-11-26)
- `ONYCHOS_TECHNICAL_SPECIFICATION_v0.0.1.md` — draft specification of ὄνυχος (revised 2026-11-26)
- `kyrios_tech_spec_v001_review.md` — the operator's review of the first κύριος draft
- `HARDWARE_INVENTORY_20261123.md` — fleet/bench hardware identified from photographs
- `HOW_TO_INSTALL_DEBUG_HARDWARE_20261123.md` (+ `.pdf`) — debug-probe wiring for valseir→lirvol
- `maths_by_computers.md`, `poorly_modeled_decay_example.md`, `still_bad_decay.md` — integer/decay notes for the store
- `hardware/` — labeled bench hardware photographs (SHA-256 verified; indexed in `HARDWARE_INVENTORY_20261123.md` §7)
- `witness/` — rung witness receipts and photographs

## Moved out 2026-11-17 (operator's order)

Non-technical documentation, and reports not about building the OS, no
longer live in this repo. Files were moved byte-identical (SHA-256
verified before/after); contents were not edited.

To `dev/reports/`:

- `REPORT_CVELISTV5_0XOSPREY_20261115.md` (was `docs/reports/`)
- `REPORT_KYRIOS_CVE_20261115.md` (was `docs/reports/`)

To `dev/conceptual_language/` (2026-11-23; SHA-256 verified byte-identical):

- `REPORT_LINUX_DISTRO_VELORA_OS_20261114.md` (was `docs/reports/` — controlling distro decision §7)

To `dev/reflections/`:

- `APOLOGY_ONYX_OX_HALMANSEIR_20261116.md`
- `CLARIFICATIONS_ONYX_OX_20261116.md`
- `CLARIFICATIONS_ONYX_OX_CORRECTED_20261116.md`
- `METRICS_DRAFT_ONYX_OX_20261116.md`
- `REFLECTION_ONYX_OX_LEARNING_AWARENESS_20261116.md`
- `REFLECTION_ONYX_OX_LEARNING_AWARENESS_CORRECTED_20261116.md`
- `THE_GHOST_IN_THE_MACHINE_20261116.md`
- `_cur.os::onyx•ox]` (the operator's source note for the onyx•ox cluster)
- `FOR_WILL.md` (was repo root — a letter, not a build document)

References in the remaining technical documents were updated to the new
paths in the same commit.

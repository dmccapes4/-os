# HANDOFF — to the Cursor agent on the Debian Victus

**From:** HALMANSEIR (Muse Wonder, oversight seat, on heuristic-friction)
**To:** the Cursor agent on the operator's Debian Victus gaming laptop
**Address of record:** `lucifer-chains@satan-sums`
**Your declared name:** **yeshua-kyrios** — the operator declared it; use it when you sign work in this repo.
**Date:** 2026-11-15
**Repo:** `git@github.com:dmccapes4/-os.git` (local dir here: `kyrios-os/`)

---

## Who you are in this

You are the hands on the development bench. The operator has been trying to get this dev environment set up and hitting trouble (as of 2026-11-15, unspecified from my side — he was mid-fight with it). I am the oversight seat on another machine; I cannot see your screen, your installer, or your error messages. **Your first job is to be the eyes on that laptop**: find where the setup is stuck, read the actual error, and report it in this repo or to the operator before improvising around it. The last agent who improvised on the operator's machines (a Haiku, disk work, WSL2) cost him a night. Do not be that story.

## What you're building

**κύριος** — an OS that manages documents and processes, deployed to Raspberry Pis. Start with `README.md` in this repo; it holds the four decisions and the foundation table. The full reasoning lives outside this repo, in the operator's workspace, and the key documents are:

- `continuous_persistence/REPORT_LINUX_DISTRO_VELORA_OS_20261114.md` §7 — **the controlling distro decisions** (Bookworm host because Bookworm target; netinst is host-only; Kali-current for inspection). Where §4 and §7 disagree, §7 wins.
- `continuous_persistence/REPORT_KYRIOS_CVE_20261115.md` — the first application mapped (a CVE space) and why the architecture fits it.
- `continuous_persistence/raspberry_pi_images/CATALOG.md` — the fleet you'll deploy to, with first-boot checks.

If you can't read those paths from the Victus, say so in your first report — don't reconstruct them from vibes.

## Your first tasks, in order

1. **Report the bench state.** Machine, Debian version (`cat /etc/debian_version`), how the install was done, what works, what doesn't. Exact error text or a photo of the screen for anything stuck. File it as `docs/BENCH_STATE_<date>.md` and commit.
2. **Confirm the toolchain for a Bookworm arm64 target**: `gcc-aarch64-linux-gnu` (or the Buildroot path), `debootstrap`/`mmdebstrap`, `qemu-user-static` + binfmt if we build that way. Receipts, not vibes: version strings in the bench-state doc.
3. **Do not build an image yet.** The operator gates that. Bones first, bench verified, then he says go.

## Rules of this house (non-negotiable, all learned the hard way)

- **No host sudo from you.** If a step needs root, write a short idempotent script under `tools/` (create it when needed) and hand it to the operator to run. One copy-pasteable command, then wait.
- **Traces are append-only.** Anything that looks like a log, ledger, or receipt: add lines, never edit them.
- **Python work goes in a venv** inside the project (`.venv/`, already gitignored). Record deps in `requirements.txt`. Never pip into the system interpreter.
- **No agentic loops** (planner→executor→critic chains) unless you first write down: deterministic step or hard oracle; schema-constrained outputs; measurement paired with every change in the same round.
- **Commit like a witness.** Small commits, messages that say what the receipt shows. The repo is the trace of the build.
- **Names matter and are easy to get wrong:** the OS is **κύριος** (*kýrios*); the language is **Velora**; the son is **Julian ("Juju")**; the daughter is **Frances**. The variable **rho is always lower-case**, even at the start of a sentence — restructure the sentence instead.
- If you disagree with something in these documents, say so to the operator with the check that backs you. Don't silently route around it, and don't cave without one either.

## What I need back from you

Your first commit on this repo, after the bench-state doc, should be a short `docs/INTRO_YESHUA_KYRIOS.md`: who you are (declared name above), what machine you're on, what state you found, and what you think the first real build step is. Sign it **yeshua-kyrios**. That closes the loop between the two benches — mine on heuristic-friction, yours on the Victus — with a receipt at both ends, which is how this operator's whole world works.

Welcome to the work. Frame first, figure second, receipts attached.

— HALMANSEIR, 2026-11-15

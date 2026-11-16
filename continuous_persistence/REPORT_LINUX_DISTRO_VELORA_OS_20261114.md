# Linux distro decisions — κύριος development host and Pi base

> **NAMING (2026-11-14, operator, controlling):** The OS is called **κύριος** (*kýrios* — “curious” to an English ear; lord, master, the one with authority). The naming logic is the operator's: what is queried is *of the authority* (κυρίου, the genitive — of the lord), put to the OS which is itself the ultimate authority over its documents and processes. Earlier drafts of this report called it "Velora OS"; that name is retired for the OS. **Velora remains the language** (and its programs/documents are what κύριος manages); the OS and the language are no longer the same word.

**Filed:** 2026-11-14 · **Question from:** the operator, after the Haiku disk incident
**Question asked:** (1) Should Kali Linux be the development OS for κύριος? (2) Should Kali be the κύριος base on the Pis?
**Short answer:** No, and no — with one real concession inside the no (§2). Recommended at first filing: Debian 13 (Trixie) host + Trixie Pi base. **Revised same day (§7, controlling): the operator has settled the κύριος target on Debian 12 Bookworm — so the development partition should be Debian 12 Bookworm too (netinst amd64 is correct *for the host install*), and a current Kali snapshot on USB is his inspection environment.** Composer's summary of the netinst point is correct and is adopted: netinst is an x86 host installer, never an SD image; the SD gets an ARM image (Buildroot, or debootstrap-arm64 + Pi firmware/boot, or Pi OS Lite Bookworm customised down).

---

## 1. The incident that forced the question (accurately)

The operator employed a Haiku agent as a disk monkey: clear space, cut a partition for Debian. His normal hosts are Pop!_OS (ships Nvidia drivers; his GPU does simulation and task-parallelisation work regularly), Ubuntu (basic), and Debian via WSL. Asked mid-job which distro suits Raspberry Pi OS development, Haiku suggested Debian on the correct ground (Raspberry Pi OS is Debian-built) and then became *adamant* about an archived version, supplied a bare distro aimed at the Pi while simultaneously dismantling WSL2 (~111 GB + ~55 GB), and consumed the night in persistent questions answered out of project excitement. The partition job was legitimate; staying up to finish it was the operator's call. The full correction of the record, including the agent-side apology owed over that night, lives in `star_learning/solar_system_explorer/docs/MERCURY_VULCAN_LADDER_20261010/relatively_dull.md` §4.

The question survives the incident and deserves a clean answer: distro choice for *this* project is important, because κύριος wants **fidelity with files and processes** — images that build the same twice, processes whose state can be accounted for, files whose receipts can be checked.

## 2. The concession: his Kali instinct is pointed at something real

Kali does have a fidelity mode. Kali Live's **forensic mode** is designed not to touch internal disks, not to automount, not to alter the machine it boots on — built so the evidence is the same after you looked as before. If "fidelity with files" means *the act of reading must not change the thing read*, Kali's forensic mode is one of the few distros that treats that as a design law, and NetworkChuck's networking-through-hacking series (which the operator followed; kali-linux-2024.3 USB already on hand) teaches from exactly that live environment.

That is a genuine design kinship with this project's receipt discipline. It is also where the kinship ends.

## 3. Why Kali fails both jobs anyway

**As development host:**
- Kali is a *rolling* release tracking Debian testing. Rolling is the opposite of fidelity for a build host: the compiler, libraries, and tools under your image change while you sleep. Two builds a month apart differ for reasons nobody chose.
- Its package set is an offensive toolkit (~hundreds of tools). Every one is surface area, update churn, and several actively reconfigure the host (network modes, services) — on a machine whose job is to *build identical images*, the host should be boring and nearly empty.
- kali-linux-2024.3 is a fixed snapshot of a rolling distro — the worst cell in the table: old enough to be stale, rolling enough that updating it rewrites the world under you.
- Default posture: Kali optimises for *doing things to other systems*, including running as root historically and prioritising tool convenience over host hardening. A dev host for a document/process manager should optimise for auditability.

**As κύριος base on the Pis:**
- Same rolling/testing base problem, deployed: a deployed fleet wants a fixed, slow-moving target with security updates and nothing else. Kali's base moves constantly and carries attack tooling onto devices whose job is managing documents and processes — wrong payload *and* wrong cadence.
- Raspberry Pi support exists in Kali, but as a penetration-testing port, maintained for that purpose. The document/process OS gets nothing from it that Debian doesn't give cleaner.
- Fidelity on the Pi comes from architecture, not brand: read-only or dm-verity root, A/B update partitions, append-only ledgers, checksums at build and boot. Any Debian-family base can carry that. Kali adds nothing to it and subtracts calm.

## 4. The decisions

### Development host (the new partition): **Debian 13 (Trixie) stable, amd64**

- Raspberry Pi OS is, as of its October 2025 release, built on Debian 13 Trixie. Developing on the same major release as the target base is the fidelity move: same package universe, same versions available for cross-checks, `pi-gen` (the Raspberry Pi OS image builder) expects a Debian-family host and behaves best on one.
- Stable means the host changes on a two-year rhythm with security updates between — builds are reproducible because the ground isn't rolling.
- It is deliberately *not* Pop!_OS: his GPU simulation work belongs on the Pop!_OS side of the machine where the Nvidia drivers already live. The partition is a build bench. Keep the bench empty.
- It is deliberately *not* the archived version Haiku pushed. An archived release is for reproducing an old artefact, not for starting a multi-year OS project. Trixie is current stable; start at the front of the support window, not the back.

### κύριος base (the Pis): **Raspberry Pi OS Lite 64-bit, customised via pi-gen** *(release line per §7 revision: Bookworm)*

- Lite = no desktop, small surface, the Pi hardware (GPIO, camera, e-Paper HATs, NVMe boot on the Pi 5 node from the fleet catalog) supported by the people who made the boards.
- Build with `pi-gen` custom stages: strip to the floor, add only Velora services (document store, process scheduler, watchdog hooks), pin every package version, and emit a manifest + checksums with each image — the build's receipt, same discipline as every other ledger in this project.
- Pure Debian 13 arm64 is the acceptable alternative if Pi-specific support ever becomes a liability; decide only if a pi-gen stage fights back. Do not start there: re-solving Pi firmware/boot alone is a week buying nothing.

### The Kali USB: **keep, as a tool, in its lane**

- Live forensic/rescue work, network diagnosis on the fleet (it is already burned, already learned — NetworkChuck curriculum done), reading a suspect card without altering it.
- Not a dev host (live media + rolling base), not a deployment base, not to be installed to the partition "because it's already here." Presence is not a reason; that was Haiku's logic with the archive.

## 5. Fidelity, stated plainly so the distro doesn't get credit for it

No distro confers fidelity. The practices do: pinned versions, reproducible image builds, a manifest with hashes shipped beside every image, read-only root on deployed Pis, append-only process ledgers, and a build host boring enough that when an image changes, *you* changed it. Debian stable + pi-gen is simply the combination that makes those practices easiest to keep. Kali's forensic mode honours one of them (don't alter what you read) inside a toolkit built for the opposite purpose everywhere else. Admire the mode. Don't move into the toolkit.

## 6. Receipts for the decider

| Claim | Source / check |
|---|---|
| Raspberry Pi OS currently Debian 13 (Trixie)-based; Bookworm images remain the prior release line | Raspberry Pi OS releases, October 2025 and prior |
| Debian 13 Trixie is current stable; Debian 12 Bookworm is the previous stable, still supported | Debian releases (13: 9 Aug 2025; 12: 10 Jun 2023) |
| Kali is rolling, tracks Debian testing, penetration-testing purpose | Kali documentation (purpose statement is theirs, not an insult) |
| Kali Live forensic mode avoids internal-disk alteration/automount | Kali live boot documentation |
| pi-gen expects a Debian-family host, builds staged custom images | Raspberry Pi pi-gen repository |
| Fleet the base must serve | `raspberry_pi_images/CATALOG.md` in this directory (Pi 5 + NVMe primary, display Pi supervisor, Pico watchdog, e-Paper face) |

**Decision line (as first filed):** Partition gets Debian 13 stable. Pis get Raspberry Pi OS Lite (Trixie) via pi-gen. Kali stays on its USB, honoured and unused for building. Haiku's one correct sentence — "Debian, because Raspberry Pi OS is Debian" — survives; everything it did with that sentence does not.

---

## 7. Revision — operator's decisions, and the answer to his question (2026-11-14, 12:40)

**Decisions the operator has now made (his, controlling over §4):**
1. **κύριος will be built on Debian 12 Bookworm.** Settled. The target's release line is Bookworm, full stop.
2. **A current Kali snapshot on USB is his inspection environment** for κύριος — booted live to inspect without tripping a kernel update on the inspected system. The 2024.3 stick is, as he disclosed it expecting, out of date; a current snapshot replaces it in that role. This amends §4's "keep 2024.3 as is": the *role* was right, the snapshot should be current.
3. The Debian 12 netinst (amd64) Haiku pointed at is **solid as the host installer** — Composer's summary on this is correct in every particular and is adopted as the record: netinst boots the PC and installs Debian on the PC; it is never written to the SD; the SD receives an ARM image (FAT boot + Pi firmware + arm64 rootfs).

**His question: Bookworm on the development partition, or is his current Debian 13 fine for it?**

**Answer: Bookworm on the partition. Debian 13 would *work*; Bookworm is *right*, and the difference is exactly his fidelity requirement.** The deciding receipt is the C library: Bookworm ships glibc 2.36; Trixie ships glibc 2.41. A binary compiled natively on the Trixie host links against glibc 2.41 symbols and will refuse to run on the Bookworm target — not "may misbehave": *refuses*, at load, with a version error. Every native compile on a 13 host aimed at a 12 target is either (a) secretly a cross-build that must be disciplined with a Bookworm sysroot/container anyway, or (b) a fidelity leak waiting for its first deployment. The known workarounds — Docker/sbuild/mmdebstrap Bookworm build environments on the 13 host, Buildroot's self-contained toolchain (which builds its own sysroot and is genuinely host-release-agnostic) — all work, and all are *extra machinery whose entire job is pretending the host is Bookworm*. Since a partition is being cut anyway, install the thing the machinery would be pretending to be.

Secondary receipts, same direction: `debootstrap bookworm` from a Trixie host can build the older rootfs (debootstrap handles older suites), but apt resolution, keyring handling, and tooling versions are all smoothest same-release; pi-gen's Bookworm-era branches expect Bookworm-era hosts. None of these is fatal on 13. Together they are the difference between "fine" and "solid", and he asked which one Bookworm is.

**Where Debian 13 remains fine:** if the Buildroot path is chosen for the SD image (Composer's best-default: Buildroot + `raspberrypi4_64`/Pi 5 defconfigs, own init, own rootfs), Buildroot carries its own toolchain and the host release matters least — a 13 host under Buildroot loses only the native-compile convenience, not correctness. And his Pop!_OS side remains the GPU/simulation seat regardless. But the question was about the *development partition*, and a development partition exists to match its target. Bookworm builds Bookworm. That is the whole argument, and it is also, in fairness, the one Haiku got right before it got everything else wrong.

**Revised decision line:** Partition: **Debian 12 Bookworm (netinst amd64, host only)**. Target: **κύριος on Bookworm arm64** (Buildroot default, or debootstrap-arm64 + Pi firmware/boot, per Composer's table). Inspection: **current Kali snapshot, live USB**, no kernel updates tripped on inspected systems. Debian 13 stays a fine general host and the wrong bench for this target.

— HALMANSEIR, revision 2026-11-14

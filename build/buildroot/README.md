# build/buildroot — the arm64 image track (ladder track P)

Pinned: **Buildroot 2026.08.1**, base defconfig `raspberrypi5_defconfig`,
kyrios additions in `kyrios_lirvol.fragment` (Python 3 stdlib for the
socket daemon, OpenSSH server, small receipt tools).

## Steps

1. `./fetch_buildroot.sh` — download, checksum, extract to `-os/_build/`
   (gitignored), configure. No sudo, nothing built.
2. **Gate G2/P3 (yours):** `cd -os/_build/buildroot-2026.08.1 && make -j$(nproc)`
   — the long build. Produces `output/images/sdcard.img`.
3. Gate G3/P4: flash a spare SD, boot `lirvol`, ladder acceptance on hardware.

The kyrios payload (runtime/, spaces/, processors/, ledger/, systemd or
init wiring, fleet key, hifiberry-digi equivalent for Buildroot's kernel)
is folded in at step 2's first real build — deliberately not before,
because Buildroot kernel/firmware config decides how several of those
pieces land, and guessing them now would be prose, not engineering.

Note on fidelity: Buildroot builds its own toolchain, so the README's
"Bookworm builds Bookworm" rule constrains bench binaries, not this rootfs.
The daemon's stdlib-only rule (ladder D4) is what keeps that honest.

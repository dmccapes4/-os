#!/bin/sh
# κύριος bench toolchain — Bookworm host, Bookworm arm64 target.
# Prepared by an agent; run by the operator (house rule: no host sudo from agents).
# Idempotent: safe to re-run. Prints version receipts at the end for the bench-state doc.
set -eu

sudo apt-get update
sudo apt-get install -y --no-install-recommends \
  build-essential \
  gcc-aarch64-linux-gnu \
  libc6-dev-arm64-cross \
  debootstrap \
  mmdebstrap \
  qemu-user-static \
  binfmt-support \
  file \
  git \
  python3 \
  python3-venv \
  ca-certificates

echo "--- receipts ---"
cat /etc/debian_version
aarch64-linux-gnu-gcc --version | head -1
/usr/sbin/debootstrap --version
mmdebstrap --version
qemu-aarch64-static --version | head -1
python3 --version
git --version
echo "Toolchain present. Next: file docs/BENCH_STATE_<date>.md from these receipts."

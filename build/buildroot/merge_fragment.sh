#!/bin/sh
# Merge a kyrios config fragment into the configured Buildroot tree and
# settle olddefconfig. Run from inside the extracted Buildroot tree.
set -eu
FRAGMENT="${1:?usage: merge_fragment.sh <fragment>}"
HERE="$(pwd)"
[ -f "$HERE/support/kconfig/merge_config.sh" ] || {
  echo "not a Buildroot tree: $HERE" >&2; exit 2; }
"$HERE/support/kconfig/merge_config.sh" -m "$HERE/.config" "$FRAGMENT"
make olddefconfig
echo "--- kyrios fragment now in .config: ---"
grep -E "^BR2_PACKAGE_(PYTHON3|OPENSSH)|^BR2_ROOTFS_OVERLAY|^BR2_PACKAGE_KYRIOS" "$HERE/.config" || true

#!/bin/sh
# Fetch and configure (never build) the pinned Buildroot for lirvol.
# Building the image is a separate, operator-gated step (ladder gate G2/P3):
#   cd -os/_build/buildroot-2026.08.1 && make -j$(nproc)
# No sudo anywhere: Buildroot builds as the invoking user.
set -eu

VERSION="2026.08.1"
TARBALL="buildroot-${VERSION}.tar.gz"
BASE="https://buildroot.org/downloads"
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
DEST="$ROOT/_build"

mkdir -p "$DEST"
cd "$DEST"

if [ ! -f "$TARBALL" ]; then
  echo "--- downloading $TARBALL ---"
  curl -fO "$BASE/$TARBALL"
  curl -fsO "$BASE/$TARBALL.sha256" 2>/dev/null || true
fi
if [ -s "$TARBALL.sha256" ] && grep -q "$TARBALL" "$TARBALL.sha256"; then
  sha256sum -c "$TARBALL.sha256"
else
  rm -f "$TARBALL.sha256"
  echo "no upstream per-file .sha256 (buildroot publishes a PGP .sign instead); local checksum:"
  sha256sum "$TARBALL"
fi

if [ ! -d "buildroot-$VERSION" ]; then
  echo "--- extracting ---"
  tar xf "$TARBALL"
fi

cd "buildroot-$VERSION"
echo "--- configuring: raspberrypi5_defconfig + kyrios fragment ---"
make raspberrypi5_defconfig
"$ROOT/build/buildroot/merge_fragment.sh" "$ROOT/build/buildroot/kyrios_lirvol.fragment"
echo "configured. Build (gated) with:  cd $DEST/buildroot-$VERSION && make -j$(nproc)"

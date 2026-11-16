#!/usr/bin/env python3
"""Extract Pi 4 firmware files from a Raspberry Pi OS .img.xz (FAT32 p1).

Minimal read-only FAT32 root-directory parser — enough for flat boot
partitions, no external tools (no mtools on the bench). Files land in
kernel/boot-sd/.
"""
import lzma
import struct
import sys
from pathlib import Path

WANTED = {
    "START4.ELF", "FIXUP4.DAT", "BCM2711-RPI-4-B.DTB",
    "BCM2711-RPI-400.DTB", "BCM2711-RPI-CM4.DTB", "BOOTCODE.BIN",
}


def main(img_xz: Path, out_dir: Path) -> int:
    data = lzma.decompress(img_xz.read_bytes())
    # MBR: partition 1 entry at 0x1BE; LBA start at +8.
    lba = struct.unpack_from("<I", data, 0x1BE + 8)[0]
    fat = data[lba * 512:]
    bytes_per_sec, sec_per_clus = struct.unpack_from("<HB", fat, 11)
    reserved, nfats = struct.unpack_from("<HB", fat, 14)
    fat_size = struct.unpack_from("<I", fat, 36)[0]
    root_clus = struct.unpack_from("<I", fat, 44)[0]
    first_data = (reserved + nfats * fat_size) * bytes_per_sec
    fat_off = reserved * bytes_per_sec

    def cluster_bytes(c: int) -> bytes:
        off = first_data + (c - 2) * sec_per_clus * bytes_per_sec
        return fat[off:off + sec_per_clus * bytes_per_sec]

    def next_cluster(c: int) -> int:
        return struct.unpack_from("<I", fat, fat_off + c * 4)[0] & 0x0FFFFFFF

    def read_chain(c: int, size: int) -> bytes:
        out = b""
        while 2 <= c < 0x0FFFFFF8:
            out += cluster_bytes(c)
            c = next_cluster(c)
        return out[:size]

    out_dir.mkdir(parents=True, exist_ok=True)
    found = set()
    c = root_clus
    while 2 <= c < 0x0FFFFFF8:
        entries = cluster_bytes(c)
        lfn = ""
        for i in range(0, len(entries), 32):
            e = entries[i:i + 32]
            if e[0] == 0x00:
                break
            if e[0] == 0xE5:
                lfn = ""
                continue
            if e[11] == 0x0F:  # LFN fragment (precedes its short entry)
                chars = e[1:11] + e[14:26] + e[28:32]
                text = chars.decode("utf-16-le", "replace")
                lfn = text.split("\x00")[0] + lfn
                continue
            name = e[0:8].decode("ascii", "replace").strip()
            ext = e[8:11].decode("ascii", "replace").strip()
            full = (lfn or f"{name}.{ext}" if ext else name).upper()
            lfn = ""
            if full in WANTED:
                start = (struct.unpack_from("<H", e, 20)[0] << 16) | \
                        struct.unpack_from("<H", e, 26)[0]
                size = struct.unpack_from("<I", e, 28)[0]
                (out_dir / full.lower()).write_bytes(read_chain(start, size))
                found.add(full)
                print(f"extracted {full.lower()} ({size} B)")
        c = next_cluster(c)

    missing = WANTED - found - {"BOOTCODE.BIN", "BCM2711-RPI-400.DTB",
                                "BCM2711-RPI-CM4.DTB"}
    if missing:
        print(f"MISSING: {sorted(missing)}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main(Path(sys.argv[1]), Path(sys.argv[2])))

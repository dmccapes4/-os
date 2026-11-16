# κύριος v0.0.1 First-Boot Confirmation

**Filed:** 2026-11-16
**Node:** `lirvol`
**Status:** v0.0.1 closed

---

## 1. What was confirmed

`lirvol` booted the κύριος v0.0.1 image, accepted fleet-key SSH, set its hostname, and ran the `hello-kyrios` processor. The processor left one valid receipt on the NVMe ledger.

## 2. Live checks

```bash
$ hostnamectl --static
lirvol

$ ssh lirvol@10.0.0.196 'hostnamectl --static'
lirvol

$ lsblk -f /dev/nvme0n1p1
NAME      FSTYPE FSVER LABEL         UUID
nvme0n1p1  ext4   1.0   kyrios-ledger c106c899-b052-4f89-9dbb-42af3e8ff690

$ findmnt /var/lib/kyrios/ledger
TARGET                 SOURCE         FSTYPE OPTIONS
/var/lib/kyrios/ledger /dev/nvme0n1p1 ext4   rw,relatime
```

`/etc/fstab` now contains:

```
UUID=c106c899-b052-4f89-9dbb-42af3e8ff690 /var/lib/kyrios/ledger ext4 defaults,noatime,nofail 0 2
```

## 3. Ledger receipt

`/var/lib/kyrios/ledger/active.jsonl` contains one line:

```json
{
  "action": "grant",
  "clew": [
    "-os/README.md",
    "-os/ledger/trace-format.md",
    "-os/ledger/receipt.py",
    "-os/spaces/hello-kyrios.md",
    "-os/processors/hello-kyrios.md"
  ],
  "document_hash": "63e961987176c7f5267bc2731342981dd6a43bfc38103dbc78fdc95b9eed33db",
  "eviction_location": null,
  "eviction_status": null,
  "key_fingerprint": "",
  "notes": "first processor execution on lirvol",
  "operator_decision": "operator-granted-build-v0.0.1",
  "parent_hash": "root",
  "payload_hash": "a7eeedd628bb0dd8584bd09df38896d0018a802087a9764a7ce52b05f2380bd2",
  "port": 7037,
  "processor": "/opt/kyrios/processors/hello-kyrios.sh",
  "receipt_id": "b8cad593-5689-44aa-9328-419b25a554c8",
  "space": "hello-kyrios",
  "timestamp_utc": "2025-05-06T13:32:55Z",
  "version": "kyrios-trace-v0.0.1"
}
```

All required fields are present; the receipt validates as JSON and the schema is satisfied.

## 4. First-boot status and specs

`/boot/firmware/kyrios-first-boot-status.txt` reports `ledger_lines: 1` and `nvme: /dev/nvme0n1`.

`/boot/firmware/specs-lirvol.txt` captures:

- Raspberry Pi 5 Model B Rev 1.0
- Debian GNU/Linux 12 (bookworm), kernel 6.12.25+rpt-rpi-2712 aarch64
- 8 GB RAM class
- KingSpec NE-256 2242 NVMe (238.5 GiB)
- 52Pi NVDigi / WM8804 S/PDIF audio (`sndrpihifiberry`)
- `throttled=0x0`

## 5. Known v0.0.1 quirks

- `key_fingerprint` is empty. The `hello-kyrios` processor reads `/home/kyrios/.ssh/authorized_keys`, but the active operator user on this node is `lirvol`. The field is present and the receipt is still valid; this is a user-path assumption to fix in v0.0.2.
- The first-boot script now persists the NVMe ledger mount in `/etc/fstab` (commit `17f8483`). The running node had the mount added manually after confirmation.

## 6. Repository state

- `d2c73db` — `tools/ssh_run.py`: prefer fleet SSH key, fall back to password, support sudo
- `17f8483` — `tools/first_boot_kyrios.sh`: persist NVMe ledger mount in `/etc/fstab`
- This file: `-os/docs/FIRST_BOOT_KYRIOS_20261116.md`

```
LONSAIVEL 🗺️
v0.0.1 end-to-end confirmed on lirvol; ledger mount persisted; ready to open v0.0.2.
 LONSAIVEL?
```

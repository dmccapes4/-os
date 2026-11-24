#!/usr/bin/env python3
"""
valseir_kyrios_serial.py — talk to kyrios on lirvol over valseir's serial
wire (game plan step O2). Run ON valseir:

    python3 valseir_kyrios_serial.py '7038 record'

Each argument is sent as one line. Everything that comes back is printed,
including kyrios's echo of what was sent. The port is opened at 115200
8N1, the rate both UARTs are set to. Needs the `kali` user (dialout
group) or root.
"""
import sys
import time

import serial

PORT = "/dev/ttyAMA0"


def main() -> int:
    if len(sys.argv) < 2:
        print("usage: valseir_kyrios_serial.py '<port> <request>' [...]", file=sys.stderr)
        return 2
    s = serial.Serial(PORT, 115200, timeout=0.2)
    s.reset_input_buffer()
    for line in sys.argv[1:]:
        s.write((line + "\n").encode())
    end = time.time() + 8
    data = b""
    while time.time() < end:
        chunk = s.read(4096)
        if chunk:
            data += chunk
            end = time.time() + 2.5
    sys.stdout.write(data.decode("utf-8", "replace"))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

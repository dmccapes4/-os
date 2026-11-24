#!/usr/bin/env python3
"""Send three broadcast frames out valseir's eth0.

Ethertype 0x88b5, payload "valseir to kyrios". For the lirvol link
check: kyrios prints any frame it receives. Run with privileges enough
to open an AF_PACKET socket.
"""
import socket
import time

frame = bytes.fromhex("ffffffffffffdca63295dfeb88b5") + b"valseir to kyrios"

s = socket.socket(socket.AF_PACKET, socket.SOCK_RAW)
s.bind(("eth0", 0))
for _ in range(3):
    s.send(frame)
    time.sleep(0.2)
print(f"sent 3 frames, {len(frame)} bytes each")

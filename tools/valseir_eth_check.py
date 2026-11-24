#!/usr/bin/env python3
"""Read valseir's eth0 link state over SSH, then listen briefly.

Usage: valseir_eth_check.py [listen_seconds]

Prints carrier / operstate / speed as found, then runs tcpdump on
eth0 for listen_seconds (default 20) and reports what arrived.
Read-only apart from the tcpdump capture itself.
"""
import sys
import paramiko

HOST = "10.0.0.120"
USER = "kali"
PASSWORD = "kali"


def run(client, cmd):
    _, stdout, stderr = client.exec_command(cmd, timeout=60)
    out = stdout.read().decode(errors="replace").strip()
    err = stderr.read().decode(errors="replace").strip()
    return out, err


def main():
    listen = int(sys.argv[1]) if len(sys.argv) > 1 else 20
    client = paramiko.SSHClient()
    client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    client.connect(HOST, username=USER, password=PASSWORD, timeout=10)

    out, _ = run(client, "cat /sys/class/net/eth0/carrier "
                         "/sys/class/net/eth0/operstate "
                         "/sys/class/net/eth0/speed 2>/dev/null")
    print("carrier/operstate/speed:")
    print(out)

    out, _ = run(client, "ip -s link show eth0")
    print(out)

    print(f"listening on eth0 for {listen}s ...")
    out, err = run(client, f"echo {PASSWORD} | sudo -S timeout {listen} "
                           f"tcpdump -i eth0 -e -n -c 10 2>/dev/null")
    print(out if out else "(no packets captured)")

    client.close()


if __name__ == "__main__":
    main()

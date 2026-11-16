#!/usr/bin/env python3
"""
ssh_run.py — run commands on a κύριος fleet node via SSH.

Tries the fleet private key first (~/.ssh/kyrios_fleet_node), falling back to
password auth. Sudo commands use LIRVOL_PASSWORD from the environment.

Usage:
    LIRVOL_PASSWORD=lirvol python3 tools/ssh_run.py lirvol@10.0.0.196 hostnamectl status
    LIRVOL_PASSWORD=lirvol python3 tools/ssh_run.py lirvol@10.0.0.196 --sudo hostnamectl set-hostname lirvol

Stdin is not used; commands are given as positional args.
"""
import os
import sys
from urllib.parse import urlparse

import paramiko


def run(host: str, user: str, password: str, command: str, use_sudo: bool = False, key_path: str = None) -> str:
    client = paramiko.SSHClient()
    client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    try:
        if key_path and os.path.isfile(key_path):
            client.connect(host, username=user, key_filename=key_path, look_for_keys=False, allow_agent=False, timeout=15)
        else:
            client.connect(host, username=user, password=password, look_for_keys=False, allow_agent=False, timeout=15)
        cmd = f"sudo -S {command}" if use_sudo else command
        stdin, stdout, stderr = client.exec_command(cmd, get_pty=True)
        if use_sudo:
            stdin.write(password + "\n")
            stdin.flush()
        stdin.channel.shutdown_write()
        out = stdout.read().decode("utf-8", errors="replace")
        err = stderr.read().decode("utf-8", errors="replace")
        return f"--- stdout ---\n{out}\n--- stderr ---\n{err}"
    finally:
        client.close()


def main() -> int:
    if len(sys.argv) < 3:
        print("usage: LIRVOL_PASSWORD=... python3 tools/ssh_run.py [user@]host [--sudo] command...", file=sys.stderr)
        return 1

    target = sys.argv[1]
    if "@" in target:
        user, host = target.split("@", 1)
    else:
        user = "lirvol"
        host = target

    password = os.environ.get("LIRVOL_PASSWORD")
    if not password:
        print("LIRVOL_PASSWORD not set", file=sys.stderr)
        return 1

    use_sudo = False
    args = sys.argv[2:]
    if args[0] == "--sudo":
        use_sudo = True
        args = args[1:]

    command = " ".join(args)
    key_path = os.environ.get("KYRIOS_KEY", os.path.expanduser("~/.ssh/kyrios_fleet_node"))
    print(f"connecting {user}@{host}: {command}")
    result = run(host, user, password, command, use_sudo=use_sudo, key_path=key_path)
    print(result)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

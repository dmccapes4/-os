#!/usr/bin/env python3
"""check_memory.py: read kyrios memory by address, the way a reader with
nothing but the memory would, and check what is found.

Usage: check_memory.py <directory holding memory.bin and memory.json>

It starts at the first address (the address table of the space kyrios)
and uses nothing else from kyrios. Standard library only.

Checked:
  1. Every table and every line of every JSONL document is JSON, and
     every value in it is a string, a list or an object. No JSON numbers.
  2. Every number is a hexadecimal string of 1 or more. No zero, no sign,
     no fraction.
  3. Every document's bytes are inside its space's allotment and hash to
     the table's `hash`.
  4. Every doc id is the first 16 bytes of
     SHA-256("kyrios-doc-v0.0.1" 0 space 0 kind 0 name).
  5. The record: positions run from 1 with none missing; each trace's
     `prev` is the SHA-256 of the line before it; each interaction is
     followed by its action; each refusal has a reason.
  6. `made` and `last` in the tables are the first and latest traces that
     name the document.
  7. The weights in the tables, and the `flagged` list of every rho
     trace, are recomputed from the record and the policy alone
     (specification section 7.3) and agree.
"""
import hashlib
import json
import re
import sys

HEX = re.compile(r"^[1-9a-f][0-9a-f]*$")
LABEL = re.compile(r"^[0-9a-f]{4}$")
NUMBER_KEYS = {"at", "operation", "port", "address", "length", "step", "rho", "ticks",
               "weight", "made", "last", "since", "start", "most", "for", "take",
               "read", "revise", "every", "table", "document", "documents"}

problems = []
counted = {"checks": 0}


def check(ok, message):
    counted["checks"] += 1
    if not ok:
        problems.append(message)


def no_numbers(value, where):
    """Every value is a string, a list or an object; numbers are hex strings."""
    if isinstance(value, dict):
        for k, v in value.items():
            if k in NUMBER_KEYS and isinstance(v, str) and where != "metadata":
                check(bool(HEX.match(v)), f"{where}: {k} = {v!r} is not a hexadecimal count")
            no_numbers(v, where)
    elif isinstance(value, list):
        for v in value:
            no_numbers(v, where)
    else:
        check(isinstance(value, str), f"{where}: {value!r} is not a string")


def doc_id(space, kind, name):
    h = hashlib.sha256(b"kyrios-doc-v0.0.1\0" + space.encode() + b"\0" + kind.encode() + b"\0" + name.encode())
    return h.digest()[:16].hex()


def main():
    d = sys.argv[1]
    info = json.load(open(f"{d}/memory.json"))
    base, length = int(info["base"], 16), int(info["length"], 16)
    mem = open(f"{d}/memory.bin", "rb").read()
    check(len(mem) == length, "memory.bin has the wrong length")

    def at(address, n):
        check(base <= address and address + n <= base + length, f"{address:x}+{n:x} is outside memory")
        return mem[address - base: address - base + n]

    def table_at(address):
        raw = mem[address - base:]
        raw = raw[: raw.index(b"\0")]
        t = json.loads(raw)
        no_numbers(t, f"table at {address:x}")
        check(t.pop("schema") == "kyrios-addresses-v0.0.1", "table schema")
        return t

    # The one place to start.
    root = table_at(base)
    by_kind = {}
    for label, e in root.items():
        check(bool(LABEL.match(label)), f"label {label!r}")
        by_kind.setdefault(e["kind"], []).append(e)
    check(sorted(by_kind) == ["policy", "port", "record", "space"], f"kinds in the first table: {sorted(by_kind)}")

    spaces = {}
    tables = {"kyrios": root}
    for e in by_kind["space"]:
        body = json.loads(at(int(e["address"], 16), int(e["length"], 16)))
        no_numbers(body, f"space document {e['name']}")
        spaces[e["name"]] = body
        t = table_at(int(body["table"], 16))
        if e["name"] == "kyrios":
            check(t == root, "the space kyrios does not point back at the first table")
        tables[e["name"]] = t
    check(sorted(spaces) == ["hello-kyrios", "kyrios", "onychos"], f"spaces: {sorted(spaces)}")

    # Allotments do not overlap.
    spans = sorted((int(s["allotment"]["address"], 16), int(s["allotment"]["length"], 16)) for s in spaces.values())
    for (a, n), (b, _) in zip(spans, spans[1:]):
        check(a + n <= b, "two allotments overlap")

    # Documents: inside the allotment, right hash, right id.
    documents = 0
    for space, t in tables.items():
        lo = int(spaces[space]["allotment"]["address"], 16)
        hi = lo + int(spaces[space]["allotment"]["length"], 16)
        labels = sorted(int(x, 16) for x in t)
        check(labels == list(range(1, len(labels) + 1)), f"{space}: labels do not run from 1")
        for label, e in t.items():
            where = f"{space} {label}"
            a, n = int(e["address"], 16), int(e["length"], 16)
            check(lo <= a and a + n <= hi, f"{where}: outside the allotment")
            check(e["doc"] == doc_id(space, e["kind"], e["name"]), f"{where}: doc id")
            if e["kind"] == "record":
                check("hash" not in e, f"{where}: the record carries a hash")
            else:
                check(hashlib.sha256(at(a, n)).hexdigest() == e["hash"], f"{where}: hash")
            if "schedule" not in e:
                check("weight" not in e and "rho" not in e, f"{where}: weight without a schedule")
            documents += 1

    # Ports ledger.
    pe = by_kind["port"][0]
    ports = json.loads(at(int(pe["address"], 16), int(pe["length"], 16)))
    no_numbers(ports, "ports.json")
    check(ports.pop("schema") == "kyrios-ports-v0.0.1", "ports schema")
    for port, p in ports.items():
        check(spaces[p["space"]].get("port") == port, f"port {port}: the space document disagrees")
    check(len(ports) == sum(1 for s in spaces.values() if "port" in s), "ports and spaces disagree")

    # Policy.
    pe = by_kind["policy"][0]
    lines = at(int(pe["address"], 16), int(pe["length"], 16)).decode().splitlines()
    meta = json.loads(lines[0])
    check(meta["schema"] == "kyrios-policy-v0.0.1" and "update" in meta, "policy metadata")
    schedules = {}
    for line in lines[1:]:
        p = json.loads(line)
        no_numbers(p, "policy.jsonl")
        check(set(p) <= set(meta["fields"]) | {"method"}, f"policy field not in the metadata: {set(p) - set(meta['fields'])}")
        if p["rule"] == "schedule":
            rows = []
            for seg in p["takes"]:
                take = int(seg["take"], 16) if "take" in seg else None
                rows += [take] * (int(seg["for"], 16) if "for" in seg else 1)
            p["rows"] = rows
            schedules[p["name"]] = p
    check("documents" in schedules, "no schedule named documents")

    # The record.
    re_ = by_kind["record"][0]
    raw = at(int(re_["address"], 16), int(re_["length"], 16))
    check(raw.endswith(b"\n"), "the record does not end with a newline")
    lines = raw[:-1].split(b"\n")
    meta = json.loads(lines[0])
    check(meta["schema"] == "kyrios-record-v0.0.1" and "update" in meta, "record metadata")
    fields = set(meta["fields"])
    traces = [json.loads(x) for x in lines[1:]]

    names = {}          # (space, doc) -> positions of the traces that name it
    state = {}          # (space, doc) -> decay state, recomputed
    open_interaction = None
    for i, (t, line) in enumerate(zip(traces, lines[1:]), start=1):
        where = f"trace {i:x}"
        no_numbers(t, where)
        check(set(t) <= fields, f"{where}: field not in the metadata: {set(t) - fields}")
        check(list(t) == [k for k in meta["fields"] if k in t], f"{where}: field order")
        check(int(t["at"], 16) == i, f"{where}: at is {t['at']}")
        if i == 1:
            check("prev" not in t, f"{where}: the first trace has a prev")
        else:
            check(t.get("prev") == hashlib.sha256(lines[i - 1]).hexdigest(), f"{where}: prev")
        check(t["kind"] in ("interaction", "action"), f"{where}: kind")

        if t["kind"] == "interaction":
            check(open_interaction is None, f"{where}: an interaction before the last one's action")
            check(t["operation"] == t["at"], f"{where}: operation")
            check("result" not in t, f"{where}: result on an interaction")
            open_interaction = t
        else:
            check(t.get("result") in ("done", "refused"), f"{where}: result")
            check(("reason" in t) == (t["result"] == "refused"), f"{where}: reason")
            if "operation" in t:
                check(open_interaction is not None and open_interaction["at"] == t["operation"],
                      f"{where}: action without its interaction")
                if open_interaction is not None:
                    check(open_interaction.get("what") == t.get("what"), f"{where}: what differs from the interaction")
                open_interaction = None
            else:
                check(open_interaction is None and t["who"] == "system", f"{where}: unattached action")

        if "doc" in t:
            space = "kyrios" if t["what"] == "assign" else t["space"]
            names.setdefault((space, t["doc"]), []).append(i)

        # Decay, from the record and the policy only.
        done = t["kind"] == "action" and t.get("result") == "done"
        if done and t["what"] == "create":
            for s in schedules.values():
                if set(t.get("tags", [])) & set(s["kinds"]):
                    state[(t["space"], t["doc"])] = {"s": s, "weight": int(s["start"], 16), "row": None, "uses": {}}
        elif done and t["what"] in ("read", "revise"):
            st = state.get((t["space"], t["doc"]))
            if st:
                st["uses"][t["what"]] = st["uses"].get(t["what"], 0) + 1
        elif done and t["what"] == "rho":
            s = schedules[t["schedule"]]
            flagged = []
            for (space, doc), st in state.items():
                if st["s"] is not s:
                    continue
                add = sum(int(s["add"][w], 16) * n for w, n in st["uses"].items())
                st["uses"] = {}
                if add:
                    st["weight"] = min((st["weight"] or 0) + add, int(s["most"], 16))
                    st["row"] = None
                if st["weight"]:
                    st["row"] = 1 if st["row"] is None else min(st["row"] + 1, len(s["rows"]))
                    take = s["rows"][st["row"] - 1]
                    if take:
                        st["weight"] = st["weight"] - take if st["weight"] > take else None
                st["rho"] = int(t["step"], 16)
                if not st["weight"]:
                    flagged.append(doc)
            check(sorted(t.get("flagged", [])) == sorted(flagged), f"{where}: flagged list differs from the recomputed one")
    check(open_interaction is None, "the record ends with an interaction and no action")

    steps = [int(t["step"], 16) for t in traces if t.get("what") == "rho"]
    check(steps == list(range(1, len(steps) + 1)), "rho steps do not run from 1")

    # Tables against the record.
    scheduled = 0
    for space, t in tables.items():
        for label, e in t.items():
            where = f"{space} {label}"
            seen = names.get((space, e["doc"]), [])
            check(("made" in e) == bool(seen), f"{where}: made")
            if seen:
                check(int(e["made"], 16) == seen[0] and int(e["last"], 16) == seen[-1], f"{where}: made and last")
            st = state.get((space, e["doc"]))
            check((st is not None) == ("schedule" in e), f"{where}: schedule")
            if st:
                scheduled += 1
                have = int(e["weight"], 16) if "weight" in e else None
                check(have == st["weight"], f"{where}: weight {have} in the table, {st['weight']} recomputed")
                check(e.get("rho") == (format(st["rho"], "x") if "rho" in st else None), f"{where}: rho")

    for p in problems:
        print("FAILED", p)
    flagged_now = sum(1 for st in state.values() if not st["weight"])
    print(f"{counted['checks']} checks, {len(problems)} failed: {len(tables)} tables, {documents} documents, "
          f"{len(traces)} traces, {len(steps)} rho steps, {scheduled} weights recomputed, {flagged_now} flagged")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())

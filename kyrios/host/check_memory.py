#!/usr/bin/env python3
"""check_memory.py: read kyrios memory by address, the way a reader with
nothing but the memory would, and check what is found.

Usage: check_memory.py <directory holding memory.bin and memory.json>

It starts at the first address (the address table of the space kyrios)
and uses nothing else from kyrios. Standard library only.

Checked:
  1. Every table and every line of every JSONL document is valid JSON.
     Addressing, byte-span, and digest fields (`address`, `length`, `table`,
     `document`, `documents`, `doc`, `hash`, `was`, `prev`) are hexadecimal
     strings. General numerical, weight, port, and time fields have no
     hexadecimal-only prohibition: plain JSON numbers, timestamps, or v0.0.1
     strings are accepted.
  2. Every document's bytes are inside its space's allotment and hash to
     the table's `hash`.
  3. Every doc id is the first 16 bytes of
     SHA-256("kyrios-doc-v0.0.1" 0 space 0 kind 0 name).
  4. The record: positions run from 1 with none missing; each trace's
     `prev` is the SHA-256 of the line before it; each interaction is
     followed by its action; each refusal has a reason.
  5. `made` and `last` in the tables are the first and latest traces that
     name the document.
  6. The weights in the tables, and the `flagged` list of every rho
     trace, are recomputed from the record and the policy alone
     (specification section 7.3) and agree.
"""
import hashlib
import json
import re
import sys

HEX_ADDR = re.compile(r"^[0-9a-f]+$")
HEX_DIGEST = re.compile(r"^[0-9a-f]{64}$")
HEX_DOC = re.compile(r"^[0-9a-f]{32}$")
LABEL = re.compile(r"^[0-9a-f]{4}$")
ADDRESS_KEYS = {"address", "length", "table", "document", "documents"}
DIGEST_KEYS = {"hash", "was", "prev"}

problems = []
counted = {"checks": 0}


def check(ok, message):
    counted["checks"] += 1
    if not ok:
        problems.append(message)


def parse_int(v):
    """Accept plain JSON ints (decimal) or v0.0.1 hexadecimal count strings."""
    if isinstance(v, int) and not isinstance(v, bool):
        return v
    if isinstance(v, str):
        return int(v, 16)
    raise ValueError(f"unsupported integer representation: {v!r}")


def check_addressing_fields(value, where):
    """Verify hexadecimal formatting strictly where kyrios addresses or hashes bytes."""
    if isinstance(value, dict):
        for k, v in value.items():
            if where != "metadata":
                if k in ADDRESS_KEYS and isinstance(v, str):
                    check(bool(HEX_ADDR.match(v)), f"{where}: {k} = {v!r} is not a hexadecimal address/span")
                elif k in DIGEST_KEYS and isinstance(v, str):
                    check(bool(HEX_DIGEST.match(v)), f"{where}: {k} = {v!r} is not a 64-hex SHA-256 digest")
                elif k == "doc" and isinstance(v, str):
                    check(bool(HEX_DOC.match(v)), f"{where}: doc = {v!r} is not a 32-hex doc_id")
            check_addressing_fields(v, where)
    elif isinstance(value, list):
        for v in value:
            check_addressing_fields(v, where)


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
        check_addressing_fields(t, f"table at {address:x}")
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
        check_addressing_fields(body, f"space document {e['name']}")
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
    check_addressing_fields(ports, "ports.json")
    check(ports.pop("schema") == "kyrios-ports-v0.0.1", "ports schema")
    for port, p in ports.items():
        check(str(spaces[p["space"]].get("port")) == str(port), f"port {port}: the space document disagrees")
    check(len(ports) == sum(1 for s in spaces.values() if "port" in s), "ports and spaces disagree")

    # Policy.
    pe = by_kind["policy"][0]
    lines = at(int(pe["address"], 16), int(pe["length"], 16)).decode().splitlines()
    meta = json.loads(lines[0])
    check(meta["schema"] == "kyrios-policy-v0.0.1" and "update" in meta, "policy metadata")
    schedules = {}
    for line in lines[1:]:
        p = json.loads(line)
        check_addressing_fields(p, "policy.jsonl")
        check(set(p) <= set(meta["fields"]) | {"method"}, f"policy field not in the metadata: {set(p) - set(meta['fields'])}")
        if p["rule"] == "schedule":
            rows = []
            for seg in p["takes"]:
                take = parse_int(seg["take"]) if "take" in seg else None
                rows += [take] * (parse_int(seg["for"]) if "for" in seg else 1)
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
        check_addressing_fields(t, where)
        check(set(t) <= fields, f"{where}: field not in the metadata: {set(t) - fields}")
        check(list(t) == [k for k in meta["fields"] if k in t], f"{where}: field order")
        check(parse_int(t["at"]) == i, f"{where}: at is {t['at']}")
        if i == 1:
            check("prev" not in t, f"{where}: the first trace has a prev")
        else:
            check(t.get("prev") == hashlib.sha256(lines[i - 1]).hexdigest(), f"{where}: prev")
        check(t["kind"] in ("interaction", "action"), f"{where}: kind")

        if t["kind"] == "interaction":
            check(open_interaction is None, f"{where}: an interaction before the last one's action")
            check(parse_int(t["operation"]) == parse_int(t["at"]), f"{where}: operation")
            check("result" not in t, f"{where}: result on an interaction")
            open_interaction = t
        else:
            check(t.get("result") in ("done", "refused"), f"{where}: result")
            check(("reason" in t) == (t["result"] == "refused"), f"{where}: reason")
            if "operation" in t:
                check(open_interaction is not None and parse_int(open_interaction["at"]) == parse_int(t["operation"]),
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
                    state[(t["space"], t["doc"])] = {"s": s, "weight": parse_int(s["start"]), "row": None, "uses": {}}
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
                add = sum(parse_int(s["add"][w]) * n for w, n in st["uses"].items())
                st["uses"] = {}
                if add:
                    st["weight"] = min((st["weight"] or 0) + add, parse_int(s["most"]))
                    st["row"] = None
                if st["weight"]:
                    st["row"] = 1 if st["row"] is None else min(st["row"] + 1, len(s["rows"]))
                    take = s["rows"][st["row"] - 1]
                    if take:
                        st["weight"] = st["weight"] - take if st["weight"] > take else None
                st["rho"] = parse_int(t["step"])
                if not st["weight"]:
                    flagged.append(doc)
            check(sorted(t.get("flagged", [])) == sorted(flagged), f"{where}: flagged list differs from the recomputed one")
    check(open_interaction is None, "the record ends with an interaction and no action")

    steps = [parse_int(t["step"]) for t in traces if t.get("what") == "rho"]
    check(steps == list(range(1, len(steps) + 1)), "rho steps do not run from 1")

    # Tables against the record.
    scheduled = 0
    for space, t in tables.items():
        for label, e in t.items():
            where = f"{space} {label}"
            seen = names.get((space, e["doc"]), [])
            check(("made" in e) == bool(seen), f"{where}: made")
            if seen:
                check(parse_int(e["made"]) == seen[0] and parse_int(e["last"]) == seen[-1], f"{where}: made and last")
            st = state.get((space, e["doc"]))
            check((st is not None) == ("schedule" in e), f"{where}: schedule")
            if st:
                scheduled += 1
                have = parse_int(e["weight"]) if "weight" in e else None
                check(have == st["weight"], f"{where}: weight {have} in the table, {st['weight']} recomputed")
                have_rho = parse_int(e["rho"]) if "rho" in e else None
                check(have_rho == st.get("rho"), f"{where}: rho")

    for p in problems:
        print("FAILED", p)
    flagged_now = sum(1 for st in state.values() if not st["weight"])
    print(f"{counted['checks']} checks, {len(problems)} failed: {len(tables)} tables, {documents} documents, "
          f"{len(traces)} traces, {len(steps)} rho steps, {scheduled} weights recomputed, {flagged_now} flagged")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())

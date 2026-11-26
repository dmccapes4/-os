#!/usr/bin/env python3
"""check_epistemic_frames.py: bench verifier for KYRIOS_TECHNICAL_SPECIFICATION §6.9
and ONYCHOS_TECHNICAL_SPECIFICATION §5.3 / §6.1.

Verifies:
  1. Bounded frame validity (`frame_valid` vs `unframed`):
     - finite `n >= 1`,
     - homogeneous `unit` (guards against cross-category `satan-sums`),
     - non-empty resolvable `population` / `window`,
     - declared `method` (with optional external `hardware` offload provenance).
  2. Non-collapsing open `asterisks` across `graph retrieval` citation chains:
     - upstream open asterisks propagate into downstream `inherited_asterisks`,
     - repeating statistics under the SAME `method` CANNOT discharge an asterisk,
     - independent/orthogonal `method` (`V.frame.method != A.frame.method`) with
       `frame_valid == True` (or explicit operator `decide`) CAN discharge an asterisk.
  3. Measurable `collapse_drift` and `chain_depth`:
     - exact bounded count `(collapse_drift, n_open)` against finite upstream `n_open`,
     - surplus triplet representation against `thom = n_open`,
     - `chain_depth` along unbroken assumption chains (`lucifer-chains`),
     - natural coexistence of hexadecimal addresses/hashes with standard decimal counts,
       floats, ISO-8601 timestamps, and offloaded hardware identifiers.
"""
import sys


def surplus_triplet(with_count, thom):
    """Compute surplus triplet (§2.2) for integer counts 0 <= with_count <= thom, thom >= 1."""
    if not isinstance(thom, int) or thom < 1:
        raise ValueError("frame (thom) must be a finite integer >= 1")
    if not isinstance(with_count, int) or not (0 <= with_count <= thom):
        raise ValueError("with_count must lie within [0..thom]")
    without = thom - with_count
    thal = min(with_count, without)
    surplus = abs(with_count - without)
    out = {"thal": thal, "thom": thom}
    if surplus > 0:
        out["surplus"] = surplus
        out["direction"] = "ohn" if with_count > without else "bahn"
    return out


def validate_frame(claim):
    """Return (is_valid, reasons) for a quantitative claim's bounded frame (§6.9.1)."""
    reasons = []
    frame = claim.get("frame")
    if not isinstance(frame, dict):
        return False, ["missing_frame"]

    n = frame.get("n")
    if not (isinstance(n, int) and not isinstance(n, bool) and n >= 1):
        reasons.append("invalid_or_unbounded_n")

    unit = frame.get("unit")
    if not isinstance(unit, str) or not unit.strip():
        reasons.append("missing_unit")
    elif "+" in unit or "mixed" in unit.lower():
        reasons.append("heterogeneous_unit_sum")

    sub_units = frame.get("component_units")
    if isinstance(sub_units, list) and len(set(sub_units)) > 1:
        reasons.append("heterogeneous_unit_sum")

    pop = frame.get("population")
    win = frame.get("window")
    if not (isinstance(pop, str) and pop.strip()) or not isinstance(win, dict) or not win:
        reasons.append("unbounded_population_or_window")

    method = frame.get("method")
    if not isinstance(method, str) or not method.strip():
        reasons.append("missing_method")

    return len(reasons) == 0, reasons


def audit_document_graph(docs, edges, operator_decisions=None):
    """Audit frame validity, open-asterisk inheritance, orthogonal discharge,
    collapse_drift, and chain_depth over a directed citation/agreement graph.

    docs: dict[doc_id -> dict]
    edges: list[dict] with `from` (downstream) -> `to` (upstream cited source)
    """
    operator_decisions = operator_decisions or set()
    upstream_of = {d: [] for d in docs}
    for e in edges:
        if e["from"] in upstream_of and e["to"] in docs:
            upstream_of[e["from"]].append(e["to"])

    # Index verification documents capable of orthogonal discharge:
    # must have a valid bounded frame and explicitly target `(asterisk_id, method)`.
    verifiers_by_ast = {}
    for d_id, doc in docs.items():
        valid, _ = validate_frame(doc) if "measure" in doc or "frame" in doc else (True, [])
        if valid and "discharges" in doc:
            for target_ast in doc["discharges"]:
                verifiers_by_ast.setdefault(target_ast, []).append(doc)

    memo = {}

    def resolve_open_asterisks(d_id, visiting=None):
        """Return dict[ast_id -> {"origin_method": str, "hops": int}] of open asterisks at or above d_id."""
        if d_id in memo:
            return memo[d_id]
        visiting = visiting or set()
        if d_id in visiting:
            return {}
        visiting = visiting | {d_id}

        doc = docs[d_id]
        inherited = {}
        for up_id in upstream_of[d_id]:
            up_open = resolve_open_asterisks(up_id, visiting)
            for ast_id, info in up_open.items():
                hops = info["hops"] + 1
                if ast_id not in inherited or hops > inherited[ast_id]["hops"]:
                    inherited[ast_id] = {"origin_method": info["origin_method"], "hops": hops}

        # Check whether any inherited or local asterisk is legitimately discharged.
        own_open = dict(inherited)
        for ast in doc.get("asterisks", []):
            if ast.get("status") == "open":
                ast_id = ast["id"]
                if ast_id not in own_open:
                    own_open[ast_id] = {"origin_method": ast["method"], "hops": 0}

        # Apply orthogonal-discharge rule: same-method repetition NEVER discharges.
        still_open = {}
        for ast_id, info in own_open.items():
            if ast_id in operator_decisions:
                continue
            discharged_orthogonally = False
            for v_doc in verifiers_by_ast.get(ast_id, []):
                v_method = v_doc.get("frame", {}).get("method")
                if v_method and v_method != info["origin_method"]:
                    discharged_orthogonally = True
                    break
            if not discharged_orthogonally:
                still_open[ast_id] = info

        memo[d_id] = still_open
        return still_open

    results = {}
    for d_id, doc in docs.items():
        has_measure = "measure" in doc or "frame" in doc
        frame_ok, frame_reasons = validate_frame(doc) if has_measure else (True, [])

        # Upstream inherited open asterisks (strictly from upstream dependencies, hops >= 1)
        upstream_open = {}
        for up_id in upstream_of[d_id]:
            for ast_id, info in resolve_open_asterisks(up_id).items():
                hops = info["hops"] + 1
                if ast_id not in upstream_open or hops > upstream_open[ast_id]["hops"]:
                    upstream_open[ast_id] = {"origin_method": info["origin_method"], "hops": hops}

        # Filter out any upstream asterisks orthogonally discharged ON d_id itself
        effective_upstream = {}
        for ast_id, info in upstream_open.items():
            if ast_id in operator_decisions:
                continue
            v_method = doc.get("frame", {}).get("method")
            if frame_ok and ast_id in doc.get("discharges", []) and v_method and v_method != info["origin_method"]:
                continue
            effective_upstream[ast_id] = info

        declared_open_ids = {a["id"] for a in doc.get("asterisks", []) if a.get("status") == "open"}
        n_open = len(effective_upstream)
        if n_open == 0:
            results[d_id] = {
                "frame_state": "valid" if frame_ok else "unframed",
                "frame_reasons": frame_reasons,
                "n_open": 0,
                "kept_asterisks": None,
                "collapse_drift": None,
                "chain_depth": None,
            }
        else:
            kept = sum(1 for ast_id in effective_upstream if ast_id in declared_open_ids)
            dropped = n_open - kept
            depth = max(info["hops"] for info in effective_upstream.values())
            results[d_id] = {
                "frame_state": "valid" if frame_ok else "unframed",
                "frame_reasons": frame_reasons,
                "n_open": n_open,
                "kept_asterisks": kept,
                "collapse_drift": dropped,
                "collapse_triplet": surplus_triplet(dropped, n_open),
                "chain_depth": depth,
            }
    return results


def run_self_tests():
    checks = 0
    failures = []

    def expect(cond, msg):
        nonlocal checks
        checks += 1
        if not cond:
            failures.append(msg)

    # Planted graph mixing hex addresses/hashes with normal timestamps, floats, ints, and hardware offload:
    docs = {
        # Hop 0: Offloaded statistical study declaring 3 explicit methodological asterisks.
        "0001": {
            "address": "401000",
            "length": "3a0",
            "created_at": "2026-11-26T10:15:00Z",
            "frame": {
                "population": "space:exp/cohort_alpha[1..120]",
                "n": 120,
                "unit": "circuit_run",
                "window": {"from_run": 1, "to_run": 120},
                "method": "method:zne_poly_extrapolation_v1",
                "hardware": "offload:gpu-cluster-01",
            },
            "measure": {"mean_fidelity_est": 0.984, "std_err": 0.019},
            "asterisks": [
                {"id": "ast-zne-poly", "kind": "statistical_model", "method": "method:zne_poly_extrapolation_v1", "status": "open"},
                {"id": "ast-postselect", "kind": "selection_bias", "method": "method:zne_poly_extrapolation_v1", "status": "open"},
                {"id": "ast-thermal", "kind": "hardware_env", "method": "method:zne_poly_extrapolation_v1", "status": "open"},
            ],
        },
        # Hop 1: Intermediate summary keeps only 1 of 3 asterisks and tries (invalidly!) to discharge
        # `ast-zne-poly` by re-running statistics with the SAME method.
        "0002": {
            "address": "401400",
            "length": "210",
            "created_at": "2026-11-26T10:30:00Z",
            "frame": {
                "population": "space:exp/cohort_alpha[1..120]",
                "n": 120,
                "unit": "circuit_run",
                "window": {"from_run": 1, "to_run": 120},
                "method": "method:zne_poly_extrapolation_v1",
            },
            "measure": {"p_value": 0.003},
            "discharges": ["ast-zne-poly"],  # Same method -> MUST NOT discharge!
            "asterisks": [
                {"id": "ast-thermal", "kind": "hardware_env", "method": "method:zne_poly_extrapolation_v1", "status": "open"},
            ],
        },
        # Hop 2: Downstream synthesis drops ALL asterisks AND uses an invalid heterogeneous frame (`satan-sums`).
        "0003": {
            "address": "401700",
            "length": "180",
            "created_at": "2026-11-26T10:45:00Z",
            "frame": {
                "population": "space:exp/combined",
                "n": 170,
                "unit": "circuit_run+simulated_prior",
                "component_units": ["circuit_run", "simulated_prior"],
                "window": {"from_run": 1, "to_run": 170},
                "method": "method:meta_average_v1",
            },
            "measure": {"headline_claim": "unconditional_advantage"},
            "asterisks": [],
        },
        # Orthogonal physical bench measurement discharging `ast-thermal` via direct thermocouple telemetry.
        "0004": {
            "address": "401900",
            "length": "140",
            "created_at": "2026-11-26T10:50:00Z",
            "frame": {
                "population": "space:bench/thermal_sweep[1..60]",
                "n": 60,
                "unit": "kelvin_sample",
                "window": {"seconds": 600},
                "method": "method:direct_thermocouple_trace_v1",
            },
            "measure": {"max_delta_k": 0.04},
            "discharges": ["ast-thermal"],
            "asterisks": [],
        },
    }

    edges_before_orthogonal = [
        {"from": "0002", "to": "0001", "type": "ref.derives"},
        {"from": "0003", "to": "0002", "type": "ref.cites"},
    ]

    # Case A: Before orthogonal physical check `0004` is present in the graph
    docs_no_orth = {k: v for k, v in docs.items() if k != "0004"}
    audit_a = audit_document_graph(docs_no_orth, edges_before_orthogonal)

    expect(audit_a["0001"]["frame_state"] == "valid", "0001 valid bounded frame with offload metadata")
    expect(audit_a["0001"]["collapse_drift"] is None, "0001 has no upstream open asterisks")

    # 0002 attempted same-method discharge of ast-zne-poly -> rejected; keeps 1 of 3 -> collapse_drift == 2
    expect(audit_a["0002"]["n_open"] == 3, f"0002 n_open expected 3, got {audit_a['0002']['n_open']}")
    expect(audit_a["0002"]["kept_asterisks"] == 1, "0002 kept 1 asterisk")
    expect(audit_a["0002"]["collapse_drift"] == 2, f"0002 collapse_drift expected 2, got {audit_a['0002']['collapse_drift']}")
    expect(audit_a["0002"]["chain_depth"] == 1, "0002 chain_depth == 1")
    expect(audit_a["0002"]["collapse_triplet"] == {"surplus": 1, "direction": "ohn", "thal": 1, "thom": 3},
           f"0002 triplet mismatch: {audit_a['0002']['collapse_triplet']}")

    # 0003 dropped all 3 across 2 hops AND failed homogeneous-unit frame check
    expect(audit_a["0003"]["frame_state"] == "unframed", "0003 heterogeneous unit rejected as unframed")
    expect("heterogeneous_unit_sum" in audit_a["0003"]["frame_reasons"], "0003 flagged heterogeneous_unit_sum")
    expect(audit_a["0003"]["n_open"] == 3 and audit_a["0003"]["collapse_drift"] == 3, "0003 collapse_drift 3 of 3")
    expect(audit_a["0003"]["chain_depth"] == 2, "0003 chain_depth == 2")

    # Case B: Add orthogonal bench measurement `0004` -> `ast-thermal` discharges cleanly; 2 statistical asterisks remain open!
    audit_b = audit_document_graph(docs, edges_before_orthogonal)
    expect(audit_b["0002"]["n_open"] == 2, f"orthogonal method reduced n_open from 3 to 2, got {audit_b['0002']['n_open']}")
    expect(audit_b["0002"]["collapse_drift"] == 2, "remaining 2 statistical asterisks still unkept on 0002")
    expect(audit_b["0003"]["n_open"] == 2 and audit_b["0003"]["collapse_drift"] == 2, "0003 now has 2 open upstream asterisks")

    for f in failures:
        print("FAILED", f)
    print(f"{checks} epistemic-frame & collapse-drift checks, {len(failures)} failed")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(run_self_tests())

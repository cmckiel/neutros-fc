#!/usr/bin/env python3
"""
check_trace.py - Validate traceability of a Sphinx-Needs needs.json.

Reads a needs.json (produced by Sphinx-Needs with needs_build_json=True or
by `sphinx-build -b needs`) and asserts a set of cross-need trace rules.

The rules currently encoded:

    llr_has_impl         Every LLR has at least one impl that implements it.
    llr_has_test         Every LLR has at least one test that verifies it.
    llr_satisfies_hlr    Every LLR satisfies at least one HLR.
    hlr_satisfies_sysreq Every HLR satisfies at least one System Requirement.
    hlr_has_llr          Every HLR is satisfied by at least one LLR.
    sysreq_has_hlr       Every System Req is satisfied by at least one HLR.
    impl_targets_llr     Every impl points to an existing LLR.
    test_targets_llr     Every test verifies an existing LLR.
    design_targets_llr   Every design element realizes at least one LLR.

Exit status: 0 if clean; 1 if any rule fails or any schema violation is
surfaced. Individual rules can be disabled with --skip <rule_id>.

Optionally surfaces issues from schema_violations.json so CI gets one
unified pass/fail signal across both validators.

Usage:
    python tools/check_trace.py docs/_build/html/needs.json
    python tools/check_trace.py docs/_build/html/needs.json \\
        --schema-violations docs/_build/html/schema_violations.json
    python tools/check_trace.py --list   # show all rule ids
"""

import argparse
import json
import sys
from collections import defaultdict
from pathlib import Path


def outgoing_link_check(field, target_types):
    """Build a check: the need must have outgoing <field> linking to >=1
    need whose type is in target_types, and every target must exist."""
    def check(need, needs_by_id):
        targets = need.get(field) or []
        if not targets:
            return False, f"missing outgoing {field!r}"
        for t in targets:
            if t not in needs_by_id:
                return False, f"{field}={t!r} does not exist"
            actual = needs_by_id[t]["type"]
            if actual not in target_types:
                return False, (
                    f"{field}={t!r} has type {actual!r}, "
                    f"expected one of {sorted(target_types)}"
                )
        return True, None
    return check


def incoming_link_check(field, source_types):
    """Build a check: at least one need of source_types must link to this
    need via the corresponding outgoing field."""
    back_field = field + "_back"

    def check(need, needs_by_id):
        sources = need.get(back_field) or []
        if not sources:
            return False, f"no incoming {field!r}"
        for s in sources:
            src = needs_by_id.get(s)
            if src is None:
                continue  # stale backlink, unlikely
            if src["type"] not in source_types:
                return False, (
                    f"incoming {field!r} from {s!r} has type {src['type']!r}, "
                    f"expected one of {sorted(source_types)}"
                )
        return True, None
    return check


RULES = [
    {
        "id": "llr_has_impl",
        "description": "Every LLR has at least one implementation.",
        "applies_to": {"llr"},
        "check": incoming_link_check("implements", {"impl"}),
    },
    {
        "id": "llr_has_test",
        "description": "Every LLR has at least one verification.",
        "applies_to": {"llr"},
        "check": incoming_link_check("verifies", {"test"}),
    },
    {
        "id": "llr_satisfies_hlr",
        "description": "Every LLR satisfies at least one HLR.",
        "applies_to": {"llr"},
        "check": outgoing_link_check("satisfies", {"hlr"}),
    },
    {
        "id": "hlr_satisfies_sysreq",
        "description": "Every HLR satisfies at least one System Requirement.",
        "applies_to": {"hlr"},
        "check": outgoing_link_check("satisfies", {"sysreq"}),
    },
    {
        "id": "hlr_has_llr",
        "description": "Every HLR is satisfied by at least one LLR.",
        "applies_to": {"hlr"},
        "check": incoming_link_check("satisfies", {"llr"}),
    },
    {
        "id": "sysreq_has_hlr",
        "description": "Every System Requirement is satisfied by at least one HLR.",
        "applies_to": {"sysreq"},
        "check": incoming_link_check("satisfies", {"hlr"}),
    },
    {
        "id": "impl_targets_llr",
        "description": "Every impl points to an existing LLR.",
        "applies_to": {"impl"},
        "check": outgoing_link_check("implements", {"llr"}),
    },
    {
        "id": "test_targets_llr",
        "description": "Every test verifies an existing LLR.",
        "applies_to": {"test"},
        "check": outgoing_link_check("verifies", {"llr"}),
    },
    {
        "id": "design_targets_llr",
        "description": "Every design element realizes at least one LLR.",
        "applies_to": {"design"},
        "check": outgoing_link_check("realizes", {"llr"}),
    },
]


def load_needs(path):
    with path.open() as f:
        data = json.load(f)
    versions = data.get("versions") or {}
    if not versions:
        sys.exit(f"error: {path} has no versions block")
    version_key = data.get("current_version") or next(iter(versions))
    if version_key not in versions:
        version_key = next(iter(versions))
    v = versions[version_key]
    return v.get("needs", {}), version_key or "<unversioned>"


def load_schema_violations(path):
    if not path or not path.exists():
        return []
    with path.open() as f:
        data = json.load(f)
    items = []
    for severity, entries in (data.get("validation_warnings") or {}).items():
        if isinstance(entries, list):
            for e in entries:
                items.append((severity, str(e)))
        else:
            items.append((severity, str(entries)))
    return items


def main():
    p = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    p.add_argument("needs_json", nargs="?", type=Path,
                   help="Path to needs.json (omit when using --list).")
    p.add_argument("--skip", action="append", default=[], metavar="RULE_ID",
                   help="Skip a rule by id (repeatable).")
    p.add_argument("--list", action="store_true",
                   help="List rule ids and descriptions, then exit.")
    p.add_argument("--schema-violations", type=Path, default=None,
                   help="Optional schema_violations.json to surface alongside.")
    args = p.parse_args()

    if args.list:
        print("Trace rules:")
        for r in RULES:
            print(f"  {r['id']:24s} {r['description']}")
        return

    if args.needs_json is None:
        p.error("needs_json path is required (or pass --list)")

    needs, version = load_needs(args.needs_json)
    print(f"Loaded {len(needs)} need(s) from {args.needs_json} "
          f"(version: {version})")

    skipped = set(args.skip)
    violations = defaultdict(list)

    for rule in RULES:
        if rule["id"] in skipped:
            continue
        for nid, need in sorted(needs.items()):
            if need["type"] not in rule["applies_to"]:
                continue
            ok, reason = rule["check"](need, needs)
            if not ok:
                violations[rule["id"]].append((nid, reason))

    schema_issues = load_schema_violations(args.schema_violations)

    total_trace = sum(len(v) for v in violations.values())
    total_problems = total_trace + len(schema_issues)

    if total_problems == 0:
        print("OK: no trace or schema violations.")
        if skipped:
            print(f"(skipped {len(skipped)} rule(s): {sorted(skipped)})")
        return

    if violations:
        print("\nTrace violations:")
        for rule in RULES:
            vs = violations.get(rule["id"], [])
            if not vs:
                continue
            print(f"\n  [{rule['id']}] {rule['description']}")
            for nid, reason in vs:
                print(f"    {nid}: {reason}")

    if schema_issues:
        print(f"\nSchema violations ({len(schema_issues)}):")
        for severity, msg in schema_issues:
            print(f"  [{severity}] {msg}")

    print(f"\nFAILED: {total_problems} problem(s).")
    sys.exit(1)


if __name__ == "__main__":
    main()

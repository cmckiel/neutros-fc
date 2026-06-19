#!/usr/bin/env python3
"""
Extract @impl and @verifies trace tags from source code and tests,
emit a Sphinx-Needs RST file linking implementations and verifications to LLRs.

For each @impl tag, the script parses the C function declaration immediately
below and emits a Sphinx :c:func: cross-reference to it.

For each @verifies tag, the script identifies the verification target below
the tag:
    - GTest TEST/TEST_F/TEST_P/TYPED_TEST macros          -> method=GTest
    - pytest def test_* functions                          -> method=pytest
    - Any other Python callable                            -> method=analysis
    - Anything else                                        -> method=test

When the target name cannot be auto-detected, the need still emits with a
file:line fallback so the trace link is preserved.

Required Sphinx-Needs configuration (in conf.py):

    needs_extra_options = ["method"]

Tag syntax (single line, in any comment style):

    @impl LLR-IMU-001
    @impl LLR-IMU-001, LLR-IMU-002
    \\impl LLR-IMU-001              (Doxygen backslash form also accepted)
    @verifies LLR-IMU-001

Usage:

    python tools/extract_trace_tags.py \\
        --src src lib \\
        --tests tests tools \\
        --output docs/_generated/trace_tags.rst
"""

import argparse
import re
import sys
from collections import defaultdict
from pathlib import Path

# Tag scanner. Captures (kind, comma-separated id list).
TAG_RE = re.compile(
    r"[@\\](impl|verifies)\s+"
    r"([A-Z][A-Z0-9_-]*(?:\s*,\s*[A-Z][A-Z0-9_-]*)*)"
)

# GTest macro forms. Full test name is Suite.Name.
GTEST_RE = re.compile(
    r"\b(TEST|TEST_F|TEST_P|TYPED_TEST|TYPED_TEST_P)\s*"
    r"\(\s*(\w+)\s*,\s*(\w+)\s*\)"
)

# pytest function. Must start with test_.
PYTEST_RE = re.compile(r"^\s*def\s+(test_\w+)\s*\(", re.MULTILINE)

# Generic Python def (for analysis scripts that aren't pytest tests).
PYDEF_RE = re.compile(r"^\s*def\s+(\w+)\s*\(", re.MULTILINE)

SOURCE_EXTENSIONS = {".c", ".h", ".cpp", ".hpp", ".cc", ".hh", ".cxx"}
TEST_EXTENSIONS = SOURCE_EXTENSIONS | {".py"}
LOOKAHEAD_LINES = 25


def clean_chunk(text):
    """Strip Doxygen comment continuations and preprocessor noise."""
    if "*/" in text:
        text = text.split("*/", 1)[1]
    text = re.sub(r"(?m)^\s*\*\s?", "", text)
    text = re.sub(r"//[^\n]*", "", text)
    text = re.sub(r"(?m)^\s*#.*$", "", text)
    return text


def chunk_after_tag(lines, tag_line, tag_col_end):
    """Build a code chunk starting just after the tag's match end."""
    rest_of_line = lines[tag_line][tag_col_end:]
    tail = "\n".join(lines[tag_line + 1 : tag_line + 1 + LOOKAHEAD_LINES])
    return clean_chunk(rest_of_line + "\n" + tail)


KEYWORDS_TO_SKIP = {
    "if", "while", "for", "switch", "return", "sizeof",
    "typeof", "alignas", "alignof", "_Generic", "_Alignof",
    "static_assert", "_Static_assert",
}


def extract_c_function(chunk):
    """
    Find the function name in a chunk of C code below a Doxygen tag.

    Strategy: walk the chunk character by character, tracking paren depth.
    The function name is the first identifier followed by '(' at depth 0
    that is not a control-flow keyword or a __-prefixed decorator like
    __attribute__ or __declspec. This correctly skips constructs such as
    __attribute__((noreturn)) and __attribute__((format(printf, 1, 2))).

    Returns None if no plausible function name is found, in which case the
    caller falls back to a file:line reference.
    """
    depth = 0
    i = 0
    while i < len(chunk):
        c = chunk[i]
        if c == "(":
            if depth == 0:
                j = i - 1
                while j >= 0 and chunk[j].isspace():
                    j -= 1
                end = j + 1
                while j >= 0 and (chunk[j].isalnum() or chunk[j] == "_"):
                    j -= 1
                ident = chunk[j + 1 : end]
                if (
                    ident
                    and not ident.startswith("__")
                    and ident not in KEYWORDS_TO_SKIP
                ):
                    return ident
            depth += 1
        elif c == ")":
            depth -= 1
        i += 1
    return None


def extract_verification_target(chunk, file_suffix):
    """
    Identify the verification target below a @verifies tag.
    Returns (name, method) or (None, None).
    """
    m = GTEST_RE.search(chunk)
    if m:
        return f"{m.group(2)}.{m.group(3)}", "GTest"

    if file_suffix == ".py":
        m = PYTEST_RE.search(chunk)
        if m:
            return m.group(1), "pytest"
        m = PYDEF_RE.search(chunk)
        if m:
            return m.group(1), "analysis"
        return None, "analysis"  # script-level @verifies, no def

    name = extract_c_function(chunk)
    if name:
        return name, "test"
    return None, "test"


def find_tags(root, extensions):
    """Yield (path, lines, line_no_0based, match) for every tag."""
    for path in sorted(root.rglob("*")):
        if not path.is_file() or path.suffix not in extensions:
            continue
        try:
            text = path.read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        lines = text.splitlines()
        for line_no, line in enumerate(lines):
            for match in TAG_RE.finditer(line):
                yield path, lines, line_no, match


def sanitize(name):
    return re.sub(r"[^A-Za-z0-9]", "_", name)


def collect(dirs, extensions, expected_kind, label):
    """Walk dirs, gather tag records, warn on misplaced tags."""
    matching, misplaced = [], []
    for d in dirs:
        if not d.exists():
            print(f"warning: {label} dir does not exist: {d}", file=sys.stderr)
            continue
        for path, lines, line_no, match in find_tags(d, extensions):
            kind = match.group(1)
            ids = [s.strip() for s in match.group(2).split(",")]
            chunk = chunk_after_tag(lines, line_no, match.end())

            if kind == "impl":
                target_name = extract_c_function(chunk)
                target_method = None
            else:
                target_name, target_method = extract_verification_target(
                    chunk, path.suffix
                )

            record = (path, line_no + 1, kind, ids, target_name, target_method)
            if kind == expected_kind:
                matching.append(record)
            else:
                print(
                    f"warning: @{kind} tag in {label} tree at "
                    f"{path}:{line_no + 1} (expected @{expected_kind} here)",
                    file=sys.stderr,
                )
                misplaced.append(record)
    return matching, misplaced


def relpath(path, root):
    try:
        return path.relative_to(root)
    except ValueError:
        return path


def emit_impl(lines, record, counter, repo_root):
    path, line_no, _, llr_ids, func_name, _ = record
    rel = relpath(path, repo_root)
    anchor = sanitize(func_name) if func_name else sanitize(path.stem)
    for llr_id in llr_ids:
        base = f"IMPL_{llr_id}_{anchor}"
        counter[base] += 1
        n = counter[base]
        need_id = base if n == 1 else f"{base}_{n}"
        title = f"{func_name}()" if func_name else f"{rel}:{line_no}"
        lines += [
            f".. impl:: {title}",
            f"   :id: {need_id}",
            f"   :implements: {llr_id}",
            "",
        ]
        if func_name:
            lines.append(
                f"   Implemented by :c:func:`{func_name}` "
                f"in ``{rel}`` (line {line_no})."
            )
        else:
            lines.append(
                f"   Implementation in ``{rel}`` at line {line_no} "
                f"(function name not auto-detected; consider simplifying "
                f"the declaration or adding the tag immediately above it)."
            )
        lines.append("")


def emit_verification(lines, record, counter, repo_root):
    path, line_no, _, llr_ids, name, method = record
    rel = relpath(path, repo_root)
    anchor = sanitize(name) if name else sanitize(path.stem)
    for llr_id in llr_ids:
        base = f"TEST-{llr_id}-{anchor}"
        counter[base] += 1
        n = counter[base]
        need_id = base if n == 1 else f"{base}-{n}"
        title = name if name else f"{rel}:{line_no}"
        lines += [
            f".. test:: {title}",
            f"   :id: {need_id}",
            f"   :verifies: {llr_id}",
            f"   :method: {method}",
            "",
        ]
        if name:
            lines.append(
                f"   Verified by ``{name}`` ({method}) "
                f"in ``{rel}`` (line {line_no})."
            )
        else:
            lines.append(
                f"   Verified in ``{rel}`` at line {line_no} "
                f"(target name not auto-detected)."
            )
        lines.append("")


def emit_rst(impls, verifications, output, repo_root):
    out = [
        ".. AUTO-GENERATED by tools/extract_trace_tags.py. Do not edit by hand.",
        ".. Edit @impl / @verifies tags in source files instead.",
        "",
        "Code Trace Tags",
        "===============",
        "",
        "Implementations",
        "---------------",
        "",
    ]
    impl_counter = defaultdict(int)
    for record in impls:
        emit_impl(out, record, impl_counter, repo_root)

    out += ["Verifications", "-------------", ""]
    verify_counter = defaultdict(int)
    for record in verifications:
        emit_verification(out, record, verify_counter, repo_root)

    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text("\n".join(out), encoding="utf-8")


def main():
    p = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    p.add_argument("--src", nargs="+", type=Path,
                   default=[Path("src"), Path("lib")])
    p.add_argument("--tests", nargs="+", type=Path,
                   default=[Path("tests")])
    p.add_argument("--output", type=Path,
                   default=Path("docs/_generated/trace_tags.rst"))
    p.add_argument("--repo-root", type=Path, default=Path.cwd())
    args = p.parse_args()

    src_impls, src_misplaced = collect(
        args.src, SOURCE_EXTENSIONS, "impl", "src"
    )
    test_verifies, test_misplaced = collect(
        args.tests, TEST_EXTENSIONS, "verifies", "tests"
    )

    impls = src_impls + test_misplaced
    verifications = test_verifies + src_misplaced

    emit_rst(impls, verifications, args.output, args.repo_root)
    print(
        f"wrote {len(impls)} impl(s) and "
        f"{len(verifications)} verification(s) to {args.output}"
    )


if __name__ == "__main__":
    main()

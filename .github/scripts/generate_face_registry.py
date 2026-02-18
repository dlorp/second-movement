#!/usr/bin/env python3
"""
generate_face_registry.py

Auto-generates builder/face_registry.json by scanning the repo source.

Usage:
    python3 .github/scripts/generate_face_registry.py [--repo-root PATH] [--output PATH] [--check]

Options:
    --repo-root PATH   Root of the repo (default: directory two levels up from this script)
    --output PATH      Output file path (default: builder/face_registry.json)
    --check            CI mode: fail with exit code 1 if generated output differs from existing file

AXIOM-037: No Unicode or emoji in this file. Plain ASCII only.
"""

import argparse
import json
import os
import re
import sys

CATEGORIES = [
    {"id": "clock",        "name": "Clock"},
    {"id": "complication", "name": "Complication"},
    {"id": "sensor",       "name": "Sensor"},
    {"id": "demo",         "name": "Demo"},
    {"id": "io",           "name": "IO"},
    {"id": "settings",     "name": "Settings"},
]

CATEGORY_IDS = {c["id"] for c in CATEGORIES}


def repo_root_from_script():
    """Compute repo root as two directories up from this script."""
    script_dir = os.path.dirname(os.path.abspath(__file__))
    return os.path.normpath(os.path.join(script_dir, "..", ".."))


def parse_srcs_from_mk(mk_path):
    """Parse watch-faces.mk and return list of .c source file paths (relative to repo root)."""
    srcs = []
    with open(mk_path, "r") as f:
        content = f.read()

    # Match lines like:  ./watch-faces/category/name.c \
    pattern = re.compile(r"^\s+(\.\/watch-faces\/[^\s\\#]+\.c)", re.MULTILINE)
    for match in pattern.finditer(content):
        path = match.group(1).strip()
        srcs.append(path)
    return srcs


def face_id_from_header(header_path):
    """
    Extract the watch_face_t macro name from a header file.

    Handles patterns:
      1. Same line:  #define face_name ((const watch_face_t){ ...
      2. Line cont:  #define face_name                              \
                         ((const watch_face_t){ ...

    Returns the face id string, or None if not found.
    """
    if not os.path.exists(header_path):
        return None
    with open(header_path, "r", errors="replace") as f:
        content = f.read()

    # Pattern 1: #define <identifier> ((const watch_face_t) on the same line
    pattern1 = re.compile(
        r"^\s*#\s*define\s+([A-Za-z_][A-Za-z0-9_]*)\s+\(\s*\(\s*const\s+watch_face_t\s*\)",
        re.MULTILINE,
    )
    match = pattern1.search(content)
    if match:
        return match.group(1)

    # Pattern 2: #define <identifier> <whitespace>\  followed by ((const watch_face_t)
    # Join line continuations then re-search
    joined = re.sub(r"\\\n\s*", " ", content)
    match2 = pattern1.search(joined)
    if match2:
        return match2.group(1)

    return None


def description_from_header(header_path):
    """
    Extract a best-effort description from the header file.

    Strategy:
    1. Look for the named face comment block (e.g. /* CLOCK FACE\n * desc... */)
       that appears AFTER the MIT license block.
    2. Fall back to the first non-license /* ... */ block.
    3. Fall back to the first // comment that looks like a description.

    Returns a single-line ASCII string, or empty string if nothing found.
    """
    if not os.path.exists(header_path):
        return ""

    with open(header_path, "r", errors="replace") as f:
        content = f.read()

    # Remove the MIT license block first (it always starts with /* MIT License)
    mit_pattern = re.compile(
        r"/\*\s*(?:SPDX-License-Identifier:[^\n]*)?\s*\n\s*\*\s*MIT License.*?\*/",
        re.DOTALL | re.IGNORECASE,
    )
    stripped = mit_pattern.sub("", content, count=1)

    # Look for a /* ... */ block that starts with a face name line
    # e.g. "/* CLOCK FACE\n * ...\n */"
    block_pattern = re.compile(r"/\*(?P<body>.*?)\*/", re.DOTALL)
    for match in block_pattern.finditer(stripped):
        body = match.group("body")
        # Clean up lines
        lines = body.split("\n")
        clean_lines = []
        for line in lines:
            # Strip leading " * " or " *"
            line = re.sub(r"^\s*\*\s?", "", line).strip()
            if line:
                clean_lines.append(line)

        if not clean_lines:
            continue

        # Skip blocks that are just copyright/license remnants
        first_line = clean_lines[0].upper()
        if "COPYRIGHT" in first_line or "LICENSE" in first_line or "SPDX" in first_line:
            continue

        # Collect description lines (skip the face title line at top)
        desc_lines = []
        for i, line in enumerate(clean_lines):
            # Skip the face-name header line (e.g. "CLOCK FACE" or "WYOSCAN .5 hz watchface")
            if i == 0 and re.match(r"^[A-Z0-9 _/-]+$", line.upper()) and len(line) < 60:
                continue
            # Skip lines that are just URLs
            if line.startswith("http://") or line.startswith("https://"):
                continue
            desc_lines.append(line)
            if len(desc_lines) >= 3:
                break

        if desc_lines:
            desc = " ".join(desc_lines)
            # Strip any non-ASCII characters (AXIOM-037)
            desc = desc.encode("ascii", errors="ignore").decode("ascii")
            # Collapse whitespace
            desc = re.sub(r"\s+", " ", desc).strip()
            if desc:
                return desc

    return ""


def name_from_id(face_id):
    """
    Convert face id to display name.
    e.g. clock_face -> Clock
         wyoscan_face -> Wyoscan
         fast_stopwatch_face -> Fast Stopwatch
         lis2dw_monitor_face -> Lis2dw Monitor
    """
    # Strip trailing _face
    name = re.sub(r"_face$", "", face_id)
    # Replace underscores with spaces
    name = name.replace("_", " ")
    # Title-case each word
    name = " ".join(w.capitalize() for w in name.split())
    return name


def parse_default_faces(config_path):
    """
    Parse movement_config.h to find the ordered list of default faces.
    Returns a dict: {face_id: position (0-indexed)} for faces in watch_faces[].
    """
    defaults = {}
    if not os.path.exists(config_path):
        return defaults

    with open(config_path, "r", errors="replace") as f:
        content = f.read()

    # Extract the watch_faces[] initializer block
    block_match = re.search(
        r"watch_face_t\s+watch_faces\s*\[\s*\]\s*=\s*\{([^}]+)\}",
        content,
        re.DOTALL,
    )
    if not block_match:
        return defaults

    block = block_match.group(1)
    # Find all identifiers (face ids) - these are the entries
    entries = re.findall(r"\b([A-Za-z_][A-Za-z0-9_]*_face)\b", block)
    for i, face_id in enumerate(entries):
        defaults[face_id] = i

    return defaults


def category_from_path(src_path):
    """
    Extract category from path like ./watch-faces/clock/clock_face.c -> clock
    """
    parts = src_path.replace("\\", "/").split("/")
    # Expected: ['.', 'watch-faces', '<category>', '<filename>']
    if len(parts) >= 4 and parts[1] == "watch-faces":
        cat = parts[2]
        return cat if cat in CATEGORY_IDS else cat
    return "other"


def generate_registry(repo_root):
    """Main logic: scan repo and return the registry dict."""
    mk_path = os.path.join(repo_root, "watch-faces.mk")
    config_path = os.path.join(repo_root, "movement_config.h")

    srcs = parse_srcs_from_mk(mk_path)
    defaults = parse_default_faces(config_path)

    faces = []
    seen_ids = set()

    for src in srcs:
        # src is like ./watch-faces/clock/clock_face.c
        # Build absolute paths
        src_rel = src  # keep the ./ prefix for output
        abs_src = os.path.join(repo_root, src.lstrip("./"))

        # Derive header path
        header_rel = re.sub(r"\.c$", ".h", src_rel)
        abs_header = os.path.join(repo_root, header_rel.lstrip("./"))

        # Extract face id from header
        face_id = face_id_from_header(abs_header)
        if face_id is None:
            # No watch_face_t macro found - may be a helper file (e.g. comms_rx.c)
            # Skip silently
            print(f"  [skip] {src}: no watch_face_t macro found", file=sys.stderr)
            continue

        if face_id in seen_ids:
            print(f"  [warn] duplicate face id '{face_id}' from {src}", file=sys.stderr)
            continue
        seen_ids.add(face_id)

        category = category_from_path(src)
        name = name_from_id(face_id)
        description = description_from_header(abs_header)

        is_default = face_id in defaults
        default_position = defaults.get(face_id, None)

        # requires_sensor: heuristic - sensor category or known sensor faces
        requires_sensor = None
        if category == "sensor":
            requires_sensor = "sensor"

        entry = {
            "id": face_id,
            "name": name,
            "category": category,
            "source": src_rel,
            "header": header_rel,
            "description": description,
            "requires_sensor": requires_sensor,
            "default": is_default,
            "default_position": default_position,
        }
        faces.append(entry)

    # Sort: defaults first (by position), then alphabetically by id
    def sort_key(f):
        if f["default"] and f["default_position"] is not None:
            return (0, f["default_position"], f["id"])
        return (1, 0, f["id"])

    faces.sort(key=sort_key)

    registry = {
        "version": "1.0",
        "generated_from": "watch-faces.mk",
        "faces": faces,
        "categories": CATEGORIES,
    }
    return registry


def main():
    parser = argparse.ArgumentParser(
        description="Generate builder/face_registry.json from watch-faces.mk"
    )
    parser.add_argument(
        "--repo-root",
        default=None,
        help="Root of the repo (default: two levels up from this script)",
    )
    parser.add_argument(
        "--output",
        default=None,
        help="Output file path (default: builder/face_registry.json relative to repo root)",
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="CI mode: exit 1 if generated output differs from existing file",
    )
    args = parser.parse_args()

    repo_root = args.repo_root or repo_root_from_script()
    output_path = args.output or os.path.join(repo_root, "builder", "face_registry.json")

    print(f"Scanning repo: {repo_root}", file=sys.stderr)
    registry = generate_registry(repo_root)

    generated = json.dumps(registry, indent=2, ensure_ascii=True) + "\n"

    if args.check:
        if not os.path.exists(output_path):
            print(f"ERROR: {output_path} does not exist. Run generate_face_registry.py to create it.", file=sys.stderr)
            sys.exit(1)
        with open(output_path, "r") as f:
            existing = f.read()
        if existing != generated:
            print(f"ERROR: {output_path} is out of date. Run generate_face_registry.py to update it.", file=sys.stderr)
            sys.exit(1)
        print(f"OK: {output_path} is up to date.", file=sys.stderr)
        sys.exit(0)

    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    with open(output_path, "w") as f:
        f.write(generated)

    face_count = len(registry["faces"])
    print(f"Wrote {face_count} faces to {output_path}", file=sys.stderr)


if __name__ == "__main__":
    main()

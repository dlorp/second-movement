#!/usr/bin/env python3
"""
settings_discovery.py

Scans the firmware for all configurable compile-time settings and generates
builder/settings_registry.json.

Sources scanned:
  - movement_config.h  (primary user-configurable settings)
  - movement.h         (any user-configurable #defines)

Usage:
    python3 .github/scripts/settings_discovery.py [--repo-root PATH] [--output PATH] [--check]

Options:
    --repo-root PATH   Root of the repo (default: two levels up from this script)
    --output PATH      Output file path (default: builder/settings_registry.json)
    --check            CI mode: fail with exit code 1 if generated output differs from existing file

AXIOM-037: No Unicode or emoji in this file. Plain ASCII only.
"""

import argparse
import json
import os
import re
import sys


def repo_root_from_script():
    """Compute repo root as two directories up from this script."""
    script_dir = os.path.dirname(os.path.abspath(__file__))
    return os.path.normpath(os.path.join(script_dir, "..", ".."))


def human_name_from_id(define_id):
    """
    Convert a #define name to a human-readable label.
    e.g. MOVEMENT_DEFAULT_24H_MODE -> 24H Mode
         MOVEMENT_DEFAULT_RED_COLOR -> Red Color
         MOVEMENT_DEBOUNCE_TICKS -> Debounce Ticks
         SIGNAL_TUNE_DEFAULT -> Signal Tune
    """
    name = define_id

    # Special cases that need custom names
    special = {
        "SIGNAL_TUNE_DEFAULT": "Signal Tune",
        "MOVEMENT_SECONDARY_FACE_INDEX": "Secondary Face Index",
        "MOVEMENT_NUM_FACES": "Number of Faces",
    }
    if name in special:
        return special[name]

    # Strip common prefixes
    for prefix in ("MOVEMENT_DEFAULT_", "MOVEMENT_", "DEFAULT_"):
        if name.startswith(prefix):
            name = name[len(prefix):]
            break

    # Replace underscores with spaces, title-case each word
    parts = name.split("_")
    name = " ".join(w.capitalize() for w in parts if w)
    return name


def parse_value(raw):
    """
    Parse a raw #define value string into a Python value and infer type.

    Returns (value, type_str) where type_str is 'bool', 'int', 'string', or 'choice'.
    """
    v = raw.strip()

    # Boolean literals
    if v.lower() == "true":
        return (True, "bool")
    if v.lower() == "false":
        return (False, "bool")

    # Hex integer
    hex_match = re.match(r"^0[xX]([0-9A-Fa-f]+)$", v)
    if hex_match:
        return (int(hex_match.group(1), 16), "int")

    # Decimal integer (possibly negative)
    int_match = re.match(r"^-?\d+$", v)
    if int_match:
        return (int(v), "int")

    # Parenthesised integer or expression like (MOVEMENT_NUM_FACES - 5)
    paren_match = re.match(r"^\((.+)\)$", v)
    if paren_match:
        inner = paren_match.group(1).strip()
        # Try simple int
        try:
            return (int(inner), "int")
        except ValueError:
            pass
        # Return as a string expression
        return (v, "choice")

    # Named constant or enum value (e.g. WATCH_BUZZER_VOLUME_SOFT)
    if re.match(r"^[A-Z_][A-Z0-9_]*$", v):
        return (v, "choice")

    # Empty value (bare #define with no value) -> treated as bool flag
    if v == "":
        return (True, "bool")

    # Fall through: treat as string
    v_ascii = v.encode("ascii", errors="ignore").decode("ascii")
    return (v_ascii, "string")


def strip_comment_delimiters(line):
    """
    Strip C comment delimiters from a single line.
    Handles lines like '/* text */', ' * text', ' */', '/*', etc.
    Returns plain text or empty string.
    """
    s = line.strip()
    # Remove leading /* or */ or * (in any combination)
    s = re.sub(r"^/?\*+/?", "", s).strip()
    # Remove trailing */ or *
    s = re.sub(r"/?\*+/?$", "", s).strip()
    return s


def extract_comment_before(lines, line_index):
    """
    Look backwards from line_index for adjacent comment lines or a block comment.
    Returns a single-line description string.
    Stops at non-comment, non-blank lines (e.g. #define or code).
    """
    # Collect // comment lines immediately above
    i = line_index - 1
    inline_comments = []
    while i >= 0:
        stripped = lines[i].strip()
        if stripped.startswith("//"):
            text = stripped.lstrip("/").strip()
            inline_comments.insert(0, text)
        elif stripped == "" and inline_comments:
            break
        else:
            break
        i -= 1

    if inline_comments:
        return " ".join(inline_comments)

    # Look for a closing */ immediately above (one line gap allowed for blank)
    i = line_index - 1
    while i >= 0 and lines[i].strip() == "":
        i -= 1

    if i < 0:
        return ""

    top_stripped = lines[i].strip()

    # Single-line block comment: /* text */
    single_line_match = re.match(r"^/\*(.+?)\*/$", top_stripped)
    if single_line_match:
        text = single_line_match.group(1).strip()
        text = text.encode("ascii", errors="ignore").decode("ascii")
        return re.sub(r"\s+", " ", text).strip()

    # Multi-line block comment: ends with */
    if top_stripped.endswith("*/"):
        # i is at the closing */ line - scan backward to find /*
        j = i - 1
        block_lines = []
        while j >= 0:
            stripped = lines[j].strip()
            if stripped.startswith("/*"):
                # Opening delimiter - may have text after /*
                m = re.match(r"^/\*+\s*(.*)", stripped)
                if m:
                    text = m.group(1).rstrip("*/").strip()
                    if text:
                        block_lines.insert(0, text)
                break
            elif stripped == "*/":
                # Nested or mismatched - stop
                break
            elif stripped.startswith("*"):
                # Body line: strip leading * and whitespace
                text = stripped.lstrip("*").strip()
                if text:
                    block_lines.insert(0, text)
            else:
                # Hit code - not part of this block
                break
            j -= 1

        if block_lines:
            result = " ".join(block_lines)
            result = result.encode("ascii", errors="ignore").decode("ascii")
            return re.sub(r"\s+", " ", result).strip()

    return ""


def extract_inline_comment(line):
    """Extract // comment from end of a #define line."""
    # Match // comment after the value
    match = re.search(r"//(.*)", line)
    if match:
        return match.group(1).strip()
    return ""


def extract_range_from_comments(comments_text):
    """
    Try to extract min/max from comment text like:
      'Valid values are: 0: Never, 1: 10 minutes, ..., 7: 7 days'
      or '0-15' or '0=off, 15=max'
    Returns (min_val, max_val) or (None, None).
    """
    # Pattern: "0-15" or "0..15"
    range_match = re.search(r"\b(\d+)\s*[-\.]{1,2}\s*(\d+)\b", comments_text)
    if range_match:
        return (int(range_match.group(1)), int(range_match.group(2)))

    # Pattern: enumerate "0: ..., 1: ..., N: ..." - find max N
    entries = re.findall(r"\b(\d+)\s*:", comments_text)
    if len(entries) >= 2:
        nums = [int(e) for e in entries]
        return (min(nums), max(nums))

    return (None, None)


def collect_context_comment(lines, define_line_index):
    """
    Gather all comment lines in the vicinity of a #define:
    - inline comment on the same line
    - comment block immediately above
    Returns a combined description string (ASCII only).
    """
    line = lines[define_line_index]
    inline = extract_inline_comment(line)
    above = extract_comment_before(lines, define_line_index)

    parts = []
    if above:
        parts.append(above)
    if inline and inline not in above:
        parts.append(inline)

    result = " ".join(parts)
    # Strip non-ASCII (AXIOM-037)
    result = result.encode("ascii", errors="ignore").decode("ascii")
    result = re.sub(r"\s+", " ", result).strip()
    return result


def collect_block_context(lines, define_line_index):
    """
    Collect all comment lines in the contiguous block above the #define.
    Looks up to 30 lines above.
    Returns the raw text of the whole block.
    """
    collected = []
    i = define_line_index - 1
    limit = max(0, define_line_index - 30)
    while i >= limit:
        stripped = lines[i].strip()
        if stripped in ("*/", "/*", "*"):
            # Pure delimiter lines - skip but continue scanning
            i -= 1
            continue
        if stripped.startswith("//") or stripped.startswith("*") or stripped.startswith("/*") or stripped.endswith("*/"):
            text = strip_comment_delimiters(stripped)
            if text:
                collected.insert(0, text)
        elif stripped == "":
            # blank line: stop collecting
            break
        else:
            break
        i -= 1
    return " ".join(collected)


def parse_settings_from_file(filepath, skip_defines=None):
    """
    Parse a C header file and extract #define settings.

    skip_defines: set of define names to ignore (e.g. include guards, internal macros).

    Returns a list of setting dicts.
    """
    if not os.path.exists(filepath):
        return []

    if skip_defines is None:
        skip_defines = set()

    with open(filepath, "r", errors="replace") as f:
        content = f.read()

    lines = content.split("\n")
    settings = []

    for i, line in enumerate(lines):
        stripped = line.strip()
        # Match simple #define NAME VALUE or #define NAME (with no value)
        m = re.match(r"^#\s*define\s+([A-Z_][A-Z0-9_]*)\s*(.*?)(?:\s*//.*)?$", stripped)
        if not m:
            continue

        define_id = m.group(1)
        raw_value = m.group(2).strip()

        # Skip internal / structural defines
        if define_id in skip_defines:
            continue
        if define_id.endswith("_H_") or define_id.endswith("_H"):
            continue
        if define_id.startswith("__"):
            continue

        # Skip function-like macros (contain a '(')
        if "(" in raw_value and not raw_value.startswith("("):
            continue

        # Skip #define that is just another identifier with no _face or similar meaning
        # We want only settings relevant to user configuration.
        # Heuristic: the define must look like a user setting (has MOVEMENT_ or DEFAULT_ prefix,
        # or is SIGNAL_TUNE_ or DEBOUNCE or similar).
        user_setting_prefixes = (
            "MOVEMENT_DEFAULT_",
            "MOVEMENT_NUM_",
            "MOVEMENT_SECONDARY_",
            "MOVEMENT_DEBOUNCE_",
            "DEFAULT_LED_",
            "SIGNAL_TUNE_",
        )
        is_user_setting = any(define_id.startswith(p) for p in user_setting_prefixes)
        if not is_user_setting:
            continue

        value, type_str = parse_value(raw_value)

        # Gather surrounding comments for description
        description = collect_context_comment(lines, i)
        if not description:
            description = collect_block_context(lines, i)

        entry = {
            "id": define_id,
            "name": human_name_from_id(define_id),
            "type": type_str,
            "default": value,
            "description": description,
        }

        # For int type, try to extract range from nearby comments
        if type_str == "int":
            # Look for range info in the block above
            block_text = collect_block_context(lines, i)
            combined = description + " " + block_text
            min_v, max_v = extract_range_from_comments(combined)
            if min_v is not None:
                entry["min"] = min_v
                entry["max"] = max_v

        # For choice type, try to enumerate choices from block comments
        if type_str == "choice":
            block_text = collect_block_context(lines, i)
            combined = description + " " + block_text
            # Find "N: description" entries
            choice_matches = re.findall(r"\b(\d+)\s*:\s*([^,\n]+)", combined)
            if len(choice_matches) >= 2:
                entry["choices"] = [
                    {"value": int(num), "label": label.strip()}
                    for num, label in choice_matches
                ]

        settings.append(entry)

    return settings


def generate_settings_registry(repo_root):
    """Main logic: scan repo and return the settings registry dict."""
    config_path = os.path.join(repo_root, "movement_config.h")
    movement_path = os.path.join(repo_root, "movement.h")

    # Defines that are structural, not user settings
    skip = {
        "MOVEMENT_CONFIG_H_",
        "MOVEMENT_H_",
        "MOVEMENT_NUM_FACES",
    }

    settings = []
    seen_ids = set()

    for path in [config_path, movement_path]:
        extracted = parse_settings_from_file(path, skip_defines=skip)
        for entry in extracted:
            if entry["id"] not in seen_ids:
                seen_ids.add(entry["id"])
                settings.append(entry)

    registry = {
        "version": "1.0",
        "settings": settings,
    }
    return registry


def main():
    parser = argparse.ArgumentParser(
        description="Generate builder/settings_registry.json from movement_config.h"
    )
    parser.add_argument(
        "--repo-root",
        default=None,
        help="Root of the repo (default: two levels up from this script)",
    )
    parser.add_argument(
        "--output",
        default=None,
        help="Output file path (default: builder/settings_registry.json relative to repo root)",
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="CI mode: exit 1 if generated output differs from existing file",
    )
    args = parser.parse_args()

    repo_root = args.repo_root or repo_root_from_script()
    output_path = args.output or os.path.join(repo_root, "builder", "settings_registry.json")

    print(f"Scanning repo: {repo_root}", file=sys.stderr)
    registry = generate_settings_registry(repo_root)

    generated = json.dumps(registry, indent=2, ensure_ascii=True) + "\n"

    if args.check:
        if not os.path.exists(output_path):
            print(
                f"ERROR: {output_path} does not exist. Run settings_discovery.py to create it.",
                file=sys.stderr,
            )
            sys.exit(1)
        with open(output_path, "r") as f:
            existing = f.read()
        if existing != generated:
            print(
                f"ERROR: {output_path} is out of date. Run settings_discovery.py to update it.",
                file=sys.stderr,
            )
            sys.exit(1)
        print(f"OK: {output_path} is up to date.", file=sys.stderr)
        sys.exit(0)

    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    with open(output_path, "w") as f:
        f.write(generated)

    count = len(registry["settings"])
    print(f"Wrote {count} settings to {output_path}", file=sys.stderr)


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""
generate_registry.py - Scan watch-faces/ and produce builder/face_registry.json.

Run automatically by sync-registry.yml on pushes to main that touch watch-faces/.
Can also be run manually: python3 .github/scripts/generate_registry.py
"""

import argparse
import json
import os
import re
import sys


WATCH_FACES_ROOT = "watch-faces"

CATEGORY_NAMES = {
    "clock":       "Clock",
    "complication": "Complication",
    "sensor":      "Sensor",
    "demo":        "Demo",
    "io":          "I/O",
    "settings":    "Settings",
}

DEFAULT_FACES = {
    "clock_face",
    "alarm_face",
    "stopwatch_face",
    "set_time_face",
    "settings_face",
}


def read_description(c_path: str) -> str:
    """Try to extract a one-line description from the matching .h file."""
    h_path = c_path.replace(".c", ".h")
    try:
        with open(h_path) as f:
            content = f.read(2000)
        m = re.search(r"\*\s+([A-Z][^\n*]{10,80})", content)
        if m:
            return m.group(1).strip()[:80]
    except OSError:
        pass
    return ""


def scan_faces(root: str):
    faces = []
    categories = set()

    for dirpath, _dirs, filenames in os.walk(root):
        for fname in sorted(filenames):
            if not fname.endswith(".c"):
                continue
            face_id = fname[:-2]          # strip .c
            parts = dirpath.split(os.sep)
            cat = parts[1] if len(parts) >= 2 else "other"
            categories.add(cat)

            c_path = os.path.join(dirpath, fname)
            faces.append({
                "id":          face_id,
                "name":        " ".join(
                    w.capitalize()
                    for w in face_id.replace("_face", "").split("_")
                ),
                "category":    cat,
                "path":        c_path,
                "description": read_description(c_path),
                "default":     face_id in DEFAULT_FACES,
            })

    faces.sort(key=lambda f: (f["category"], f["name"]))

    category_list = [
        {"id": c, "name": CATEGORY_NAMES.get(c, c.title())}
        for c in sorted(categories)
    ]

    return {"faces": faces, "categories": category_list}


def main():
    parser = argparse.ArgumentParser(description="Generate face_registry.json")
    parser.add_argument("--output", default="builder/face_registry.json")
    args = parser.parse_args()

    if not os.path.isdir(WATCH_FACES_ROOT):
        print(f"ERROR: {WATCH_FACES_ROOT}/ not found. Run from repo root.", file=sys.stderr)
        sys.exit(1)

    registry = scan_faces(WATCH_FACES_ROOT)

    os.makedirs(os.path.dirname(args.output) or ".", exist_ok=True)
    with open(args.output, "w") as f:
        json.dump(registry, f, indent=2)
        f.write("\n")

    print(f"Generated {args.output}: {len(registry['faces'])} faces")


if __name__ == "__main__":
    main()

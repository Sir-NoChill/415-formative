#!/usr/bin/env python3
"""Fail if a Markdown file has a prose line that could be wrapped at <=55 cols.

The repo wraps prose at 55 columns (see CONTRIBUTING.md). This check is lenient:
it flags a line only when it exceeds 55 *and* has a breakable space at or before
column 55, i.e. it could have been wrapped shorter. Lines that cannot be helped
are ignored: fenced code, tables, headings, HTML, link-reference definitions, and
lines whose first token alone already runs past 55 (long URLs, paths, `code`).
"""
import re
import sys

LIMIT = 55
LINKDEF = re.compile(r"^\[[^\]]+\]:\s")
# Leading whitespace, blockquote markers, and an optional list marker are the
# "prefix"; a break point has to fall in the content *after* it.
PREFIX = re.compile(r"^(\s*(?:>\s?)*(?:[-*+]\s+|\d+[.)]\s+)?)")


def violations(path):
    out = []
    in_fence = False
    with open(path, encoding="utf-8") as fh:
        for n, raw in enumerate(fh, 1):
            line = raw.rstrip("\n")
            s = line.lstrip()
            if s.startswith("```") or s.startswith("~~~"):
                in_fence = not in_fence
                continue
            if in_fence or not s:
                continue
            if s.startswith("#") or s.startswith("<") or "|" in line:
                continue
            if LINKDEF.match(s):
                continue
            if len(line) <= LIMIT:
                continue
            prefix = PREFIX.match(line).group(1)
            content = line[len(prefix):]
            sp = content.find(" ")
            # Wrappable only if a space in the content yields a first line <=55.
            if sp != -1 and len(prefix) + sp <= LIMIT:
                out.append((n, len(line)))
    return out


def main(argv):
    bad = False
    for path in argv[1:]:
        for n, length in violations(path):
            print(f"{path}:{n}: prose line is {length} cols (>55, wrappable)")
            bad = True
    if bad:
        print("Wrap prose to <=55 cols (see CONTRIBUTING.md).", file=sys.stderr)
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))

#!/usr/bin/env python3
"""Fail if a file contains a non-ASCII character (code point > 0x7F).

Markdown and C++ sources are kept plain ASCII: use `--` for an em dash, `->` for
an arrow, `<=`/`>=`, `...` for an ellipsis, and so on. Reports the first offending
character on each line.
"""
import sys


def offenders(path):
    out = []
    with open(path, encoding="utf-8") as fh:
        for n, line in enumerate(fh, 1):
            for col, ch in enumerate(line, 1):
                if ord(ch) > 127:
                    out.append((n, col, ch))
                    break
    return out


def main(argv):
    bad = False
    for path in argv[1:]:
        for n, col, ch in offenders(path):
            print(f"{path}:{n}:{col}: non-ASCII U+{ord(ch):04X} {ch!r}")
            bad = True
    if bad:
        print(
            "Use ASCII only in Markdown and C++ "
            "(-- em dash, -> arrow, <= >=, ... ellipsis).",
            file=sys.stderr,
        )
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))

#!/usr/bin/env python3
"""Reflow Markdown prose to <=55 columns, in place, preserving structure.

Wraps paragraphs, blockquote bodies, and list-item text at 55 columns by breaking
only at spaces (never inside a word/URL/`code`/link token). Leaves untouched:
fenced code blocks, tables, headings, HTML blocks, blank lines, and link-reference
definitions. Soft wraps inside a paragraph render as spaces, so this never changes
the rendered output. Usage: wrap-md.py FILE... (edits in place).
"""
import re
import sys
import textwrap

W = 55
MARKER = re.compile(r"^(\s*)([-*+]|\d+[.)])(\s+)(.*)$")
LINKDEF = re.compile(r"^\[[^\]]+\]:\s")


def is_special(s):
    return (
        s.startswith("#")
        or s.startswith("<")
        or "|" in s
        or bool(LINKDEF.match(s))
    )


def wrap(text, width, init="", sub=""):
    tw = textwrap.TextWrapper(
        width=max(width, 1),
        break_long_words=False,
        break_on_hyphens=False,
        initial_indent=init,
        subsequent_indent=sub,
    )
    return tw.wrap(text) or [init.rstrip()]


def process(lines, width=W):
    out = []
    i, n = 0, len(lines)
    while i < n:
        line = lines[i]
        s = line.lstrip()
        indent = line[: len(line) - len(s)]

        if s.startswith("```") or s.startswith("~~~"):
            out.append(line)
            i += 1
            while i < n:
                out.append(lines[i])
                closing = lines[i].lstrip().startswith(("```", "~~~"))
                i += 1
                if closing:
                    break
            continue

        if not s:
            out.append(line)
            i += 1
            continue

        if is_special(s):
            out.append(line)
            i += 1
            continue

        if s.startswith(">"):  # blockquote: strip one level, recurse, re-prefix
            block = []
            while i < n and lines[i].lstrip().startswith(">"):
                inner = lines[i].lstrip()[1:]
                if inner.startswith(" "):
                    inner = inner[1:]
                block.append(inner)
                i += 1
            for p in process(block, width - 2):
                out.append(("> " + p).rstrip() if p.strip() else ">")
            continue

        m = MARKER.match(line)
        if m:  # list item + its continuation lines
            lead, mark, sp, rest = m.groups()
            init = lead + mark + sp
            sub = " " * len(init)
            text = rest.strip()
            i += 1
            while i < n and lines[i].strip():
                nxt = lines[i]
                ns = nxt.lstrip()
                if MARKER.match(nxt) or ns.startswith((">", "```", "~~~")) or is_special(ns):
                    break
                text += " " + nxt.strip()
                i += 1
            out.extend(wrap(text, width, init, sub))
            continue

        para = [s]  # plain paragraph
        i += 1
        while i < n and lines[i].strip():
            nxt = lines[i]
            ns = nxt.lstrip()
            if MARKER.match(nxt) or ns.startswith((">", "```", "~~~")) or is_special(ns):
                break
            para.append(nxt.strip())
            i += 1
        out.extend(wrap(" ".join(para), width, indent, indent))
    return out


def main(argv):
    for path in argv[1:]:
        src = open(path, encoding="utf-8").read().split("\n")
        trailing = src and src[-1] == ""
        if trailing:
            src = src[:-1]
        res = process(src)
        open(path, "w", encoding="utf-8").write("\n".join(res) + "\n")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))

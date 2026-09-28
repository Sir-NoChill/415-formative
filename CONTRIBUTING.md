# Contributing

## Commit conventions

Commits follow [Conventional
Commits](https://www.conventionalcommits.org/) with a
**mandatory scope**:

```
type(scope): subject
```

- **types** are listed in [`TAGS.md`](TAGS.md).
- **scopes** are listed in [`SCOPES.md`](SCOPES.md).
- subject: imperative mood, lowercase initial, no
  trailing period, ≤ 50 chars.

Example: `feat(ex07): add Mini AST and
semantic-analysis pass`

## Git hooks

The hooks live in `.githooks/` (version-controlled).
Enable them once per clone:

```bash
git config core.hooksPath .githooks
```

- **commit-msg** validates the subject against
  `TAGS.md` / `SCOPES.md`.
- **pre-commit** runs `clang-format` on staged C++
  (LLVM style, see `.clang-format`), `shellcheck` on
  staged shell scripts, a `-Werror` build smoke test of
  the self-contained `g++` examples (ex05, ex07, ex08)
  when their sources change, and the Markdown width
  check (below) on staged `.md`.

Do not bypass the hooks (`--no-verify`); fix the cause
instead.

## Text width (55 columns)

Both code and prose wrap at **55 columns** — a
comfortable reading measure. (The reader-facing line
length on the website is set by the site's CSS
`max-width`; this 55-col rule is the source
convention.)

- **Code:** `.clang-format` sets `ColumnLimit: 55`.
  Format with `clang-format -i <file>`. Where 55 makes
  a construct obviously worse, guard it in-source with
  `// clang-format off` / `// clang-format on`; comment
  reflow is already disabled so hand-formatted teaching
  comments keep their layout.
- **Prose:** wrap Markdown at 55 with `python3
  scripts/wrap-md.py <file>...`, and check with
  `python3 scripts/check-md-width.py <file>...`. The
  check is lenient — it ignores code fences, tables,
  headings, link definitions, and lines whose first
  token alone runs past 55 (long URLs, paths, `code`).

## Building the examples

- **ex01, ex03, ex05, ex07, ex08** — a C++17 compiler
  only (`g++` or `clang++`): `g++ -std=c++17 -Wall
  -Wextra solution/main.cpp -o out && ./out`.
- **ex02, ex04** — a local LLVM/MLIR build
  (`llvm-tblgen` / `mlir-tblgen`); see each example's
  `run.sh`.
- **ex06** — an ANTLR 4 tool jar + the ANTLR C++
  runtime; `./run.sh` (`CXX=clang++ ./run.sh` to use
  Clang).

# Contributing

## Commit conventions

Commits follow [Conventional Commits](https://www.conventionalcommits.org/) with a
**mandatory scope**:

```
type(scope): subject
```

- **types** are listed in [`TAGS.md`](TAGS.md).
- **scopes** are listed in [`SCOPES.md`](SCOPES.md).
- subject: imperative mood, lowercase initial, no trailing period, ≤ 50 chars.

Example: `feat(ex07): add Mini AST and semantic-analysis pass`

## Git hooks

The hooks live in `.githooks/` (version-controlled). Enable them once per clone:

```bash
git config core.hooksPath .githooks
```

- **commit-msg** validates the subject against `TAGS.md` / `SCOPES.md`.
- **pre-commit** runs `clang-format` on staged C++ (LLVM style, see
  `.clang-format`), `shellcheck` on staged shell scripts, and a `-Werror` build
  smoke test of the self-contained `g++` examples (ex05, ex07, ex08) when their
  sources change.

Do not bypass the hooks (`--no-verify`); fix the cause instead.

## Building the examples

- **ex01, ex03, ex05, ex07, ex08** — a C++17 compiler only (`g++` or `clang++`):
  `g++ -std=c++17 -Wall -Wextra solution/main.cpp -o out && ./out`.
- **ex02, ex04** — a local LLVM/MLIR build (`llvm-tblgen` / `mlir-tblgen`); see
  each example's `run.sh`.
- **ex06** — an ANTLR 4 tool jar + the ANTLR C++ runtime; `./run.sh`
  (`CXX=clang++ ./run.sh` to use Clang).

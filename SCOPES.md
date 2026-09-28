# Commit scopes (SCOPES)

The scope names the area of the repository a commit
touches. It is **required** on every commit. The
`commit-msg` hook parses the allowed scopes from the
backtick-quoted tokens below, so keep the list in this
exact shape (`- \`scope\` — description`).

## Cross-cutting

- `repo` — top-level files: root `README.md`,
  `.gitignore`, repository layout
- `tooling` — commit/code-lint conventions, git hooks,
  `.clang-format`, `CONTRIBUTING`

## Lesson text

- `lesson-01` — Lesson 1 (Foundations) prose
- `lesson-02` — Lesson 2 (Synthesis) prose
- `lesson-03` — Lesson 3 (Types of ASTs) prose
- `lesson-04` — Lesson 4 (Developing ASTs) prose

## Examples

- `ex01` — colors X-macro
- `ex02` — hello TableGen
- `ex03` — tokenkinds X-macro analysis
- `ex04` — mini MLIR dialect (TableGen)
- `ex05` — homogeneous vs. heterogeneous AST
- `ex06` — ANTLR homogeneous parse tree
- `ex07` — Mini AST + semantic pass
- `ex08` — adapting an AST (add `for`)

If a change spans several examples in one lesson,
prefer the `lesson-0N` scope; if it spans the whole
pack, use `repo`.

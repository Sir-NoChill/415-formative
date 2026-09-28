# Commit types (TAGS)

This repository uses [Conventional Commits](https://www.conventionalcommits.org/)
with a **mandatory scope** (see `SCOPES.md`). The `commit-msg` hook in
`.githooks/` parses the allowed types from the backtick-quoted tokens below, so
keep the list in this exact shape (`- \`type\` — description`).

- `feat` — a new lesson, example, or user-facing capability
- `fix` — a bug fix in example code, a hook, or a build script
- `docs` — lesson text, READMEs, walkthroughs, or other prose only
- `style` — formatting/whitespace with no behavior change (e.g. clang-format)
- `refactor` — code change that neither fixes a bug nor adds a feature
- `perf` — a change that improves performance
- `test` — adding or correcting tests / verification harnesses
- `build` — build scripts, compiler flags, or dependency wiring (`run.sh`, etc.)
- `ci` — continuous-integration configuration
- `chore` — repository housekeeping that doesn't touch lesson content
- `revert` — reverts a previous commit

Format: `type(scope): subject` — imperative mood, lowercase initial, no trailing
period, aim for ≤ 50 characters. Breaking changes use `type(scope)!: subject`.

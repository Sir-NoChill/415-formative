#!/usr/bin/env bash
# Build the MkDocs site. Because docs_dir cannot be the repo root, we stage the
# committed lesson tree into _docs/ (deterministic: only tracked files, no build
# artifacts) and build from there. Extra args are forwarded to `mkdocs build`
# (e.g. --site-dir /tmp/site). Used by both local runs and the Pages workflow.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$root"

rm -rf _docs
mkdir -p _docs
git archive HEAD -- \
  README.md CONTRIBUTING.md TAGS.md SCOPES.md \
  lesson-01-foundations lesson-02-synthesis lesson-03-ast-types lesson-04-ast-development \
  | tar -x -C _docs

# Theme assets (U of A stylesheet, logo, favicon) — copied from the worktree so
# local builds pick up uncommitted tweaks; in CI the worktree is the checkout.
mkdir -p _docs/assets
cp -R mkdocs-assets/. _docs/assets/

mkdocs build "$@"

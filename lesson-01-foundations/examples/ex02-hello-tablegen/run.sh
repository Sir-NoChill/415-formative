#!/usr/bin/env bash
# Run a .td file through your local llvm-tblgen and show three views of it.
#
# Usage:
#   ./run.sh                 # runs solution/colors.td
#   ./run.sh starter         # runs starter/colors.td
#
# llvm-tblgen is located via $LLVM_DIR (same convention as the gazc project) or
# found on PATH. Override with:  LLVM_DIR=/path/to/llvm ./run.sh
set -euo pipefail

LLVM_DIR="${LLVM_DIR:-$HOME/Code/Compilers/LLVM/22.1.7}"
TBLGEN="$LLVM_DIR/bin/llvm-tblgen"
[ -x "$TBLGEN" ] || TBLGEN="$(command -v llvm-tblgen || true)"
[ -x "$TBLGEN" ] || { echo "error: llvm-tblgen not found. Set LLVM_DIR." >&2; exit 1; }

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WHICH="${1:-solution}"
TD="$HERE/$WHICH/colors.td"
[ -f "$TD" ] || { echo "error: no such file: $TD" >&2; exit 1; }

echo "# Using: $TBLGEN"
echo "# Input: $TD"
echo
echo "=================================================================="
echo " 1) --print-records : the parsed records, human-readable"
echo "=================================================================="
"$TBLGEN" --print-records "$TD"

echo
echo "=================================================================="
echo " 2) --dump-json : the SAME data as a machine-readable model."
echo "    Notice we can query it WITHOUT a C++ compiler -- something an"
echo "    X-macro can never do, because its data lives only inside cpp."
echo "=================================================================="
"$TBLGEN" --dump-json "$TD" | python3 -m json.tool

echo
echo "=================================================================="
echo " 3) A tiny 'backend' (show.py): pull just Name + computed Packed value."
echo "=================================================================="
"$TBLGEN" --dump-json "$TD" | python3 "$HERE/show.py"

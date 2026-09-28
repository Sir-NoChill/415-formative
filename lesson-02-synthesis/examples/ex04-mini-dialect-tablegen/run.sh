#!/usr/bin/env bash
# Configure, build, and exercise the `demo` dialect against your local MLIR.
#
# Usage:
#   ./run.sh              # build + run demo-opt on the test inputs
#   ./run.sh clean        # remove the build directory
#
# LLVM/MLIR is located via $LLVM_DIR (default: ~/Code/Compilers/LLVM/22.1.7),
# the same convention as the Gazprea project's .envrc.template.
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD="$HERE/build"

if [ "${1:-}" = "clean" ]; then rm -rf "$BUILD"; echo "removed $BUILD"; exit 0; fi

export LLVM_DIR="${LLVM_DIR:-$HOME/Code/Compilers/LLVM/22.1.7}"
export MLIR_DIR="${MLIR_DIR:-$LLVM_DIR/lib/cmake/mlir}"
echo "# LLVM_DIR = $LLVM_DIR"

cmake -G Ninja -S "$HERE" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release >/dev/null
cmake --build "$BUILD"

OPT="$BUILD/bin/demo-opt"
echo
echo "=================================================================="
echo " demo-opt test/add.mlir   (demo.add exists -> round-trips)"
echo "=================================================================="
"$OPT" "$HERE/test/add.mlir"

echo
echo "=================================================================="
echo " demo-opt test/mul.mlir   (demo.mul is the EXERCISE)"
echo "=================================================================="
if "$OPT" "$HERE/test/mul.mlir"; then
  echo "-> demo.mul round-tripped: you have applied the solution. Nice."
else
  echo
  echo "-> demo.mul is unknown, as expected in the starter state."
  echo "   Do the exercise in WALKTHROUGH.md (add Demo_MulOp to"
  echo "   include/Demo/DemoOps.td), then re-run ./run.sh."
fi

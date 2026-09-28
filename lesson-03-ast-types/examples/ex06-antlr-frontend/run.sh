#!/usr/bin/env bash
# Generate an ANTLR C++ parser from Expr.g4, compile it with the evaluator, and
# run it -- so you can see the homogeneous parse tree and the visitor over it.
#
# Usage:
#   ./run.sh                       # solution grammar, default program
#   ./run.sh starter               # starter grammar (missing '%'), default program
#   ./run.sh solution "a = 6/2;"   # run your own program string
#   ./run.sh clean                 # remove the build directory
#
# The ANTLR tool jar and C++ runtime are discovered automatically; override with
# the ANTLR_JAR and ANTLR_INS environment variables. On the CMPUT 415 machines
# the runtime lives in 415-labs/antlr-install (the same one the ANTLR lab uses).
# The C++ compiler is $CXX (default c++); e.g. `CXX=clang++ ./run.sh`.
set -euo pipefail
CXX="${CXX:-c++}"

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD="$HERE/build"

if [ "${1:-}" = "clean" ]; then rm -rf "$BUILD"; echo "removed $BUILD"; exit 0; fi

WHICH="${1:-solution}"
PROG="${2:-x = 5; y = 2 * (x + 1); z = 17 % 4;}"
SRCDIR="$HERE/$WHICH"
[ -f "$SRCDIR/Expr.g4" ] || { echo "error: no such variant: $WHICH" >&2; exit 1; }

# --- locate the ANTLR tool jar (system locations first; no sibling repo assumed) ---
find_jar() {
  for c in "${ANTLR_JAR:-}" \
           /usr/share/java/antlr-complete.jar \
           /usr/share/java/antlr-4*-complete.jar \
           /usr/local/share/java/antlr-4*-complete.jar \
           /usr/local/lib/antlr-4*-complete.jar \
           "$HOME/Code/Compilers/415-labs/antlr-install/antlr.jar"; do  # course-machine fallback
    [ -n "$c" ] && [ -f "$c" ] && { echo "$c"; return; }
  done
}
JAR="$(find_jar || true)"
[ -n "$JAR" ] || { cat >&2 <<'EOF'
error: ANTLR tool jar not found.
  Install ANTLR 4 (e.g. `antlr-4.13.2-complete.jar` from https://www.antlr.org/download.html),
  then set ANTLR_JAR=/path/to/antlr-4.x-complete.jar and re-run.
EOF
  exit 1; }

# --- locate the ANTLR C++ runtime (headers + a static OR shared lib) ---
# Checks standard install prefixes first; the 415-labs path is only a convenience
# on the course machines. Students provide it via ANTLR_INS or a system install.
find_ins() {
  for c in "${ANTLR_INS:-}" \
           /usr/local /usr \
           "$HOME/Code/Compilers/415-labs/antlr-install"; do  # course-machine fallback
    [ -n "$c" ] && [ -d "$c/include/antlr4-runtime" ] && \
      ls "$c"/lib*/libantlr4-runtime.* >/dev/null 2>&1 && { echo "$c"; return; }
  done
}
INS="$(find_ins || true)"
[ -n "$INS" ] || { cat >&2 <<'EOF'
error: ANTLR C++ runtime not found (need include/antlr4-runtime + libantlr4-runtime).
  Options:
    * install the ANTLR4 C++ runtime package for your system, or
    * build it from https://github.com/antlr/antlr4/tree/master/runtime/Cpp, then
      set ANTLR_INS=/path/to/prefix  (the prefix containing include/ and lib/).
EOF
  exit 1; }
# Prefer the static lib for a self-contained binary; fall back to the shared one.
RTLIB="$(ls "$INS"/lib*/libantlr4-runtime.a 2>/dev/null | head -1)"
[ -n "$RTLIB" ] || RTLIB="$(ls "$INS"/lib*/libantlr4-runtime.so* 2>/dev/null | head -1)"

echo "# variant : $WHICH"
echo "# jar     : $JAR"
echo "# runtime : $INS"
echo "# CXX     : $CXX"
echo

GEN="$BUILD/$WHICH/gen"
mkdir -p "$GEN"
java -jar "$JAR" -Dlanguage=Cpp -visitor -no-listener -package calc \
     -o "$GEN" "$SRCDIR/Expr.g4"

BIN="$BUILD/$WHICH/exprdemo"
"$CXX" -std=c++17 -Wall "$SRCDIR/main.cpp" "$GEN"/*.cpp \
    -I"$INS/include/antlr4-runtime" -I"$GEN" \
    "$RTLIB" -lpthread -o "$BIN"

echo "=================================================================="
"$BIN" "$PROG"

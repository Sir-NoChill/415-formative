# X-Macros & TableGen — a hands-on lesson pack

A two-lesson, example-driven course on two code-generation techniques that run
through the entire LLVM/MLIR ecosystem: **X-macros** (a C-preprocessor pattern) and
**TableGen** (LLVM's domain-specific language for generating code from structured
data).

The examples are grounded in real code on this machine — the LLVM monorepo, the
CMPUT 415 *ParserByHand* lab, and the Gazprea (`gazc`) compiler — so what you learn
here transfers directly to reading and extending those projects.

> **Audience.** Fourth-year CS students who have seen C++ but aren't fluent. Jargon
> is defined on first use; no template metaprogramming background is assumed.

## The two lessons

### [Lesson 1 — Foundations](lesson-01-foundations/) *(knowledge & comprehension)*
Define an X-macro, understand TableGen as an "advanced X-macro system," and connect
both to code hygiene.
- **ex01 — colors-xmacro:** the parallel-list bug and the X-macro that kills it. Plain `g++`.
- **ex02 — hello-tablegen:** the same list as TableGen records, run through the real `llvm-tblgen`.

### [Lesson 2 — Synthesis](lesson-02-synthesis/) *(analysis & application)*
Compare the two techniques, analyze real constructs, and extend both yourself.
- **ex03 — tokenkinds-analysis:** trace a lexer token X-macro through four expansions, then extend it. Plain `g++`.
- **ex04 — mini-dialect-tablegen:** build a real MLIR dialect with `mlir-tblgen`, then add an operation to it.

## Objective → example map

| Lesson | Objective | Where it's taught |
|---|---|---|
| 1 | Define *X-macro* | ex01 + L1 §1 |
| 1 | TableGen as an advanced X-macro system | ex02 + L1 §3 |
| 1 | Relate X-macros to code hygiene | ex01 + L1 §2 |
| 1 | Identify good uses | L1 §4 (LLVM/Clang/Gazprea) |
| 2 | Compare & contrast X-macro vs TableGen | L2 §1 (both examples) |
| 2 | Analyze a given X-macro | ex03 |
| 2 | Analyze a given TableGen construct | ex04 |
| 2 | Adapt an X-macro template | ex03 exercise |
| 2 | Adapt a set of TableGen tools | ex04 exercise |

Every example ships a **`starter/`**, a **`solution/`**, and a **`WALKTHROUGH.md`**
with the fully documented answer.

## Prerequisites & setup

- A C++17 compiler (`g++` or `clang++`) — for ex01 and ex03. No other dependencies.
- A local **LLVM/MLIR 22** build — for ex02 (`llvm-tblgen`) and ex04
  (`mlir-tblgen` + MLIR libraries). CMake ≥ 3.20 and Ninja for ex04.

The examples locate LLVM via the `LLVM_DIR` environment variable, the same
convention as the Gazprea project. Copy the template and adjust if needed:

```bash
cp .envrc.template .envrc     # then edit LLVM_DIR if your build lives elsewhere
# or just export it for the session:
export LLVM_DIR="$HOME/Code/Compilers/LLVM/22.1.7"
export MLIR_DIR="$LLVM_DIR/lib/cmake/mlir"
```

The `run.sh` scripts in ex02 and ex04 set these for you if you skip this step.

## Quick start

```bash
# Lesson 1
cd lesson-01-foundations/examples/ex01-colors-xmacro
g++ -std=c++17 -Wall -Wextra starter/main.cpp -o colors && ./colors   # see the bug
g++ -std=c++17 -Wall -Wextra solution/main.cpp -o colors && ./colors  # see the fix

cd ../ex02-hello-tablegen
./run.sh solution        # runs the real llvm-tblgen

# Lesson 2
cd ../../../lesson-02-synthesis/examples/ex03-tokenkinds-analysis
g++ -std=c++17 -Wall -Wextra solution/main.cpp -o lexdemo && ./lexdemo "1+2*(3-4)"

cd ../ex04-mini-dialect-tablegen
./run.sh                 # configures, builds a real MLIR dialect, runs demo-opt
```

## Repository layout

```
xmacro-tablegen-lessons/
├── README.md                     # you are here
├── .envrc.template               # LLVM_DIR / MLIR_DIR discovery (copy to .envrc)
├── lesson-01-foundations/
│   ├── README.md                 # the Lesson 1 text
│   └── examples/
│       ├── ex01-colors-xmacro/   # starter/ solution/ WALKTHROUGH.md
│       └── ex02-hello-tablegen/  # starter/ solution/ run.sh show.py WALKTHROUGH.md
└── lesson-02-synthesis/
    ├── README.md                 # the Lesson 2 text
    └── examples/
        ├── ex03-tokenkinds-analysis/   # starter/ solution/ WALKTHROUGH.md
        └── ex04-mini-dialect-tablegen/ # a full out-of-tree MLIR dialect + solution/
```

## Reference material used by these lessons

All paths are on this machine:

- **LLVM monorepo** — `../../LLVM/llvm-project/`
  - `llvm/include/llvm/IR/Instruction.def` — the iconic LLVM X-macro.
  - `clang/include/clang/Basic/TokenKinds.def` — the frontend token X-macro.
  - `mlir/examples/toy/` and `mlir/examples/standalone/` — canonical TableGen dialects.
- **ParserByHand lab** — `../../415-labs/ParserByHand/` — a hand-written parser built
  on token and AST X-macros (`.def` files).
- **Gazprea compiler** — `../gazc/` — uses **both** idioms in production: X-macros for
  lexer tables, TableGen (and a custom `gazprea-tblgen`) for its MLIR dialect and AST.

## A note on verification

Every code example in this pack was compiled and run against the LLVM/MLIR 22.1.7
build on this machine before being written up; the outputs quoted in the
walkthroughs are real tool output, not illustrations.

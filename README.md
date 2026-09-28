# X-Macros, TableGen & ASTs -- a hands-on lesson pack

A four-lesson, example-driven course on the
code-generation techniques and data structures at the
heart of the LLVM/MLIR ecosystem. Lessons 1-2 cover the
two code-generation techniques -- **X-macros** (a
C-preprocessor pattern) and **TableGen** (LLVM's DSL
for generating code from structured data). Lessons 3-4
cover the data structure those techniques most often
describe -- the **abstract syntax tree** -- how ASTs
are typed, what they must provide, and how you design,
decorate, and extend one. The two halves join up
literally: a real AST's enum, visitor, and RTTI are
kept in sync by exactly the X-macro / TableGen idioms
from Lessons 1-2.

The lessons draw on real production code -- the LLVM
monorepo, Clang, the CMPUT 415 *ParserByHand* and
*ANTLR* labs, and the Gazprea (`gazc`) compiler -- so
what you learn transfers directly to reading and
extending real projects. **Those are reference
material, not requirements:** every hands-on example in
this pack is self-contained (see *Prerequisites*
below), and the pointers to `gazc`, the 415 labs, and
the LLVM tree are there for when you have them, not
assumed. Lesson 3 also covers **ANTLR**, since some
students build their parsing frontend with it rather
than by hand.

> **Audience.** Fourth-year CS students who have seen
> C++ but aren't fluent. Jargon is defined on first
> use; no template metaprogramming background is
> assumed.

## The four lessons

### [Lesson 1 -- Foundations](lesson-01-foundations/) *(knowledge & comprehension)*
Define an X-macro, understand TableGen as an "advanced
X-macro system," and connect both to code hygiene.
- **ex01 -- colors-xmacro:** the parallel-list bug and
  the X-macro that kills it. Plain C++17.
- **ex02 -- hello-tablegen:** the same list as TableGen
  records, run through the real `llvm-tblgen`.

### [Lesson 2 -- Synthesis](lesson-02-synthesis/) *(analysis & application)*
Compare the two techniques, analyze real constructs,
and extend both yourself.
- **ex03 -- tokenkinds-analysis:** trace a lexer token
  X-macro through four expansions, then extend it.
  Plain C++17.
- **ex04 -- mini-dialect-tablegen:** build a real MLIR
  dialect with `mlir-tblgen`, then add an operation to
  it.

### [Lesson 3 -- Types of ASTs](lesson-03-ast-types/) *(knowledge & comprehension)*
Define homogeneous vs. heterogeneous ASTs, identify
what an AST must provide, outline the passes that walk
it, and compare the two designs.
- **ex05 -- homo-vs-hetero:** the same program built
  both ways, with the same passes over each. Plain
  `$CXX`.
- **ex06 -- antlr-frontend:** the homogeneous parse
  tree in a real **ANTLR** frontend. Java + `antlr4` +
  `$CXX`.

### [Lesson 4 -- Developing ASTs](lesson-04-ast-development/) *(analysis & application)*
Critique an existing AST, design one from scratch,
write a semantic pass over it, and extend it with a new
construct.
- **ex07 -- mini-ast:** design the "Mini" AST and
  implement name-resolution + type checking. Plain
  `$CXX`.
- **ex08 -- adapt-add-for:** add a `for` loop two ways
  (new node vs. desugar), then on real code. Plain
  `$CXX`.

## Objective -> example map

| Lesson | Objective | Where it's taught |
|---|---|---|
| 1 | Define *X-macro* | ex01 + L1 sec. 1 |
| 1 | TableGen as an advanced X-macro system | ex02 + L1 sec. 3 |
| 1 | Relate X-macros to code hygiene | ex01 + L1 sec. 2 |
| 1 | Identify good uses | L1 sec. 4 (LLVM/Clang/Gazprea) |
| 2 | Compare & contrast X-macro vs TableGen | L2 sec. 1 (both examples) |
| 2 | Analyze a given X-macro | ex03 |
| 2 | Analyze a given TableGen construct | ex04 |
| 2 | Adapt an X-macro template | ex03 exercise |
| 2 | Adapt a set of TableGen tools | ex04 exercise |
| 3 | Define homogeneous & heterogeneous ASTs | ex05 + L3 sec. 1 |
| 3 | Identify the functionality an AST must have | L3 sec. 2 (ParserByHand) |
| 3 | Outline the passes that walk an AST | ex05 + L3 sec. 3 |
| 3 | Compare & contrast homo vs hetero ASTs | ex05, ex06 + L3 sec. 4 |
| 4 | Analyze an existing AST for drawbacks | ex07 + L4 sec. 1 |
| 4 | Design an AST from scratch | ex07 + L4 sec. 2 |
| 4 | Implement a semantic analysis pass | ex07 exercise + L4 sec. 3 |
| 4 | Adapt an AST to add a construct | ex08 + L4 sec. 4 |

Every example ships a **`starter/`**, a
**`solution/`**, and a **`WALKTHROUGH.md`** with the
fully documented answer.

## Prerequisites & setup

- A C++17 compiler in `$CXX` (defaults to `c++`) -- for
  ex01, ex03, ex05, ex07, ex08. No other dependencies.
- A local **LLVM/MLIR 22** build -- for ex02
  (`llvm-tblgen`) and ex04 (`mlir-tblgen` + MLIR
  libraries). CMake >= 3.20 and Ninja for ex04.
- **ANTLR** -- for ex06: a JVM, the ANTLR tool jar, and
  the ANTLR C++ runtime. On the CMPUT 415 machines
  these are already present
  (`/usr/share/java/antlr-*-complete.jar` and
  `415-labs/antlr-install/`), and ex06's `run.sh`
  discovers them automatically; override with the
  `ANTLR_JAR` and `ANTLR_INS` environment variables if
  needed.

The examples locate LLVM via the `LLVM_DIR` environment
variable, the same convention as the Gazprea project.
Copy the template and adjust if needed:

```bash
cp .envrc.template .envrc     # then edit LLVM_DIR if your build lives elsewhere
# or just export it for the session:
export LLVM_DIR="$HOME/Code/Compilers/LLVM/22.1.7"
export MLIR_DIR="$LLVM_DIR/lib/cmake/mlir"
```

The `run.sh` scripts in ex02 and ex04 set these for you
if you skip this step.

## Quick start

```bash
# Lesson 1
cd lesson-01-foundations/examples/ex01-colors-xmacro
$CXX -std=c++17 -Wall -Wextra starter/main.cpp -o colors && ./colors   # see the bug
$CXX -std=c++17 -Wall -Wextra solution/main.cpp -o colors && ./colors  # see the fix

cd ../ex02-hello-tablegen
./run.sh solution        # runs the real llvm-tblgen

# Lesson 2
cd ../../../lesson-02-synthesis/examples/ex03-tokenkinds-analysis
$CXX -std=c++17 -Wall -Wextra solution/main.cpp -o lexdemo && ./lexdemo "1+2*(3-4)"

cd ../ex04-mini-dialect-tablegen
./run.sh                 # configures, builds a real MLIR dialect, runs demo-opt

# Lesson 3
cd ../../../lesson-03-ast-types/examples/ex05-homo-vs-hetero
$CXX -std=c++17 -Wall -Wextra solution/main.cpp -o ex05 && ./ex05   # same tree, both ways

cd ../ex06-antlr-frontend
./run.sh                 # generates an ANTLR parser, prints the homogeneous parse tree

# Lesson 4
cd ../../../lesson-04-ast-development/examples/ex07-mini-ast
$CXX -std=c++17 -Wall -Wextra solution/main.cpp -o ex07 && ./ex07   # a semantic pass

cd ../ex08-adapt-add-for
$CXX -std=c++17 -Wall -Wextra solution/main.cpp -o ex08 && ./ex08   # add a `for` two ways
```

## Repository layout

```
llvm-infra-lessons/
|-- README.md                     # you are here
|-- .envrc.template               # LLVM_DIR / MLIR_DIR discovery (copy to .envrc)
|-- lesson-01-foundations/
|   |-- README.md                 # the Lesson 1 text
|   `-- examples/
|       |-- ex01-colors-xmacro/   # starter/ solution/ WALKTHROUGH.md
|       `-- ex02-hello-tablegen/  # starter/ solution/ run.sh show.py WALKTHROUGH.md
|-- lesson-02-synthesis/
|   |-- README.md                 # the Lesson 2 text
|   `-- examples/
|       |-- ex03-tokenkinds-analysis/   # starter/ solution/ WALKTHROUGH.md
|       `-- ex04-mini-dialect-tablegen/ # a full out-of-tree MLIR dialect + solution/
|-- lesson-03-ast-types/
|   |-- README.md                 # the Lesson 3 text
|   `-- examples/
|       |-- ex05-homo-vs-hetero/  # starter/ solution/ WALKTHROUGH.md  (plain C++)
|       `-- ex06-antlr-frontend/  # starter/ solution/ run.sh WALKTHROUGH.md  (ANTLR)
`-- lesson-04-ast-development/
    |-- README.md                 # the Lesson 4 text
    `-- examples/
        |-- ex07-mini-ast/        # starter/ solution/ WALKTHROUGH.md  (plain C++)
        `-- ex08-adapt-add-for/   # starter/ solution/ WALKTHROUGH.md  (plain C++)
```

## Reference material used by these lessons

These are **external, real-world sources the lessons
point to** -- you do **not** need them to do any
example (the hands-on work is self-contained). They
live on the course machines at the paths below; if
you're off that machine, treat them as reading
references (LLVM and Clang are open source; the labs
are the CMPUT 415 handouts; `gazc` is the course
compiler).

- **LLVM monorepo** -- `../../LLVM/llvm-project/`
  - `llvm/include/llvm/IR/Instruction.def` -- the
    iconic LLVM X-macro.
  - `clang/include/clang/Basic/TokenKinds.def` -- the
    frontend token X-macro.
  - `clang/include/clang/AST/` -- the industrial
    heterogeneous AST + `RecursiveASTVisitor`.
  - `mlir/examples/toy/` and
    `mlir/examples/standalone/` -- canonical TableGen
    dialects.
- **ParserByHand lab** --
  `../../415-labs/ParserByHand/` -- a hand-written
  parser with a **heterogeneous AST** generated from an
  X-macro: `demo-finished/include/Config/ASTNodes.def`
  drives the `ASTNodeKind` enum, the RTTI range
  predicates, and the visitor (`AST.h`,
  `ASTVisitor.h`); `lib/Sema.cpp` is a real semantic
  pass. Used by Lessons 3-4.
- **ANTLR lab** -- `../../415-labs/ANTLR/` -- the
  **same "Config" language** as ParserByHand, parsed
  with ANTLR into a **homogeneous parse tree**
  (`demo-finished/grammar/Config.g4`,
  `src/ConfigEvaluator.cpp`). Used by Lesson 3 (ex06).
  The C++ runtime lives in
  `../../415-labs/antlr-install/`.
- **Gazprea compiler** -- `../gazc/` -- uses **both**
  idioms in production: X-macros for lexer tables,
  TableGen (and a custom `gazprea-tblgen`) for its MLIR
  dialect (`include/Gazprea/GazpreaOps.td`) and its
  **AST** (`include/AST/ASTNodes.td`).

### Canonical online references (for the claims in the lessons)

- LLVM-style RTTI (`isa`/`cast`/`dyn_cast`/`classof`)
  --
  <https://llvm.org/docs/HowToSetUpLLVMStyleRTTI.html>
- Clang AST, `ASTContext`, `RecursiveASTVisitor` --
  <https://clang.llvm.org/docs/IntroductionToTheClangAST.html>,
  <https://clang.llvm.org/docs/RAVFrontendAction.html>
- MLIR -- Language Reference & Operation Definition
  Spec -- <https://mlir.llvm.org/docs/LangRef/>,
  <https://mlir.llvm.org/docs/DefiningDialects/Operations/>
- ANTLR & Terence Parr, *Language Implementation
  Patterns* (the AST typing taxonomy) --
  <https://www.antlr.org/>,
  <https://pragprog.com/titles/tpdsl/language-implementation-patterns/>
- The expression problem (Wadler, 1998) --
  <https://homepages.inf.ed.ac.uk/wadler/papers/expression/expression.txt>
- Small teaching languages (context for Lesson 4's
  *Mini*) -- LLVM Kaleidoscope
  <https://llvm.org/docs/tutorial/>, *Crafting
  Interpreters* (Lox)
  <https://craftinginterpreters.com/>
- X-macros -- <https://en.wikipedia.org/wiki/X_macro> -
  Aho/Lam/Sethi/Ullman, *Compilers* (Dragon Book) --
  <https://en.wikipedia.org/wiki/Compilers:_Principles,_Techniques,_and_Tools>

## A note on verification

Every code example in this pack was compiled and run on
this machine before being written up; the outputs
quoted in the walkthroughs are real tool output, not
illustrations. Lessons 1-2's TableGen examples ran
against the LLVM/MLIR 22.1.7 build. The self-contained
C++ examples (ex01, ex03, ex05, ex07, ex08) build clean
under **both** `g++ 16` and `clang++ 22` with
`-std=c++17 -Wall -Wextra`; ex06 was generated with
ANTLR 4.13.2 and compiled against the ANTLR C++ runtime
under both compilers (`CXX=clang++ ./run.sh`). Lessons
3-4 also *use* Lessons 1-2: ex07 and ex08 generate
their `Kind` enum from an X-macro `mini_nodes.def`, the
same idiom ex03 teaches.

# ex06 -- antlr-frontend

**Lesson 3.** Define a homogeneous AST (objective 1)
and see the homo/hetero trade-off in a *real* frontend
(objective 4) -- via [ANTLR](https://www.antlr.org/),
because some students build their parser with it. Needs
a **JVM**, the **ANTLR tool jar**, and the **ANTLR C++
runtime** (headers + `libantlr4-runtime.a`).

```bash
./run.sh                       # solution grammar, default program
./run.sh starter               # starter grammar (missing '%'), default program
./run.sh solution "a = 6/2;"   # run your own program string
./run.sh clean                 # remove build/
```

ANTLR hands you a **homogeneous** parse tree: every
node is the same static type (`antlr4::tree::ParseTree`
/ `RuleContext`), told apart only by a runtime tag --
the exact pole Lesson 3 introduces. This example parses
a tiny assignment language into that tree, prints it,
and evaluates it with a generated visitor. The visitor
is where ANTLR's **labeled alternatives** (`# MulDiv`,
`# Var`) pay off: each one makes ANTLR emit a typed
context class (`MulDivContext`) and a typed visitor
method (`visitMulDiv`) -- a *normalized heterogeneous*
handle over the homogeneous tree.

## Prerequisites

Three things -- unlike ex05/07/08, this example needs
more than a compiler:

- a **JVM** (`java`) to run the ANTLR tool;
- the **ANTLR 4 tool jar** (e.g.
  `antlr-4.13.2-complete.jar` from
  <https://www.antlr.org/download.html>);
- the **ANTLR C++ runtime** -- headers
  `include/antlr4-runtime/` and a `libantlr4-runtime`
  (static or shared). Install your system's ANTLR4 C++
  runtime package, or build it from the ANTLR repo's
  `runtime/Cpp`.

`run.sh` discovers all three automatically from
standard locations (and, on the CMPUT 415 machines,
from the course ANTLR install as a fallback). If yours
live elsewhere, point it there:
`ANTLR_JAR=/path/to/antlr.jar ANTLR_INS=/path/to/prefix
./run.sh`. The script prints which jar and runtime it
used in its header, and gives install guidance if it
can't find them.

## How to run

```bash
./run.sh              # generate + compile + run the solution grammar
./run.sh starter      # same, on the starter grammar (the '%' is missing)
./run.sh solution "p = 3 * 4 - 1;"   # your own program as argv[2]
./run.sh clean        # delete build/
```

Under the hood `run.sh` runs `java -jar <jar>
-Dlanguage=Cpp -visitor -no-listener -package calc`
over `Expr.g4`, then compiles `main.cpp` plus the
generated `*.cpp` with `$CXX` (default `c++`) against
the runtime. First run does the codegen + compile; that
is all there is to it. To build with Clang instead:

```bash
CXX=clang++ ./run.sh          # your C++17 compiler via CXX
```

## What to look at

Run `./run.sh` and read the output next to two files:

- **The printed parse tree is homogeneous.** Every
  interior node prints as a generic `(expr ...)` or
  `(stat ...)` -- a multiply, an add, a parenthesised
  group, and a variable reference are all the *same*
  C++ type (`RuleContext`), distinguished only at
  runtime. That uniform `(expr (expr ...) op (expr
  ...))` shape *is* the homogeneous pole.
- **The typed handles are in `main.cpp`.** The `Eval`
  visitor subclasses the generated
  `calc::ExprBaseVisitor` and overrides `visitMulDiv`,
  `visitAddSub`, `visitParen`, `visitVar`, `visitInt`
  -- one per labeled alternative. Inside `visitMulDiv`
  it reads `ctx->expr(0)`, `ctx->expr(1)`, `ctx->op`
  with compile-time checking. Those `*Context` classes
  are the "normalized heterogeneous" middle ground:
  typed names layered over a homogeneous tree.

## The exercise

The starter's `MulDiv` alternative is missing the `%`
(modulo) operator. In `starter/Expr.g4` change

```antlr
expr op=('*'|'/') expr   # MulDiv
```

to

```antlr
expr op=('*'|'/'|'%') expr   # MulDiv
```

then rerun `./run.sh starter`. That is the whole fix --
a **grammar-only** change. You never touch `main.cpp`:
`%` routes to the *same* labeled alternative
(`MulDiv`), and `visitMulDiv` already computes `l % r`.
Extending an operator's token set does not add a node
type, so it costs nothing on the visitor side.

The completed grammar is `solution/Expr.g4`; `diff`
against it to check your work.

## See also

- **`WALKTHROUGH.md`** -- what ANTLR generated, reading
  the homogeneous tree, the one-token exercise
  explained, and how this ANTLR frontend and
  ParserByHand's hand-built AST are the two poles over
  the *same* language.
- The parent lesson `../../README.md` sec. 4 frames
  ANTLR as "the homogeneous frontend some of you will
  use."
- The real lab: `415-labs/ANTLR/demo-finished/`.

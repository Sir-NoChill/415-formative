# Lesson 2 — Synthesis: comparing, analyzing, and adapting

> **Prerequisite:** Lesson 1
> (`../lesson-01-foundations/`). This lesson assumes
> you can already define an X-macro and say, in one
> sentence, why TableGen is "an advanced X-macro
> system." Here you put those ideas to work: you'll
> compare the two head-to-head, read real constructs
> and predict their effect, and extend both a real
> X-macro and a real MLIR dialect yourself.

## Learning objectives

By the end of this lesson you will be able to:

1. **Compare and contrast the use of an X-macro with
   the TableGen infrastructure.**
2. **Analyze the effect of a given X-macro.**
3. **Analyze the effect of a given TableGen
   construct.**
4. **Adapt an existing X-macro template to your own
   custom repository.**
5. **Adapt an existing set of TableGen tools to your
   own custom project.**

## How to work through this lesson

| Example | Folder | Objectives it drives |
|---|---|---|
| **ex03** | `examples/ex03-tokenkinds-analysis/` | Analyze an X-macro (2), adapt an X-macro (4). Plain `g++`. |
| **ex04** | `examples/ex04-mini-dialect-tablegen/` | Analyze a TableGen construct (3), adapt TableGen tools (5). Real MLIR + `mlir-tblgen`. |

Objective 1 (compare & contrast) is developed in this
document and reinforced by doing both examples. As
before, each example has `starter/`, `solution/`, and a
`WALKTHROUGH.md`.

---

## 1. Compare and contrast (objective 1)

You've now seen the same colour list expressed both
ways (ex01 vs ex02). Let's make the comparison precise.
They share **one core idea**: *write a list once,
generate many artifacts from it, so the artifacts can
never drift apart.* Everything else differs.

### Where they're the same
- Both eliminate parallel-list duplication (the Lesson
  1 hygiene win).
- Both separate *the data* (the list) from *its uses*
  (the generated shapes).
- Both are resolved at build time — there's no runtime
  cost.

### Where they differ

| Dimension | X-macro | TableGen |
|---|---|---|
| **Engine** | the C preprocessor (text substitution) | a backend program compiled into `llvm-tblgen`/`mlir-tblgen` |
| **The data model** | untyped rows of tokens | typed records with named fields |
| **Computation** | none — only pastes text | `!add`, `!shl`, conditionals, etc. |
| **Inheritance / reuse** | none | `class` inheritance, mixins, defaults |
| **Validation** | none | type checks, constraints, predicates |
| **Introspection** | invisible after preprocessing | dumpable (`--print-records`, `--dump-json`) |
| **Number of outputs** | one per `#include` site you write | many, via different `-gen-*` backends |
| **Cost to adopt** | zero (it's just C) | a language + toolchain + build wiring to learn |
| **Debuggability** | poor (macro-expanded code) | poor at the boundary, but errors are typed |

### The honest engineering summary

- Use an **X-macro** when the list is **simple and
  flat**, consumed only by your own C/C++, and you
  don't want a build dependency. It's unbeatable for a
  token table or an error-code enum.
- Use **TableGen** when the data is **rich or
  structured**, **several tools** need it, or you need
  **computation/validation/inheritance** — and the
  project is big enough to justify the toolchain.

The Gazprea compiler in this repo makes *both* choices
on purpose. Its lexer tables are hand-written X-macros
(`gazc/include/Operators.def`, `Keywords.def`), while
its MLIR ops and AST hierarchy are TableGen
(`gazc/include/Gazprea/GazpreaOps.td`,
`gazc/include/AST/ASTNodes.td`). One file even
documents the trade-off in a comment:
`gazc/include/Sema/SemaChecks.def` says it stays an
X-macro *"so we avoid the TableGen dependency."* That
is objective 1 in a real engineer's own words.

---

## 2. Analyzing the effect of an X-macro (objective 2)

The core skill is simple to state and worth practising
until it's automatic:

> **To analyze an X-macro, mentally substitute the
> macro body into every row of the list, at each
> `#include` site.** Do it once per site, because each
> site defines the macros differently.

Watch for these when you read one:

- **Layered macros.** A `.def` often defines one macro
  *in terms of* another (`#define PUNCTUATOR(S, N)
  TOK(N)`). This lets a consumer that only knows the
  general macro still receive the specific rows, while
  a consumer that defines the specific macro handles
  them specially. Trace both paths.
- **Token-pasting `##`** builds identifiers (`tok_ ##
  Name` → `tok_plus`).
- **Stringize `#`** turns a token into a string
  (`#Name` → `"plus"`).
- **The same list, several `#include`s** — find *all*
  the include sites to know the full effect of one row.

> **➡ Do ex03 now.**
> `examples/ex03-tokenkinds-analysis/` is a lexer token
> table (mirroring the real *ParserByHand* lab and
> Clang's `TokenKinds.def`). You'll trace the *same*
> six-row list through four different expansions — an
> enum, two name switches, and the lexer's character
> switch — and confirm your analysis by running the
> program.

---

## 3. Analyzing the effect of a TableGen construct (objective 3)

Analyzing TableGen is different from analyzing an
X-macro, because you reason about *records and a
backend* rather than text substitution:

> **To analyze a TableGen construct, ask three
> questions:** (a) *What record does this `def`/`class`
> produce* — what are its fields and their values,
> including inherited and computed ones? (b) *Which
> backend consumes it* — which `-gen-*` generator,
> producing what kind of output? (c) *What C++ (or
> docs, or table) does that backend emit for this
> record?*

For MLIR operation definitions specifically, learn to
read these parts of a `def`:

- **Traits** in `[...]` (e.g. `Pure`,
  `SameOperandsAndResultType`) — they attach behaviour
  and, crucially, affect what the parser/printer need
  to spell out.
- **`arguments = (ins ...)` / `results = (outs ...)`**
  — the typed operands and results, in TableGen's
  parenthesised *dag* notation.
- **`assemblyFormat`** — a declarative grammar from
  which the backend generates *both* a textual parser
  and a printer.

The leap from Lesson 1: in ex02 the "backend" was a
10-line Python script printing colour lines. In real
MLIR the backend is `mlir-tblgen`, and it emits
**entire C++ classes** — builders, accessors,
verifiers, parsers, printers — one per `def`.

> **➡ Do ex04 now (part 1: analyze).**
> `examples/ex04-mini-dialect-tablegen/` is a small but
> real MLIR *dialect*. Read `Demo_AddOp` in
> `include/Demo/DemoOps.td` and predict what C++ it
> generates, then build it and confirm the op
> round-trips through the `demo-opt` tool.

---

## 4. Adapting an X-macro template (objective 4)

Adapting means: take an existing `.def`-driven system
and extend it for your own needs, *by editing the list,
not the consumers.* The test of whether a system is a
good X-macro is precisely that adding an item is a
one-line, one-file change.

The ex03 exercise makes this measurable. You add two
rows to a token table:

```tablegen
PUNCTUATOR('=', equal)
PUNCTUATOR('%', percent)
```

…and **four** derived constructs update themselves —
the `TokenKind` enum, two name switches, and the
lexer's character switch — while you never open a
single switch statement. When you can do that
confidently on the ex03 table, you can do it on Clang's
`TokenKinds.def` or Gazprea's `Operators.def`; they're
the same shape at larger scale.

> **➡ ex03 exercise:** add the two punctuators, rebuild
> *without touching `main.cpp`*, and watch `5%2=1`
> start lexing correctly. Full answer in ex03's
> `WALKTHROUGH.md`.

---

## 5. Adapting a set of TableGen tools (objective 5)

This is the headline skill of the lesson: take a
project already wired for TableGen and extend it — add
a record, let the generator regenerate the C++, and get
new working functionality with **no hand-written
boilerplate.**

In ex04 you add a whole new compiler operation,
`demo.mul`, by writing one `def`:

```tablegen
def Demo_MulOp : Demo_Op<"mul", [Pure, SameOperandsAndResultType]> {
  let arguments = (ins I32:$lhs, I32:$rhs);
  let results   = (outs I32:$result);
  let assemblyFormat = "$lhs `,` $rhs attr-dict `:` type($result)";
}
```

Rebuild, and `demo-opt test/mul.mlir` round-trips. What
you did *not* write is the point: no parser, no
printer, no verifier, no builder, and not even the
registration line — `mlir-tblgen` generated the classes
and the `GET_OP_LIST` that registers them. (And note:
that registration uses the exact `#define X` /
`#include "...inc"` X-macro idiom from Lesson 1 to
consume TableGen's *own* output — the two techniques
compose.)

This is exactly how the `gaz` dialect in the Gazprea
compiler grows: someone adds a `def` to
`gazc/include/Gazprea/GazpreaOps.td` and rebuilds.
Gazprea goes one step further and ships a *custom*
TableGen tool, `gazprea-tblgen`, with hand-written
backends that generate its AST visitor and traversal
from `gazc/include/AST/ASTNodes.td`. That's the natural
sequel to this lesson: once you see TableGen as "a data
model plus a code generator," you can write your own
backend, not just consume LLVM's.

> **➡ ex04 exercise:** add `Demo_MulOp`, rebuild via
> `./run.sh`, confirm `test/mul.mlir` round-trips. Full
> answer in `solution/DemoOps.td` and ex04's
> `WALKTHROUGH.md`.

---

## Check yourself

You've met the objectives if you can:

1. Give three concrete differences between an X-macro
   and TableGen, and state a situation where each is
   the *right* choice.
2. Take an unfamiliar `.def` file and describe what
   each `#include` site generates.
3. Take an unfamiliar MLIR `def` and describe the C++
   it will generate and which backend generates it.
4. Add an item to an X-macro-driven system by editing
   only the list.
5. Add an operation to a TableGen-driven dialect and
   explain everything the generator did on your behalf.

### Where to go next
- Read the real thing:
  `gazc/include/Gazprea/GazpreaOps.td` and
  `gazc/include/AST/ASTNodes.td`, then find their
  generated `.inc` files in the build tree.
- Study a custom backend: `gazc/tools/gazprea-tblgen/`
  — the emitters that turn `ASTNodes.td` into a visitor
  and a traversal.
- Browse LLVM's shipped backends in
  `LLVM/llvm-project/mlir/tools/mlir-tblgen/` and
  `LLVM/llvm-project/llvm/utils/TableGen/`.

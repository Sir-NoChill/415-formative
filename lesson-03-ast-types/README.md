# Lesson 3 -- Types of ASTs

> **Prerequisite:** Lessons 1-2
> (`../lesson-01-foundations/`,
> `../lesson-02-synthesis/`). Those lessons taught you
> to generate boilerplate from a single list -- an
> X-macro or a TableGen record set. This lesson is
> about the data structure that list most often
> describes: the **abstract syntax tree**. In fact the
> connection is literal. ParserByHand's AST is
> generated from an X-macro, `ASTNodes.def`; Gazprea's
> is generated from TableGen,
> `gazc/include/AST/ASTNodes.td`. The "list -> many
> artifacts" idea you just learned is exactly how a
> real AST's enum, visitor, and RTTI are kept in sync.

## Learning objectives

By the end of this lesson you will be able to:

1. **Define a homogeneous and a heterogeneous AST** (in
   terms of typing).
2. **Identify the key functionality an AST must have**
   for effective analysis and traversal.
3. **Outline a set of passes** normally provided by a
   compiler as walks over the AST.
4. **Compare and contrast** the benefits of a
   heterogeneous vs. homogeneous AST for analysis and
   code transformation.

## How to work through this lesson

| Example | Folder | Objectives it drives |
|---|---|---|
| **ex05** | `examples/ex05-homo-vs-hetero/` | Define homo/hetero (1), passes as walks (3), key functionality (2), compare & contrast (4). Plain `g++`. |
| **ex06** | `examples/ex06-antlr-frontend/` | The homogeneous parse tree in a *real* frontend -- because some students build their parser with **ANTLR** (1, 4). Java + `antlr4` + `g++`. |

Objectives 2 and 3 are developed in this document and
reinforced by the passes in ex05. As before, each
example ships `starter/`, `solution/`, and a
`WALKTHROUGH.md`.

The examples here are self-contained (a C++17 compiler;
plus an ANTLR install for ex06). Where this lesson says
"the real thing," it points at production code for
reference -- the CMPUT 415 **ParserByHand** lab
(`415-labs/ParserByHand/`), the **ANTLR** lab
(`415-labs/ANTLR/`), and the **Gazprea** compiler
(`gazc/`). Those live on the course machines; you are
**not** required to have them to do the exercises --
they're there to read when you do.

---

## 1. Homogeneous vs. heterogeneous (objective 1)

An AST design answers one foundational question: **how
many static types does a node have?**

- A **homogeneous AST** uses *one* node type for the
  whole language. A node is a generic record with a
  runtime **kind** tag, some payload, and a uniform
  list of children. Lisp s-expressions and
  [ANTLR](https://www.antlr.org/)'s parse tree are
  homogeneous: an addition, an `if`, and a call are all
  the *same* C++ type, told apart only by a tag you
  check at runtime.
- A **heterogeneous AST** gives each construct its
  *own* static type -- a class `BinaryExpr`, a class
  `IdExpr`, a class `Assignment` -- usually sharing a
  base. Each class has exactly the fields that
  construct needs, with real names and types (`lhs`,
  `rhs`, `cond`), not a nameless `kids[2]`. This is the
  [Clang/LLVM](https://clang.llvm.org/docs/IntroductionToTheClangAST.html)
  style, and the style of ParserByHand and Gazprea.

> **-> Do ex05 now.** `examples/ex05-homo-vs-hetero/`
> builds the *same* three assignments -- including `y =
> 2 * (x + 1)` -- **twice**, once each way, and runs
> the same passes over both. Reading the two halves
> side by side is the fastest way to feel the
> difference.

### The three-way refinement you'll actually meet

This taxonomy is [Terence
Parr](https://www.antlr.org/)'s, from his book
[*Language Implementation
Patterns*](https://pragprog.com/titles/tpdsl/language-implementation-patterns/)
(Pragmatic Bookshelf, 2010); Parr is also the author of
ANTLR. He splits the space into three:

- **Homogeneous** -- one node type.
- **Normalized heterogeneous** -- many node types, but
  a *uniform* child list on all of them, so one generic
  traversal still works.
- **Irregular heterogeneous** -- many node types
  **and** irregular, *named* children
  (`cond`/`then`/`else`). ParserByHand and Gazprea are
  here.

ANTLR sits interestingly in the middle (see sec. 4):
its tree is homogeneous, but its *labeled alternatives*
generate typed context classes that feel normalized
heterogeneous.

---

## 2. What every AST must provide (objective 2)

Whatever the typing, an AST earns its keep only if it
supports what analyses and transformations need. Treat
this as a checklist. The right-hand column names where
each item lives in the ParserByHand lab -- a compact,
real implementation of every one -- for those who have
it; ex05 and ex07 in *this* pack show the same
capabilities in self-contained form.

| Capability | Why it matters | Where to see it in ParserByHand |
|---|---|---|
| **Kind discrimination** | A pass must ask "what is this node?" cheaply and safely. | `include/Config/AST.h` -- a hand-rolled `isa<>`/`cast<>`/`dyn_cast<>` over a `Kind` enum, exactly LLVM's idiom. |
| **Child navigation** | Traversal is *the* core operation. | Named accessors (`getLHS()`) plus the generated visitor. |
| **Source locations** | Diagnostics must point at real code. | `SourceRange` on every `ASTNode` (`include/Config/SourceLocation.h`). |
| **Annotation slots** | Passes *decorate* the tree with resolved types/symbols. | (see Gazprea's richer nodes; ex07 adds a `type` slot) |
| **Construction** | Nodes are made in bulk and must be cheap to own. | Nodes owned via `unique_ptr`; Clang/Gazprea use an `ASTContext` arena. |
| **Visitability** | Add new operations without editing every node. | `include/Config/ASTVisitor.h` -- one `visitX` per node, **generated from `ASTNodes.def`**. |

That last row is the payoff from Lessons 1-2 made
concrete. In ParserByHand,
`include/Config/ASTNodes.def` is a six-row X-macro
`#include`d to generate the `ASTNodeKind` enum, the
`isExprKind()` range predicate, *and* every `visitX`
method -- add one row and all three grow together.
Gazprea does the same from `ASTNodes.td` via its custom
`gazprea-tblgen` backend. You don't need either repo to
see this: **ex07 in Lesson 4 does exactly this
in-pack** (`mini_nodes.def` generates its `Kind` enum,
range predicate, and a name table). **An AST is the
canonical X-macro / TableGen client.**

### LLVM-style RTTI, the idiom to recognize

([Full details in the LLVM
docs](https://llvm.org/docs/HowToSetUpLLVMStyleRTTI.html).)
LLVM does not use C++ `dynamic_cast`. Every node
carries a `Kind` enum and each class defines a static
`classof`; `isa<T>`, `cast<T>`, and `dyn_cast<T>` build
on it. Order the enum so a category's members are
*contiguous* and `classof` becomes an O(1) range check
-- which is exactly why `ASTNodes.def` keeps its
expression nodes last and declares
`AST_NODE_RANGE(Expr, BinaryExpr, NumberExpr)`. ex05
and ex07 use this same idiom in miniature.

---

## 3. The passes: a compiler as a sequence of tree walks (objective 3)

Most of a front end is a series of **passes**, each a
walk over the AST that reads it and either records
information or rewrites it:

| # | Pass | What it does | Reads / Writes |
|---|---|---|---|
| 1 | **Name / scope resolution** | Bind each identifier use to its declaration. | reads names -> writes `Symbol*` |
| 2 | **Type checking** | Infer/check the type of every expression. | reads structure -> writes `Type*` |
| 3 | **Constant folding** | Evaluate constant sub-trees (`4 + 5 -> 9`). | rewrites nodes |
| 4 | **Desugaring / normalization** | Rewrite rich syntax into a small core (`for -> while`). | rewrites nodes |
| 5 | **Flow checks** | Reachability, definite assignment, unused variables. | reads -> diagnostics |
| 6 | **Lowering / codegen** | Emit IR (e.g. LLVM IR / an MLIR dialect). | reads -> emits IR |
| 7 | **Pretty-print / unparse** | Turn the tree back into text (formatters, errors). | reads -> text |

Two things to carry into Lesson 4. First, passes
**compose by decoration**: type checking needs the
`Symbol*` that resolution wrote, so pass order is a
dependency, not a preference. Second, heavy
optimization is deliberately *absent* -- it lives on
the IR, not the AST. The AST's job is to be faithful to
the source and easy to check.

ex05 runs three of these as explicit walks
(pretty-print, constant-fold, evaluate). For the real
thing, read ParserByHand's `lib/Sema.cpp` (a
name-resolution pass) and `lib/ASTPrinter.cpp` (an
unparse pass built on the generated visitor).

---

## 4. Compare and contrast (objective 4)

| Concern | Homogeneous | Heterogeneous |
|---|---|---|
| Type safety | None at compile time; a wrong field read is a runtime bug. | The compiler enforces that a `BinaryExpr` has an `lhs`. |
| Generic traversal | Trivial -- one walker via `kids[]`. | Needs a visitor / generated iterator. |
| Writing a new **pass** | Easy: one function switching on the tag. | Add one visitor subclass; nodes untouched. |
| Adding a new **node type** | Cheap: a new tag value. | Costly: a new class *and* a method in every visitor. |
| Self-documentation / IDE | Poor; you must know the tag + arity conventions. | Excellent; fields are named and typed. |
| Where errors surface | Late, at runtime. | Early, at compile time. |

This is the [**expression
problem**](https://homepages.inf.ed.ac.uk/wadler/papers/expression/expression.txt)
(a term Philip Wadler named in 1998): you have two axes
of change -- new *node types* and new *passes* -- and
no single representation makes both free. A homogeneous
AST makes new node types trivial but taxes every pass;
a heterogeneous AST makes new passes clean but ripples
a new node type through every visitor. Pick the
representation whose *cheap* axis matches how your
project changes. (Lesson 1's X-macro flips exactly this
trade-off in your favor for the homogeneous case:
adding a node = adding a row.)

### ANTLR: the homogeneous frontend some of you will use

If you build your parser with ANTLR, you inherit a
**homogeneous** parse tree of `antlr4::tree::ParseTree`
/ `RuleContext` nodes. But ANTLR's **labeled
alternatives** (`# MulDiv`, `# Var`) generate typed
context classes (`MulDivContext`) and typed visitor
methods (`visitMulDiv`) -- a *normalized heterogeneous*
handle over a homogeneous tree. Many real compilers
then run one more pass -- a visitor that **builds a
heterogeneous AST** from the parse tree -- which is
precisely the bridge from an ANTLR frontend to the
hand-built ASTs of Lesson 4.

> **-> Do ex06 now.** `examples/ex06-antlr-frontend/`
> parses the same `2 * (x + 1)` with a real ANTLR
> grammar, prints the homogeneous parse tree, and
> evaluates it through a generated visitor. It mirrors
> the CMPUT 415 ANTLR lab
> (`415-labs/ANTLR/demo-finished/`), which parses the
> same "Config" language that the ParserByHand lab
> parses into a *heterogeneous* AST -- the same
> language, both poles, side by side.

### The reconciliation: MLIR / the `gaz` dialect

Real infrastructure blends the poles.
[MLIR](https://mlir.llvm.org/docs/LangRef/) (which the
`gaz` dialect in Gazprea is built on -- see Lesson 2's
ex04) represents everything as a uniform `Operation`
(homogeneous storage: generic operands, results,
attributes, regions) but layers *typed* Op wrappers
(via its [Operation Definition
Specification](https://mlir.llvm.org/docs/DefiningDialects/Operations/))
and *interfaces* on top, so passes get heterogeneous
ergonomics and generic traversal at once. Neither pole
is "correct"; each optimizes a different cheap axis.

---

## Check yourself

You've met the objectives if you can:

1. Define homogeneous and heterogeneous ASTs and give
   one real example of each on this machine.
2. Name four things any AST must provide, and point at
   where ParserByHand implements each.
3. List the passes a front end runs over the AST and
   say which two have a hard ordering dependency (and
   what data flows between them).
4. State the expression problem and say which
   representation you'd choose for a project that adds
   *passes* often vs. one that adds *node types* often.

### Where to go next
- Read the real heterogeneous AST:
  `415-labs/ParserByHand/demo-finished/include/Config/`
  (`AST.h`, `ASTNodes.def`, `ASTVisitor.h`,
  `Sema.cpp`).
- Read the real homogeneous frontend:
  `415-labs/ANTLR/demo-finished/` (`grammar/Config.g4`,
  `src/ConfigEvaluator.cpp`).
- Read the production AST:
  `gazc/include/AST/ASTNodes.td` and its
  `gazprea-tblgen` backend -- the TableGen version of
  everything above.
- Then continue to **Lesson 4**, where you design,
  decorate, and extend an AST yourself.

## References

- Terence Parr, *Language Implementation Patterns* (the
  homo / normalized-hetero / irregular-hetero taxonomy)
  --
  <https://pragprog.com/titles/tpdsl/language-implementation-patterns/>
- ANTLR (homogeneous parse tree; labeled alternatives)
  -- <https://www.antlr.org/>
- LLVM-style RTTI (`isa`/`cast`/`dyn_cast`/`classof`)
  --
  <https://llvm.org/docs/HowToSetUpLLVMStyleRTTI.html>
- Clang AST, `ASTContext`, `RecursiveASTVisitor` --
  <https://clang.llvm.org/docs/IntroductionToTheClangAST.html>,
  <https://clang.llvm.org/docs/RAVFrontendAction.html>
- MLIR -- Language Reference (uniform `Operation`) and
  Operation Definition Spec --
  <https://mlir.llvm.org/docs/LangRef/>, <https://mlir.llvm.org/docs/DefiningDialects/Operations/>
- The expression problem (Wadler, 1998) --
  <https://homepages.inf.ed.ac.uk/wadler/papers/expression/expression.txt>

# Lesson 4 -- Developing ASTs

> **Prerequisite:** Lesson 3
> (`../lesson-03-ast-types/`). You can already define
> homogeneous vs. heterogeneous, name what an AST must
> provide, and outline the passes that walk it. Here
> you put those ideas to work: you'll critique a real
> design, build a heterogeneous AST from scratch, write
> a semantic pass over it, and extend an AST with a new
> construct -- the same four moves you'll make on any
> real compiler front end.

## Learning objectives

By the end of this lesson you will be able to:

1. **Analyze an existing AST** for potential design
   drawbacks.
2. **Design an AST from scratch** for a custom, small
   language.
3. **Implement a semantic analysis pass** using an AST
   for simple analyses.
4. **Adapt an existing AST** to add a new feature or
   language construct.

## How to work through this lesson

| Example | Folder | Objectives it drives |
|---|---|---|
| **ex07** | `examples/ex07-mini-ast/` | Analyze a naive AST (1), design the "Mini" AST (2), implement name-resolution + type checking (3). Plain `g++`. |
| **ex08** | `examples/ex08-adapt-add-for/` | Adapt the Mini AST to add a `for` loop, two ways (4) -- then do it on *real* code. Plain `g++`. |

As before, each example ships `starter/`, `solution/`,
and a `WALKTHROUGH.md`.

---

## 1. Analyzing an existing AST for drawbacks (objective 1)

The first skill is learning to *smell* a bad design.
Here is a first attempt that works on a calculator and
collapses the moment the language grows:

```cpp
struct Ast {
  std::string type;    // "add", "num", "var", "if"...
  std::string value;   // number text OR name OR operator
  Ast *left;
  Ast *right;          // only ever two children?
};
```

Read it critically. The drawbacks, worst first:

- **Stringly-typed kind.** `type == "add"` makes every
  typo a silent runtime bug and every dispatch a string
  compare. (Lesson 3: use a `Kind` enum + RTTI.)
- **Fixed binary shape.** `left`/`right` can't hold a
  call with N arguments, an `if` with three parts, or a
  block of statements. The shape lies about the
  language.
- **Overloaded payload.** `value` means a number, a
  name, or an operator depending on `type`; nothing
  enforces the correspondence.
- **No source locations.** You can never point a
  diagnostic at the code. Retrofitting locations later
  touches every construction site.
- **No expr/stmt/decl distinction.** Nothing stops a
  `while` where a number belongs.
- **No annotation slots.** Where do resolved types and
  symbols go? Nowhere -- so passes grow fragile
  external maps.
- **Unclear ownership.** Raw `new` leaks; shared
  children invite accidental aliasing.

> **Heuristic.** Audit any AST with four questions:
> *Can it represent every arity my grammar needs? Can a
> wrong field access fail to **compile** rather than at
> runtime? Where do passes hang their results? Who owns
> and frees a node?*

> **-> Start ex07 now (part 1).**
> `examples/ex07-mini-ast/` puts a well-designed AST
> beside this naive one. Contrast the two before you
> read part 2. (ParserByHand's `include/Config/AST.h`
> is a production example of every fix, if you have the
> lab -- but ex07 is self-contained and needs nothing
> external.)

---

## 2. Designing an AST from scratch (objective 2)

Design for a small imperative language, **Mini**.

> **Where "Mini" comes from.** Mini is a small language
> **invented for this lesson pack** -- it is not a
> standard or historical language, and there is no
> external spec to read. It is a deliberately tiny
> imperative core (declaration, assignment,
> `if`/`while`, `print`, over `int`/`bool` expressions)
> chosen so that a complete AST *and* a semantic pass
> fit in one readable file. It stands in the long
> tradition of small teaching languages built to make
> compiler ideas concrete -- LLVM's
> [Kaleidoscope](https://llvm.org/docs/tutorial/), Lox
> from [*Crafting
> Interpreters*](https://craftinginterpreters.com/),
> and the example languages of the [Dragon
> Book](https://en.wikipedia.org/wiki/Compilers:_Principles,_Techniques,_and_Tools).
> Nothing in these lessons depends on Mini
> specifically; the techniques transfer to any AST.

You can't design nodes for constructs you haven't
named, so pin the grammar first:

```
program := stmt*
stmt    := "let" IDENT "=" expr ";"        // declaration
         | IDENT "=" expr ";"              // assignment
         | "if" "(" expr ")" block ("else" block)?
         | "while" "(" expr ")" block
         | "print" expr ";"
block   := "{" stmt* "}"
expr    := expr ("+"|"-"|"*"|"/"|"<"|"==") expr
         | "(" expr ")" | INT | IDENT
```

The design decisions that make the later passes small
-- a shared base with a `Kind` tag (expressions kept
contiguous for range-check RTTI), an `Expr`/`Stmt`
split so the type system rejects nonsense, a
`SourceLoc` on every node, named and typed children, a
`type` annotation slot, and an arena for ownership --
are all called out `(1)..(5)` in
`ex07-mini-ast/solution/main.cpp`. They are the same
decisions ParserByHand and Gazprea make; ex07 just
shows them in one readable file. And decision `(1)` is
where Lessons 1-2 come back: the `Kind` enum, the
`isExprKind()` range predicate, and a `kindName()`
string are all generated from one **X-macro** table,
`mini_nodes.def` -- so adding a node is one row,
exactly as in ex03.

> **-> ex07 (part 2): read the design.** Match each
> `(N)` callout in the solution back to a drawback from
> sec. 1.

---

## 3. Implementing a semantic analysis pass (objective 3)

A semantic pass **decorates** and **checks** the tree.
ex07 does two classic analyses in one walk over Mini:

- **Name resolution** -- a stack of scopes; each name
  resolves to a declaration. Reports
  use-before-declaration and redeclaration.
- **Type checking** -- arithmetic needs `int` operands;
  an `if`/`while` condition needs `bool`. The inferred
  type is written back into each expression's `type`
  slot for later passes.

The engine is a scoped symbol table plus a recursive
`checkStmt`/`checkExpr`. The subtle-but-correct
decisions are exactly what the tree lets you get right:
a `let`'s name enters scope *after* its initializer is
checked; every diagnostic carries the node's
`SourceLoc`, so it prints as `line:col: error: ...`.

> **-> ex07 exercise.** In `starter/main.cpp` the
> name-resolution and condition-type checks are
> stubbed. Implement them and watch the buggy test
> program go from **1** reported error to **3**, each
> with a location. Full answer in ex07's
> `WALKTHROUGH.md`. Then compare with the real pass in
> ParserByHand's `lib/Sema.cpp`.

---

## 4. Adapting an AST to add a construct (objective 4)

Requirements change. ex08 adds a C-style `for` loop to
Mini, and choosing *how* is the lesson. There are two
honest strategies:

- **A -- a new node type.** Add a `For` class. New
  power in the tree, but every pass that walks
  statements grows a `For` case (the interpreter here;
  a type checker, pretty-printer, and lowering in a
  real compiler). This is the expression-problem tax
  from Lesson 3, made concrete.
- **B -- desugar into existing nodes.** `for (init;
  cond; step) body` becomes `{ init; while (cond) {
  body; step; } }`. *Zero* passes change -- but the
  tree no longer records that the user wrote a `for`,
  so `for`-specific diagnostics become impossible.

> **Rule of thumb.** Add a **node type** when the
> construct carries meaning later passes must see or
> report on; **desugar** when it's pure convenience
> over an existing core. Every node you add is a tax on
> every pass forever; every desugaring is a small loss
> of source fidelity. This is why [Clang's
> AST](https://clang.llvm.org/docs/IntroductionToTheClangAST.html)
> keeps rich sugar nodes (it "closely resembles ... the
> written C++ code") and desugars *late*, while a
> teaching compiler often desugars early to keep its
> core tiny.

> **-> ex08 exercise.** The starter has strategy A
> working and asks you to implement the desugaring
> (strategy B). Before: strategy B prints `sum = 0`;
> after: `sum = 15`, matching strategy A.

### Now do it on real code

The same "adapt" skill, at production scale, is a
*one-line* change thanks to Lessons 1-2:

- **ParserByHand (X-macro AST).** To add a node, add
  one `AST_NODE(...)` row to
  `415-labs/ParserByHand/demo-finished/include/Config/ASTNodes.def`
  (keeping the category range contiguous). The enum,
  the `visitX` method, and the RTTI predicate all
  regenerate -- then you write the node's fields in
  `AST.h` and one `case` per pass that cares.
- **Gazprea (TableGen AST).** Add a `def` to
  `gazc/include/AST/ASTNodes.td` and rebuild;
  `gazprea-tblgen` regenerates the visitor and
  traversal. This is the natural sequel to Lesson 2's
  ex04: there you added an *operation* to a dialect;
  here you add a *node* to an AST, the same way.

---

## Check yourself

You've met the objectives if you can:

1. Take an unfamiliar AST and list three concrete
   design drawbacks.
2. Design a node hierarchy for a small language,
   justifying the base classes, the RTTI scheme, and
   where locations and annotations live.
3. Write a scoped, location-aware semantic pass that
   both checks and decorates the tree.
4. Add a construct to an AST two ways, and argue which
   is right for a given goal.

### Where to go next
- **Self-contained:** extend ex07's Mini AST -- add a
  unary-negation expression end to end: one
  `MINI_NODE(Neg)` row in `mini_nodes.def`, a `Neg :
  Expr` class, a `checkExpr` case, and (borrowing
  ex08's interpreter) an `eval` case. Everything you
  need is in this pack.
- *If you have the course repos (reference):* do the
  same on the real ParserByHand AST (`.def` row ->
  `AST.h` class -> parser -> `Sema.cpp` ->
  `ASTPrinter.cpp`), and study Gazprea's
  `gazprea-tblgen` backends
  (`gazc/tools/gazprea-tblgen/`) that turn
  `ASTNodes.td` into a visitor and traversal.
- Read Clang's AST hierarchy
  (`clang/include/clang/AST/`, open source) and its
  `RecursiveASTVisitor` -- the industrial version of
  ex07's visitor.

## References

- Clang AST & `ASTContext` --
  <https://clang.llvm.org/docs/IntroductionToTheClangAST.html>
- Clang `RecursiveASTVisitor` --
  <https://clang.llvm.org/docs/RAVFrontendAction.html>
- LLVM-style RTTI (`isa`/`cast`/`dyn_cast`/`classof`)
  --
  <https://llvm.org/docs/HowToSetUpLLVMStyleRTTI.html>
- The expression problem (Wadler, 1998) --
  <https://homepages.inf.ed.ac.uk/wadler/papers/expression/expression.txt>
- Small teaching languages: LLVM Kaleidoscope --
  <https://llvm.org/docs/tutorial/> - *Crafting
  Interpreters* (Lox) --
  <https://craftinginterpreters.com/>
- Aho, Lam, Sethi & Ullman, *Compilers: Principles,
  Techniques & Tools* (Dragon Book) --
  <https://en.wikipedia.org/wiki/Compilers:_Principles,_Techniques,_and_Tools>
- X-macros (the table idiom `mini_nodes.def` uses) --
  see Lesson 1, and
  <https://en.wikipedia.org/wiki/X_macro>

# ex05 — Homogeneous vs. heterogeneous ASTs

**Lesson 3.** Build the *same* program two ways; run
the *same* passes over both. No dependencies (plain
C++17).

```bash
g++ -std=c++17 -Wall -Wextra solution/main.cpp -o ex05 && ./ex05
g++ -std=c++17 -Wall -Wextra starter/main.cpp  -o ex05 && ./ex05   # before the task
```

(Swap `g++` for `clang++` if you prefer — both are
verified clean under `-Wall -Wextra`.)

One `main.cpp` builds the three assignments `x = 5; z =
3 * (4 + 5); y = 2 * (x + 1)` in **two**
representations and runs **three** passes over each.

- `solution/` — both `fold` passes implemented.
- `starter/`  — same file; both `fold` passes stubbed
  as no-ops (your exercise).
- **`WALKTHROUGH.md`** — the two trees side by side,
  each pass as a walk, and the fold answer.

## 1. What this example teaches

This is a Lesson 3 example. It drives all four
objectives at once:

- **Define homogeneous and heterogeneous (objective
  1).** `namespace homo` uses *one* node type
  (`Node{kind, ival, name, kids}`) with a runtime
  `Kind` tag and a generic child list. `namespace
  hetero` gives each construct its own class (`IntLit`,
  `VarRef`, `Binary`, `Assign`) sharing a `Node` base.
  Same program, two typings.
- **Identify the key functionality an AST must have
  (objective 2).** Watch three capabilities appear in
  both halves: **kind discrimination** (a `switch` on
  the tag vs. `isa`/`dyn_cast`), **child navigation**
  (`kids[0]`/`kids[1]` vs. named `lhs`/`rhs`/`value`),
  and **construction** (a `unique_ptr` `Arena` in
  each).
- **Outline passes as walks (objective 3).** `print`,
  `fold`, and `eval` are each a recursive walk over the
  tree — a read-only unparse, a rewrite, and an
  evaluation.
- **Compare and contrast + the expression problem
  (objective 4).** The two halves are line-for-line
  comparable, so the trade-off is visible, not
  asserted.

## 2. The two representations, in one file

`homo::Node` is a single struct. `kind` says what it
is; the payload is *overloaded* (`ival` valid only for
`K_Int`, `name` valid only for `K_Var`/ `K_Assign`);
`kids` is a flat `vector<Node*>` whose length is a
**convention** per kind (`Assign` → `[value]`, a binop
→ `[lhs, rhs]`). Nothing in the type enforces that
convention.

`hetero` splits the same information across classes.
`Binary` has real `lhs`/`rhs` fields of type `Expr*`;
`Assign` has a `name` and a `value`. Kind
discrimination is [LLVM-style **hand-rolled
RTTI**](https://llvm.org/docs/HowToSetUpLLVMStyleRTTI.html):
each class defines `static bool classof(Node*)`, and
the free templates `isa<T>`, `dyn_cast<T>`, `cast<T>`
build on it (the exact idiom from ParserByHand's
`AST.h`, in miniature).

## 3. What to look at in the code

Read the two halves side by side, one pass at a time:

- **`print`** — `homo` does `switch (n->kind)` then
  indexes `n->kids[0]`/`kids[1]`; `hetero` chains
  `dyn_cast<Assign>` / `dyn_cast<Binary>` / … then
  reads `a->value`, `b->lhs`, `b->rhs` by name.
- **`fold`** — the exercise (see below).
- **`eval`** — same shape again: a tag `switch` vs. a
  `dyn_cast` chain.
- **`buildProgram`** — how each tree is constructed.
  `homo` uses `num`/`var`/`bin`/ `asn` lambdas over one
  `make`; `hetero` uses a variadic
  `Arena::make<T>(...)`.

The point is not that one is shorter. It is **where the
risk lives**: the homogeneous walks reach children by
index, so a wrong index (`kids[1]` where you meant
`kids[0]`) still compiles and fails at runtime; the
heterogeneous walks reach children by field name, so a
wrong field name (`b->rhsg`) will not compile.

## 4. Your exercise

Switch to `starter/`. Both `fold` functions are stubbed
to return their argument unchanged. Everything else is
complete. Build and run it first to see the "before":

```bash
g++ -std=c++17 -Wall -Wextra starter/main.cpp -o ex05 && ./ex05
```

Verified "before" output — note that `z` prints
**unfolded** because `fold` is a no-op, yet still
evaluates to `27` (the folding pass is an optimization
of the tree, not of the answer):

```
--- Homogeneous (one Node type, runtime tag) ---
  (= x 5)   ==> x = 5
  (= z (* 3 (+ 4 5)))   ==> z = 27
  (= y (* 2 (+ x 1)))   ==> y = 12

--- Heterogeneous (typed class per construct + RTTI) ---
  (= x 5)   ==> x = 5
  (= z (* 3 (+ 4 5)))   ==> z = 27
  (= y (* 2 (+ x 1)))   ==> y = 12

Same program, same passes, identical results -- only the typing of the tree differs.
```

**Task:** implement both `fold` functions so a binop of
two constant operands collapses to a single constant.
When done, rebuild and re-run; `z` should print folded
as `(= z 27)` in both halves, matching the verified
solution output. See `WALKTHROUGH.md` for the
fully-explained answer.

As you write the two versions, notice the difference
the task is designed to surface. The homogeneous `fold`
must `switch` on the tag and reach operands by index
(`n->kids[0]`, `n->kids[1]`) — get an index wrong and
it *still compiles*. The heterogeneous `fold`
`dyn_cast`s to `Binary` and reaches operands by name
(`b->lhs`, `b->rhs`) — get a name wrong and it *will
not compile*. That contrast is the whole lesson in your
fingers.

## 5. Cross-reference to a real compiler

You have now built both poles in miniature. The
production versions (reference only — you don't need
them to do this example) are:

- **ParserByHand**
  (`../../../../415-labs/ParserByHand/demo-finished/include/Config/AST.h`)
  — a real heterogeneous AST with the same hand-rolled
  `isa`/`cast`/`dyn_cast` RTTI that `hetero` uses here,
  driven by an `ASTNodes.def` X-macro.
- **ANTLR**
  (`../../../../415-labs/ANTLR/demo-finished/`) — a
  real *homogeneous* parse tree, the production cousin
  of `homo`. That is **ex06**.
- **Gazprea** (this repo's compiler) —
  `gazc/include/AST/ASTNodes.td` generates the
  heterogeneous AST's enum, RTTI, and visitor from
  TableGen.

Notice one thing this example deliberately does *by
hand*: the `Kind` enum and the per-class `classof`
boilerplate. That is exactly the repetition an
**X-macro** removes — and **ex07** does remove it,
generating its `Kind` enum, category predicate, and a
name table from a single `mini_nodes.def` (the Lessons
1–2 idiom). So ex05 shows the shape; ex07 shows how a
real AST stops hand-writing it.

Continue to **ex06** to see the homogeneous parse tree
of a real ANTLR frontend — the same `2 * (x + 1)`,
parsed and walked for real.

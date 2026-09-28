# ex05 — Walkthrough: the same tree, two typings

This narrates `solution/main.cpp`. It builds `x = 5; z = 3 * (4 + 5); y = 2 * (x + 1)`
twice and runs three passes over each. Read it with the file open.

## 1. The two trees, side by side

For `z = 3 * (4 + 5)` the shape is identical; only the *typing* differs.

**Homogeneous** (`namespace homo`): every node is the same struct.

```cpp
struct Node {
  Kind kind;                 // K_Assign, K_Int, K_Var, K_Add, ...
  long ival = 0;             // valid when kind == K_Int
  std::string name;          // valid when kind == K_Var or K_Assign
  std::vector<Node *> kids;  // Assign: [value]; binops: [lhs, rhs]
};
```

An `Assign(z)` node holds `name = "z"` and `kids = [ Mul ]`; the `Mul` holds
`kids = [ Int(3), Add ]`; the `Add` holds `kids = [ Int(4), Int(5) ]`. Which
payload field is live, and how long `kids` should be, is a **convention keyed on
`kind`** — the type permits any combination.

**Heterogeneous** (`namespace hetero`): each construct is its own class.

```cpp
struct Binary : Expr { char op; Expr *lhs; Expr *rhs; ... };
struct Assign : Node { std::string name; Expr *value; ... };
struct IntLit : Expr { long value; ... };
struct VarRef : Expr { std::string name; ... };
```

The same `z` is `Assign{name:"z", value: Binary{'*', IntLit{3}, Binary{'+', IntLit{4}, IntLit{5}}}}`.
Every child has a name and a static type; there is no `kids[]` and no "which field
is live" question — a `Binary` simply *has* an `op`, an `lhs`, and an `rhs`.

Kind discrimination in `hetero` is the LLVM idiom, hand-rolled:

```cpp
template <class To> bool  isa(Node *n)      { return To::classof(n); }
template <class To> To   *dyn_cast(Node *n) { return isa<To>(n) ? static_cast<To*>(n) : nullptr; }
// ... and each class: static bool classof(Node *n) { return n->kind == Kind::Binary; }
```

## 2. Each pass is a walk

All three passes are recursive walks; the two halves differ only in how they ask
"what node is this?" and "give me the children."

- **`print`** (read-only unparse). `homo` does `switch (n->kind)` and recurses into
  `n->kids[0]` / `n->kids[1]`. `hetero` chains `dyn_cast<Assign>` /
  `dyn_cast<Binary>` / `dyn_cast<IntLit>` / `dyn_cast<VarRef>` and recurses into the
  *named* `a->value`, `b->lhs`, `b->rhs`. Same s-expression out either way.
- **`fold`** (rewrite). Bottom-up: fold the children, and if both came back
  constant, replace the node with a fresh constant. Detailed below.
- **`eval`** (evaluate). Same two shapes again — a tag `switch` in `homo`, a
  `dyn_cast` chain in `hetero` — reading each `K_Var`/`VarRef` out of the `env` map.

`run()` glues them per statement: fold the value, `eval` it, record the result in
`env`, then `print`. Because `eval` reads earlier bindings from `env`, `y = 2 *
(x + 1)` correctly sees `x = 5` and yields `12`.

## 3. Why the outputs are identical

The passes compute the same thing over the same abstract structure; only the
*static typing* of the nodes differs, and typing is erased by the time anything is
printed. Verified output of `solution/main.cpp`:

```
--- Homogeneous (one Node type, runtime tag) ---
  (= x 5)   ==> x = 5
  (= z 27)   ==> z = 27
  (= y (* 2 (+ x 1)))   ==> y = 12

--- Heterogeneous (typed class per construct + RTTI) ---
  (= x 5)   ==> x = 5
  (= z 27)   ==> z = 27
  (= y (* 2 (+ x 1)))   ==> y = 12

Same program, same passes, identical results -- only the typing of the tree differs.
```

Two details worth reading closely. First, `z` prints **folded** (`(= z 27)`)
because both operands of every binop under it are constants, so `fold` collapses
the whole subtree to one `IntLit`/`K_Int`. Second, `y` prints **unfolded**
(`(= y (* 2 (+ x 1)))`) because `x` is a variable — `fold` cannot reduce a subtree
that contains a `VarRef`/`K_Var`, so it leaves it intact and `eval` finishes the
job at `12`. Folding changes the *tree*, not the *answer*: in the starter, with
`fold` a no-op, `z` prints `(= z (* 3 (+ 4 5)))` but still evaluates to `27`.

## 4. The exercise, solved

Both `fold` passes do the same job: recurse into the operands, and if both fold to
a constant, compute the value and return one fresh constant node; otherwise write
the folded children back and return the node unchanged.

**Homogeneous.** You `switch` on the tag and reach operands **by index**:

```cpp
Node *fold(Arena &a, Node *n) {
  if (isBinop(n->kind)) {
    Node *l = fold(a, n->kids[0]);
    Node *r = fold(a, n->kids[1]);
    if (l->kind == K_Int && r->kind == K_Int) {
      long v = 0;
      switch (n->kind) {
      case K_Add: v = l->ival + r->ival; break;
      case K_Sub: v = l->ival - r->ival; break;
      case K_Mul: v = l->ival * r->ival; break;
      case K_Div: v = l->ival / r->ival; break;
      default: break;
      }
      return a.make(Node{K_Int, v, "", {}});
    }
    n->kids[0] = l;
    n->kids[1] = r;
  }
  return n;
}
```

Note what the type does *not* protect you from: `n->kids[0]` and `n->kids[1]` are
just vector indexing. Swap them, or write `kids[2]`, and the code still compiles —
the mistake surfaces only at runtime (a wrong answer, or an out-of-bounds access).

**Heterogeneous.** You `dyn_cast` to the type you need and reach operands **by
name**:

```cpp
Expr *fold(Arena &a, Expr *e) {
  auto *b = dyn_cast<Binary>(e);
  if (!b)
    return e;                       // IntLit / VarRef: nothing to fold
  Expr *l = fold(a, b->lhs);
  Expr *r = fold(a, b->rhs);
  auto *li = dyn_cast<IntLit>(l);
  auto *ri = dyn_cast<IntLit>(r);
  if (li && ri) {
    long v = 0;
    switch (b->op) {
    case '+': v = li->value + ri->value; break;
    case '-': v = li->value - ri->value; break;
    case '*': v = li->value * ri->value; break;
    case '/': v = li->value / ri->value; break;
    }
    return a.make<IntLit>(v);
  }
  b->lhs = l;
  b->rhs = r;
  return b;
}
```

Here `b->lhs` and `b->rhs` are named fields of `Binary`. Mistype one and the
program *does not build* — the error moves from runtime to compile time. And
`dyn_cast<Binary>` guarantees `b->op` is only ever read on a node that actually has
one. That is the heterogeneous payoff, and the cost is the ceremony you see: a
class per construct and `classof`/`dyn_cast` to recover types the homogeneous
version never lost track of (because it never had them).

This is the **expression problem** in miniature. Adding a *new node type* is cheap
in `homo` (a new tag value) and costly in `hetero` (a new class, touched by every
pass). Adding a *new pass* is a single tag-`switch` function in `homo` but is
type-checked and self-documenting in `hetero`. Neither pole is free on both axes.

## 5. Cross-reference to a real compiler

The `hetero` half is a scale model of a production AST:

- **ParserByHand** — `415-labs/ParserByHand/demo-finished/include/Config/AST.h`
  defines the real heterogeneous `Config` AST with exactly this hand-rolled
  `isa`/`cast`/`dyn_cast` RTTI (each node's `classof` over a `Kind` enum, ordered so
  category checks are O(1) range checks). Its enum, range predicates, and visitor
  are all generated from the `ASTNodes.def` X-macro — the Lesson 1–2 idea applied to
  an AST.
- **Gazprea** — `gazc/include/AST/ASTNodes.td` is the same thing in TableGen: one
  record set generates the node enum, RTTI, and visitor via the `gazprea-tblgen`
  backend. Read it after this example and the `Binary`/`Assign` classes here will
  look familiar.

The `homo` half has a production cousin too: an ANTLR parse tree, where an
addition, an `if`, and a call are all one node type told apart by a runtime tag.
**ex06** parses the same `2 * (x + 1)` with a real ANTLR grammar, prints its
homogeneous parse tree, and evaluates it through a generated visitor — the same two
poles you just read, now on real frontends.

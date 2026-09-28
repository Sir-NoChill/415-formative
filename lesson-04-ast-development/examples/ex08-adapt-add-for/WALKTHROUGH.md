# ex08 — Adding a `for` loop, two ways

## 1. What this walkthrough covers

Requirements change, and you have to grow an AST that already works. ex08 adds a
C-style `for` to Mini and makes you *choose the shape of the change*: a new node,
or a rewrite into old nodes. This file narrates both in the code, gives the full
`desugarFor` answer, ties the choice back to the expression problem from Lesson 3,
and then shows the same move at production scale.

The loop under test, built by `buildForLoop`, is:

```cpp
for (let i = 1; i < 6; i = i + 1) { sum = sum + i; }
```

It sums `1+2+3+4+5 = 15`. `main` runs it twice — once each strategy — so the two
answers can be compared directly.

## 2. Strategy A — a first-class `For` node

This is the "add expressive power to the tree" path. In `solution/main.cpp` it is
already done, and it touches the tree's vocabulary in three places:

```cpp
// mini_nodes.def  (1. ONE row -- the Lessons 1-2 X-macro idiom)
MINI_NODE(For)   // <-- this single row generates the Kind::For enumerator

// main.cpp: the enum is generated from the table, never hand-edited
enum class Kind {
#define MINI_NODE(NAME) NAME,
#include "mini_nodes.def"
};

struct For : Stmt {                                                  // 2. class
  Stmt *init; Expr *cond; Stmt *step; Block *body;
  For(Stmt *i, Expr *c, Stmt *s, Block *b) : Stmt(Kind::For), ... {}
  static bool classof(Node *n) { return n->kind == Kind::For; }
};
```

and then the pass that walks statements grows a case:

```cpp
case Kind::For: {                       // 3. interpreter case
  auto *f = cast<For>(s);
  for (exec(f->init); eval(f->cond); exec(f->step))
    exec(f->body);
  break;
}
```

Notice the interpreter's `for` maps one-to-one onto the source `for` — the node
*is* the construct, so the interpreter re-runs `f->init` once, then loops on
`f->cond`, running `f->body` and `f->step`. Because the node survives, any later
pass can still ask "was this a `for`?" and report on it specifically. The cost is
the case itself: here it is one, but every future pass (type check, pretty-print,
lowering) owes the same case. That is the expression-problem tax — adding a
*variant* forces edits to every *operation*.

## 3. Strategy B — desugar into existing nodes

The opposite path spends **no** passes. `desugarFor` rewrites the `For` into a
`Block` of `While` and other nodes the interpreter already understood before
`For` existed:

```
for (init; cond; step) body   ==>   { init; while (cond) { <body stmts>; step; } }
```

In the **starter**, `desugarFor` is stubbed and returns an empty `Block`, so
strategy B executes nothing and `sum` stays `0`. The exercise is to build the
tree above. The solution's answer:

```cpp
Stmt *desugarFor(Arena &a, For *f) {
  auto *innerBody = a.make<Block>();
  for (Stmt *s : f->body->stmts)
    innerBody->stmts.push_back(s);
  innerBody->stmts.push_back(f->step);

  auto *loop = a.make<While>(f->cond, innerBody);

  auto *outer = a.make<Block>();
  outer->stmts.push_back(f->init);
  outer->stmts.push_back(loop);
  return outer;
}
```

Read it as three moves:

1. **inner block** — copy every statement of `f->body`, then append `f->step`
   last. Order matters: the step must run *after* the body on each iteration.
2. **while** — wrap that block in `While(f->cond, innerBody)`. The condition is
   reused verbatim; `while` already re-tests it before each pass.
3. **outer block** — run `f->init` once, then the loop. Returning this `Block` is
   returning a `for` expressed entirely in `Block` + `While`.

Only `Block` and `While` are allocated — nodes the interpreter handled all along.
No `case Kind::For` is needed on the strategy-B path at all.

## 4. Before → after

Verified starter output (`desugarFor` stubbed to an empty `Block`):

```
Strategy A (first-class For node): sum = 15
Strategy B (desugared to while): sum = 0

Both strategies compute 1+2+3+4+5 = 15.
```

Verified solution output (`desugarFor` implemented):

```
Strategy A (first-class For node): sum = 15
Strategy B (desugared to while): sum = 15

Both strategies compute 1+2+3+4+5 = 15.
```

Strategy A never changed — the point is that B *catches up to* A once the
desugaring is right, producing the identical `15` from a tree that contains no
`For` node whatsoever.

## 5. The trade-off, tied to the [expression problem](https://homepages.inf.ed.ac.uk/wadler/papers/expression/expression.txt)

Lesson 3 framed the tension: adding a **variant** (a new node) is cheap in a
class-per-node design but forces a new **operation** case everywhere; adding an
**operation** (a new pass) is cheap but must handle every existing variant. A
`for` node is a new variant — you pay it once per pass, forever. Desugaring dodges
the tax entirely by *not adding a variant* — but the tree then lies about the
source: it says `while`, and no `for`-specific diagnostic, refactoring, or
"unroll this `for`" pass is possible, because the information is gone.

So the rule of thumb: **add a node** when the construct carries meaning a later
pass must see or report on; **desugar** when it is pure convenience over a core
that already exists. Every node is a tax on every pass forever; every desugaring
is a small loss of source fidelity. This is exactly why
[Clang](https://clang.llvm.org/docs/IntroductionToTheClangAST.html) keeps rich
sugar nodes (`ForStmt`, `CXXForRangeStmt`) and desugars *late*, at CodeGen — it
wants the sugar available for diagnostics and tooling first.

## 6. Now do it on real code

You already took the first step the Lessons 1–2 way: adding `For` to the `Kind`
enum was one row in `mini_nodes.def`, not a hand-edit. Production ASTs push that
same table further — it also generates the *visitor* and the *RTTI*, so the whole
of "step 1" disappears and only the node's fields and the passes that care remain.

- **ParserByHand (X-macro AST).** Add one row to
  `415-labs/ParserByHand/demo-finished/include/Config/ASTNodes.def`:

  ```cpp
  AST_NODE(For, Stmt)     // keep the category range contiguous
  ```

  That single row regenerates the `Kind` enum entry, the `visitFor` visitor
  method, and the RTTI predicate. You then write the node's fields in `AST.h` and
  one `case` per pass that actually cares — the same three-part shape as strategy
  A here, but with the enum/visitor/RTTI parts done for you.

- **Gazprea (TableGen AST).** Add a `def` to `gazc/include/AST/ASTNodes.td` and
  rebuild; `gazprea-tblgen` regenerates the visitor and the traversal from the
  `.td`. No enum, no visitor method, no RTTI written by hand.

This is the natural sequel to **Lesson 2's ex04**: there you added an *operation*
to a dialect by adding a `def` and letting TableGen regenerate; here you add a
*node* to an AST the same way. Same move, different artifact — a table row instead
of hand-written switch arms.

## 7. Cross-references

- **ex07** (`../ex07-mini-ast/`) — where the Mini AST and interpreter you adapted
  here were designed and given a semantic pass.
- **Lesson 2 ex04** — adding an operation to a dialect; the `.td`-driven twin of
  this exercise.
- **ParserByHand** `include/Config/ASTNodes.def` and `include/Config/AST.h` — the
  real X-macro AST table this example miniaturizes.
- **Gazprea** `gazc/include/AST/ASTNodes.td` and `gazc/tools/gazprea-tblgen/` —
  the TableGen AST and the emitters that turn it into a visitor and traversal.

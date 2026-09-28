# ex07 — Walkthrough: the Mini AST and its semantic pass

This walkthrough narrates the five design decisions in `solution/main.cpp`, then
the `Sema` pass, then gives the full answer to both stubbed checks in `starter/`.

## 1. The design decisions `(1)..(5)`

Each callout in the solution answers one drawback of the naive
`struct Ast { string type; string value; Ast *left, *right; }` from Lesson 4 §1.

**(1) A `Kind` enum generated from an X-macro, expressions contiguous.** This is
where Lesson 4 reuses Lessons 1–2. The node kinds are not typed out by hand; they
live in `mini_nodes.def`, an X-macro table:

```cpp
// mini_nodes.def  (the single source of truth)
MINI_NODE(Let)  MINI_NODE(Assign) MINI_NODE(If) MINI_NODE(While)
MINI_NODE(Print) MINI_NODE(Block)
MINI_NODE(IntLit) MINI_NODE(BoolLit) MINI_NODE(VarRef) MINI_NODE(Binary)
MINI_NODE_RANGE(Expr, IntLit, Binary)   // expressions are contiguous + last
```

`main.cpp` then `#include`s it three times, each time defining the macros
differently, to generate three artifacts from the one list:

```cpp
enum class Kind {                        // 1. the enum
#define MINI_NODE(NAME) NAME,
#include "mini_nodes.def"
};
#define MINI_NODE_RANGE(BASE, FIRST, LAST)   /* 2. isExprKind() */              \
  static bool is##BASE##Kind(Kind k){ return k >= Kind::FIRST && k <= Kind::LAST; }
#include "mini_nodes.def"
#define MINI_NODE(NAME) case Kind::NAME: return #NAME;   // 3. kindName(), via #
[[maybe_unused]] static const char *kindName(Kind k){ switch(k){
#include "mini_nodes.def"
} return "?"; }
```

Because the expression kinds are a contiguous run, "is this an expression?" is a
range check, not a list of `==`. That single fact powers the RTTI: `Expr::classof`
is just `isExprKind(n->kind)` and `Stmt::classof` its negation. `isa`/`dyn_cast`/
`cast` are the LLVM three-liners built on `classof`. **Why it matters:** dispatch
is an integer compare, a mis-tagged node fails a `dyn_cast` instead of silently
comparing a wrong string, and — the Lesson 2 payoff — **adding a node is one row in
`mini_nodes.def`**; the enum, the range predicate, and the name table all grow
together. This is precisely ParserByHand's `ASTNodes.def` (which also generates its
visitor) and gazc's `ASTNodes.td`, in miniature.

**(2) A `SourceLoc` on every node.** `Node` stores `SourceLoc {line, col}`, set in
the constructor. **Why it matters:** every diagnostic can name the exact code.
Retrofitting locations into the naive AST would touch every construction site;
here it is free because it lives on the base.

**(3) The `Expr`/`Stmt` split.** Two abstract bases, each with its own `classof`.
A `Binary` holds `Expr *lhs, *rhs` — not `Node *` — so you *cannot* build a tree
with a `while` where an operand belongs. **Why it matters:** the C++ type system
enforces the grammar's expr/stmt boundary at compile time, before any pass runs.

**(4) A `type` annotation slot on `Expr`.** `Type type = Type::Error;`. The
checker writes the inferred type back into each expression node. **Why it matters:**
later passes (a lowering or codegen step) read a resolved type off the node
instead of re-deriving it or consulting a side table.

**(5) Named, typed children.** `Binary` has `lhs`/`rhs`; `If` has `cond`,
`thenB`, `elseB` (the last may be null); `Block` has `vector<Stmt*> stmts`.
**Why it matters:** the shape tells the truth about the language — an `if` has
three parts, a block has N — and reads as `i->cond` rather than `kids[0]`.

Ownership is handled by the `Arena`: a `vector<unique_ptr<Node>>` that hands back
raw pointers. The tree aliases freely; the arena frees everything at once. This is
Clang's `ASTContext` in miniature.

## 2. The `Sema` pass

`Sema` holds a **stack of scopes** — `vector<unordered_map<string, Type>>` — and
an error count. `checkBlock` pushes a scope, checks each statement, and pops it, so
inner blocks shadow outer names and go out of scope on exit. `lookup` walks the
stack from innermost (`rbegin`) outward and returns the first match, or `nullptr`.

`checkStmt` is one `switch` on `Kind`; `checkExpr` is another, and it returns the
expression's type *and* records it in the node's `type` slot (decision 4).

Two subtleties the tree lets you get right:

- **A `let`'s name enters scope *after* its initializer is checked.** In the `Let`
  case, `checkExpr(d->init)` runs first, and only then is `d->name` inserted:

  ```cpp
  Type t = checkExpr(d->init);
  if (scopes.back().count(d->name)) error(d->loc, "redeclaration of '" + d->name + "'");
  else scopes.back()[d->name] = t;   // name enters scope AFTER its initializer
  ```

  So `let a = a;` correctly reports `a` as undeclared — the name isn't visible to
  its own initializer.

- **Every diagnostic uses the offending node's `SourceLoc`.** A bad condition is
  reported at `i->cond->loc` (the condition), not `i->loc` (the `if` keyword); an
  undeclared use at the `VarRef`'s own location. That precision is only possible
  because decision (2) put a location on every node.

## 3. The two stubbed checks (the exercise)

`starter/main.cpp` leaves two things for you. Here is the full answer.

**(a) Name resolution — the `VarRef` case in `checkExpr`.** The stub assumes every
variable is a declared `int`. The real version consults the scope stack:

```cpp
case Kind::VarRef: {
  auto *v = cast<VarRef>(e);
  if (Type *t = lookup(v->name))
    return e->type = *t;
  error(v->loc, "use of undeclared '" + v->name + "'");
  return e->type = Type::Error;
}
```

Found: record and return the declared type. Not found: report at the `VarRef`'s
location and return `Type::Error` (which the `Binary` and `Assign` cases treat as
"already reported", so one mistake doesn't cascade into spurious follow-on errors).

**(b) Condition types — the `If` and `While` cases in `checkStmt`.** The stub calls
`checkExpr(i->cond)` but ignores the result. Require `bool`:

```cpp
case Kind::If: {
  auto *i = cast<If>(s);
  if (checkExpr(i->cond) != Type::Bool)
    error(i->cond->loc, "if condition must be bool");
  checkBlock(*i->thenB);
  if (i->elseB) checkBlock(*i->elseB);
  break;
}
case Kind::While: {
  auto *w = cast<While>(s);
  if (checkExpr(w->cond) != Type::Bool)
    error(w->cond->loc, "while condition must be bool");
  checkBlock(*w->body);
  break;
}
```

## 4. Before → after

With both checks stubbed, only the assignment-to-undeclared check (never stubbed)
fires. Verified "before" output:

```
=== Checking the buggy program ===
  2:1: error: assignment to undeclared 'c'
  1 semantic error(s).
```

The other two bugs — `print a + d` (undeclared `d`) and `if (a + 1)` (int
condition) — slip through: `d` is assumed to be a declared `int`, and the `if`
condition type is never checked.

After implementing both stubs, all three bugs are caught, each at its own
location. Verified output:

```
=== Checking the buggy program ===
  2:1: error: assignment to undeclared 'c'
  3:11: error: use of undeclared 'd'
  4:5: error: if condition must be bool
  3 semantic error(s).
```

Note the locations: `3:11` is the `d` inside `print a + d`, and `4:5` is the
condition expression, not the `if` keyword at `4:1`. **1 error becomes 3.**

## 5. Cross-reference to a real compiler

The pass you just finished has the same shape as a production one:

- **ParserByHand** (`../../../../415-labs/ParserByHand/demo-finished/`) —
  `lib/Sema.cpp` is a real name-resolution pass with exactly this structure: a
  scope stack, a recursive walk, and diagnostics carrying source locations from
  `include/Config/SourceLocation.h`. Its heterogeneous AST and hand-rolled RTTI
  live in `include/Config/AST.h` — the same `Kind` + `classof` scheme, only its
  node hierarchy is **generated** from the `AST_NODE(...)` rows of
  `include/Config/ASTNodes.def` (the X-macro from Lessons 1–2), including the
  contiguous category range that makes `isExprKind`-style checks a range test.
- **Gazprea** (this repo's compiler) — the same node hierarchy is generated from
  `gazc/include/AST/ASTNodes.td`, a TableGen table instead of an X-macro; its
  real semantic analysis lives in `gazc/lib/Sema/`.

You wrote the whole thing inline in one file so you could see it. The only
difference at scale is that the node classes and the RTTI are code-generated from a
table — which is exactly the machinery Lessons 1–2 taught.

## Forward to ex08

You have designed a Mini AST and a pass over it. **ex08** takes objective 4: adapt
this AST to add a C-style `for` loop, two ways — a new `For` node vs. desugaring
into the existing `while` — and weighs the trade-off (new power in the tree vs. a
tax on every pass that walks it).

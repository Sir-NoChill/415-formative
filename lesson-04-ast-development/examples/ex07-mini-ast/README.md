# ex07 -- Designing a Mini AST and a semantic pass over it

**Lesson 4.** Design a heterogeneous AST from scratch,
then walk it. No dependencies (plain C++17).

```bash
$CXX -std=c++17 -Wall -Wextra solution/main.cpp -o ex07 && ./ex07
$CXX -std=c++17 -Wall -Wextra starter/main.cpp  -o ex07 && ./ex07   # before the task
```

(`$CXX` is your C++17 compiler.)

A well-designed AST for **Mini** -- a tiny imperative
language (`let`/assign/`if`/`while`/`print`, over
`int`/`bool` expressions) -- plus a semantic-analysis
pass that does scoped name resolution and type checking
in one walk. Both programs it checks are built by hand
with explicit source locations (a parser is ex06's
job), so the diagnostics print as real `line:col:
error:` messages.

- `solution/` -- the full Mini AST and a complete
  `Sema` pass.
- `starter/` -- the *same* AST; two checks in `Sema`
  are stubbed for you.
- `mini_nodes.def` -- the X-macro node table the `Kind`
  enum is generated from.
- **`WALKTHROUGH.md`** -- the design decisions
  `(1)..(5)`, both stubbed answers, and the
  cross-reference to a real compiler.

## 1. What this example teaches

This is the Lesson 4 driver for objectives 1-3:

- **Analyze an existing AST** for design drawbacks
  (objective 1).
- **Design an AST from scratch** for a small language
  (objective 2).
- **Implement a semantic analysis pass** -- name
  resolution + type checking (objective 3).

Objective 4 (adapting the AST) is ex08's job.

## 2. The naive AST, and how Mini fixes it

Lesson 4 sec. 1 opens with a first attempt that works
on a calculator and collapses the moment the language
grows:

```cpp
struct Ast {
  std::string type;   // "add", "num", "var", "if"...
  std::string value;  // number text OR name OR operator
  Ast *left, *right;  // only ever two children?
};
```

Its drawbacks, and where `solution/main.cpp` fixes each
with a numbered callout:

- **Stringly-typed kind** (`type == "add"`) -- every
  typo a silent runtime bug. -> `(1)` a `Kind` enum,
  one value per concrete node, with LLVM-style
  `isa`/`dyn_cast`/`cast` RTTI. The enum is **not
  hand-written**: it is generated from
  `mini_nodes.def`, an **X-macro** table (the Lessons
  1-2 idiom), which is also the single source of the
  `isExprKind()` range predicate and a `kindName()`
  debug string -- one list, three artifacts that can
  never drift. Expressions are kept **contiguous** at
  the end of the table, so `isExprKind()` is a range
  check. This is exactly how ParserByHand's
  `ASTNodes.def` and gazc's `ASTNodes.td` work.
- **No source locations** -- you can never point a
  diagnostic at the code. -> `(2)` a `SourceLoc {line,
  col}` on every `Node`, set at construction.
- **No expr/stmt distinction** -- nothing stops a
  `while` where a number belongs. -> `(3)` an
  `Expr`/`Stmt` split; each has its own `classof`, so
  the C++ type system itself rejects putting a
  statement where an expression goes.
- **No annotation slots** -- resolved types have
  nowhere to live, so passes grow fragile external
  maps. -> `(4)` `Expr` carries a `type` slot the
  checker fills in.
- **Fixed binary shape / overloaded payload** --
  `left`/`right` and one `value` field can't honestly
  model `if` (cond + two blocks), a block of
  statements, or a named operator. -> `(5)` **named,
  typed children**: `Binary` has `lhs`/`rhs`, `If` has
  `cond`/`thenB`/`elseB`, `Block` has a
  `vector<Stmt*>`. No `kids[0]` convention to remember.
- **Unclear ownership** -- raw `new` leaks. -> an
  `Arena` owns every node (`unique_ptr` pool); the tree
  holds raw pointers, exactly Clang's `ASTContext`
  idea.

## 3. How to build and run

Solution (verified output; exit code 1):

```
=== Checking the good program ===
  OK: no semantic errors.

=== Checking the buggy program ===
  2:1: error: assignment to undeclared 'c'
  3:11: error: use of undeclared 'd'
  4:5: error: if condition must be bool
  3 semantic error(s).
```

The good program (`let a = 3; let b = a + 4; if (b <
20) { print b; }`) passes. The buggy one carries three
    deliberate bugs and each diagnostic points at the
    offending node's own location.

## 4. What to look at

- `solution/main.cpp` -- read top to bottom. The
  `(1)..(5)` callouts are the whole design; the `Sema`
  class below them is the whole pass. `checkStmt` and
  `checkExpr` are one `switch` on `Kind` each.
- Note in `checkStmt`'s `Let` case that the name enters
  scope **after** its initializer is checked -- so `let
  a = a;` correctly reports `a` as undeclared.

## 5. The exercise

In `starter/main.cpp` the AST is complete but two
checks in `Sema` are stubbed:

- **Name resolution** -- the `VarRef` case in
  `checkExpr` ignores the scope stack and assumes every
  variable is a declared `int`.
- **Condition types** -- the `If` and `While` cases
  check the condition expression but never require it
  to be `bool`.

With those stubs, only the assignment-to-undeclared
check (which is *not* stubbed) fires. Verified "before"
output; exit code 1:

```
=== Checking the good program ===
  OK: no semantic errors.

=== Checking the buggy program ===
  2:1: error: assignment to undeclared 'c'
  1 semantic error(s).
```

**Task:** implement the two stubs (do not touch the AST
or the driver). The buggy program should go from **1**
reported error to **3**, matching the solution above.
The full answer to both stubs is in `WALKTHROUGH.md`.

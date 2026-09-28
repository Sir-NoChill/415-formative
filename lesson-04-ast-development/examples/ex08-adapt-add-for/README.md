# ex08 -- Adapting an AST: adding a `for` loop

**Lesson 4, objective 4.** Adapt an existing AST to add
a new construct. No dependencies (plain C++17).

```bash
$CXX -std=c++17 -Wall -Wextra solution/main.cpp -o ex08 && ./ex08
$CXX -std=c++17 -Wall -Wextra starter/main.cpp  -o ex08 && ./ex08   # before the task
```

(`$CXX` is your C++17 compiler.)

You inherit the **Mini** AST from ex07
(`Let/Assign/If/While/For/Print/Block` +
`IntLit/VarRef/Binary`) and a tree-walking `Interp`.
The task is to add a C-style

```
for (init; cond; step) body
```

and there are two honest ways to do it. Choosing
between them *is* the lesson.

## The two strategies

- **A -- a first-class `For` node.** New expressive
  power in the tree, but every pass that walks
  statements grows a `For` case. Here that is just the
  interpreter's `exec`; in a real compiler it is also
  the type checker, the pretty-printer, and lowering.
  This is the expression-problem tax from Lesson 3,
  made concrete.
- **B -- desugar into existing nodes.** `for (init;
  cond; step) body` becomes `{ init; while (cond) {
  body; step; } }`, built from `Block` and `While`
  alone. *Zero* passes change -- but the tree no longer
  records that the user wrote a `for`, so
  `for`-specific diagnostics become impossible.

> **Rule of thumb.** Add a **node** when the construct
> carries meaning later passes must see or report on;
> **desugar** when it is pure convenience over an
> existing core. Every node you add is a tax on every
> pass forever; every desugaring is a small loss of
> source fidelity. This is why
> [Clang](https://clang.llvm.org/docs/IntroductionToTheClangAST.html)
> keeps rich sugar nodes and desugars *late*, while a
> teaching compiler often desugars early to keep its
> core tiny.

## The exercise

`solution/main.cpp` implements **both** strategies and
runs the same loop -- `for (let i = 1; i < 6; i = i +
1) { sum = sum + i; }` -- each way. In the starter,
   strategy A is already wired up and `desugarFor` is
   stubbed to an empty `Block`. Your job is to
   implement `desugarFor` so strategy B goes from `sum
   = 0` to `sum = 15`, matching strategy A. Do **not**
   touch anything else.

Verified starter output (before the task):

```
Strategy A (first-class For node): sum = 15
Strategy B (desugared to while): sum = 0

Both strategies compute 1+2+3+4+5 = 15.
```

Verified solution output (after implementing
`desugarFor`):

```
Strategy A (first-class For node): sum = 15
Strategy B (desugared to while): sum = 15

Both strategies compute 1+2+3+4+5 = 15.
```

## Touch points for strategy A

Adding the `For` node cost edits in three places even
in this tiny file:

1. **one row** in the `mini_nodes.def` X-macro table
   (`MINI_NODE(For)`), which generates the `Kind` enum
   entry (and `kindName()`) -- the Lessons 1-2 idiom,
   so the enum itself is never hand-edited,
2. a `struct For : Stmt` class with
   `init/cond/step/body` and `classof`,
3. a `case Kind::For:` in the interpreter's `exec`.

In a real front end the same node also demands a case
in the **type checker**, a case in the
**pretty-printer**, and a case in **lowering** -- every
pass, forever. Strategy B pays none of that, at the
cost of source fidelity. (And note: because the enum is
table-generated, step 1 is where ParserByHand and gazc
stop entirely -- their visitor and RTTI regenerate from
that one row too; see the walkthrough.)

## Files

- `solution/` -- both strategies, `desugarFor` fully
  implemented.
- `starter/`  -- same file with `desugarFor` stubbed;
  implement it.
- `mini_nodes.def` -- the X-macro node table (the `For`
  row is called out in it).
- **`WALKTHROUGH.md`** -- both strategies in the code,
  the `desugarFor` answer, and how the same "adapt"
  move looks on real code (ParserByHand, Gazprea).

# Lesson 1 — Foundations: X-Macros and TableGen

> **Who this is for.** Fourth-year CS students who have seen C++ but don't feel
> fluent in it. Every bit of jargon is defined the first time it appears. You do
> not need to be comfortable with templates or the preprocessor going in — that's
> what this lesson builds.

## Learning objectives

By the end of this lesson you will be able to:

1. **Define the term *X-macro*.**
2. **Explain the role of TableGen in the LLVM infrastructure as an advanced
   X-macro system.**
3. **Relate the use of X-macros to code hygiene.**
4. **Identify instances where an X-macro or TableGen contributes to good code
   quality.**

## How to work through this lesson

Read this document top to bottom. It sends you into two hands-on examples at the
points where they'll make the most sense:

| Example | Folder | What you'll do |
|---|---|---|
| **ex01** | `examples/ex01-colors-xmacro/` | Turn three hand-maintained lists into one X-macro. Plain `g++`, no dependencies. |
| **ex02** | `examples/ex02-hello-tablegen/` | Express the *same* list as TableGen records and run the real `llvm-tblgen`. |

Each example has a `starter/`, a `solution/`, and a `WALKTHROUGH.md` with the
documented answer. Do the starter first; peek at the solution when stuck.

---

## 1. What is an X-macro? (objective 1)

Start with the problem it solves. Suppose your program has a list of things —
colours, tokens, CPU registers, error codes — and several parts of the code each
need that list in a *different shape*: an `enum` for one, an array of names for
another, a `switch` for a third. The naive approach writes the list out once per
shape:

```cpp
enum Color { Red, Green, Blue };                 // list, shape 1
const char *name(Color c) {                      // list, shape 2
  switch (c) { case Red: return "Red"; /* ... */ }
}
```

Now the list exists in two places that must be kept identical *by hand*. Add a
colour to the enum, forget the switch, and you have a bug. These are **parallel
lists**, and keeping them in sync manually is a classic, avoidable mistake.

An **X-macro** removes the duplication. The definition:

> An **X-macro** is a list of data written **once** in a file (conventionally
> named `*.def` or `*.inc`), where each item is written as a call to a macro whose
> meaning is *left undefined by the file itself*. A piece of code that wants the
> list `#define`s that macro to mean whatever shape it needs, then `#include`s the
> file. The C preprocessor stamps out one piece of code per item. Including the
> file several times, with a different `#define` each time, produces several
> different shapes from the one list.

The name comes from the traditional macro name `X(...)`. The `.def` file is just
a list of `X(...)` rows; the "X" is a blank the includer fills in.

Concretely, the list lives in a file like this (no include guard — we *want* it
included many times):

```cpp
// colors.def
#ifndef COLOR
#define COLOR(Name, Hex)   // harmless default if the includer forgot to define it
#endif
COLOR(Red,   0xFF0000)
COLOR(Green, 0x00FF00)
COLOR(Blue,  0x0000FF)
#undef COLOR               // leave no macro state behind
```

and a consumer picks a meaning and includes it:

```cpp
enum Color {
#define COLOR(Name, Hex) Name,   // each row -> `Name,`
#include "colors.def"
};
```

> **➡ Do ex01 now.** `examples/ex01-colors-xmacro/` walks through this exact
> pattern, shows the parallel-list bug in the starter, and has you fix it. Come
> back here afterward.

### The three tell-tale features

Every well-formed X-macro file has these, and you should be able to point at them:

1. **No include guard.** Normal headers guard against multiple inclusion; X-macro
   files depend on it.
2. **A no-op fallback `#define`** for each macro, so a consumer that only cares
   about *some* of the macros still compiles.
3. **An `#undef` at the end** for each macro, so no macro state leaks into
   whatever includes the file next.

---

## 2. Code hygiene: why bother? (objective 3)

"Code hygiene" means habits that keep a codebase healthy over time — easy to
change safely, hard to break by accident. X-macros improve hygiene in a specific,
nameable way:

- **Single source of truth.** The list exists in exactly one place. Two lists that
  must agree can drift apart; one list cannot disagree with itself. In ex01 you
  *see* the drift bug (`Cyan` renders as `<unknown> #000000`) and then watch it
  become structurally impossible.
- **Changes are local and atomic.** Adding an item is one new row. You cannot
  "half-add" it — you can't add it to the enum but forget the name table, because
  there is no separate name table to forget.
- **Derived facts stay correct.** A count like `NumColors`, placed right after the
  generated entries, is always exactly right because it's generated from the same
  list.

The trade-off, stated honestly: macro-generated code is harder to read at a glance
and harder to step through in a debugger. So X-macros pay off when a list is
consumed in several shapes or is likely to grow; they're overkill for a list used
once. Knowing *when not to* is part of the hygiene too.

---

## 3. TableGen: an advanced X-macro system (objective 2)

The C preprocessor is a blunt instrument. It only pastes text. It cannot do
arithmetic, it cannot check that your data is well-formed, it cannot let one item
inherit fields from another, and its data evaporates after compilation — nothing
outside the compiler can inspect it. Real compilers have lists that badly need all
of those things: LLVM's list of machine instructions, for example, carries dozens
of structured fields per instruction and feeds a dozen different generated files.

**TableGen** is LLVM's answer. It is a small domain-specific language for writing
down structured data, plus a tool (`llvm-tblgen`, and its MLIR sibling
`mlir-tblgen`) that reads that data and generates code from it.

> Think of TableGen as **an X-macro system with a real language behind it.** Same
> core idea — *write the list once, generate many things from it* — but now the
> "list" is a set of typed **records** with fields, inheritance, and computed
> values, and the "stamping" is done by a real program (a *backend*) instead of by
> text substitution.

The vocabulary is small:

- A **`class`** is a schema — it names the fields a record has. (Like a `struct`
  definition: it describes shape, it isn't itself a value.)
- A **`def`** is a concrete record built from a class. (One row of data.)
- Records can **inherit** from other records and **compute** field values with
  operators like `!add` and `!shl`.
- A **backend** is a generator that walks the records and emits output — C++
  headers, documentation, JSON, whatever.

The same colours from ex01, as TableGen:

```tablegen
class Color<string name, int r, int g, int b> {
  string Name = name;
  int Packed = !add(!shl(r, 16), !add(!shl(g, 8), b));  // computed! cpp can't do this
}
def Red : Color<"Red", 255, 0, 0>;
```

> **➡ Do ex02 now.** `examples/ex02-hello-tablegen/` runs the real `llvm-tblgen`
> on this file, shows the records it parses, dumps them as JSON, and reads them
> back with a tiny Python "backend." It makes the "advanced X-macro" claim
> concrete.

### How the two relate

| | X-macro | TableGen |
|---|---|---|
| The "list" is… | rows of text (`X(...)`) | typed records (`def`) with fields |
| Expanded by… | the C preprocessor | a backend program in `*-tblgen` |
| Can compute / validate / inherit? | no | yes |
| Data visible outside the compiler? | no | yes (e.g. `--dump-json`) |
| Dependencies | none (it's just cpp) | the LLVM TableGen toolchain |

TableGen doesn't *replace* the X-macro idea — it industrialises it. In fact
TableGen's own generated output is consumed using the classic X-macro include
trick (you'll see `#define GET_OP_CLASSES` / `#include "...inc"` in Lesson 2).

---

## 4. Spotting good uses in the wild (objective 4)

You can now recognise where these techniques earn their keep. Reach for one when
you see a **single conceptual list feeding multiple derived artifacts.** Some real
examples living on this machine:

- **`llvm/include/llvm/IR/Instruction.def`** — every LLVM IR instruction, listed
  once, expanded into enums (`Instruction.h`) *and* a visitor + dispatch switch
  (`InstVisitor.h`). A textbook X-macro.
- **`clang/include/clang/Basic/TokenKinds.def`** — every C/C++ token and keyword,
  once, with layered macros for keywords and language-version flags.
- **The Gazprea compiler in this repo** uses *both* idioms deliberately:
  - Hand-written X-macros for lexer tables: `gazc/include/Operators.def`,
    `gazc/include/Keywords.def`, `gazc/include/Sema/Builtins.def`.
  - TableGen for its MLIR dialect (`gazc/include/Gazprea/GazpreaOps.td`) and for
    its AST node hierarchy (`gazc/include/AST/ASTNodes.td`).
  - It even has one file, `gazc/include/Sema/SemaChecks.def`, whose own comment
    says it stays an X-macro *"so we avoid the TableGen dependency"* — a real
    engineer making exactly the trade-off this lesson describes.

**The judgement call.** X-macro when the list is simple, flat, and consumed only
by your own C/C++. TableGen when the data is rich, is consumed by several tools,
or needs computation, validation, or inheritance — and the project can carry the
extra toolchain. Picking the *lighter* tool that still does the job is itself good
hygiene.

---

## Check yourself

You've met the objectives if you can answer these without looking:

1. In your own words, what is an X-macro, and what are its three structural
   tell-tales?
2. Show a two-list drift bug and explain how an X-macro makes it impossible.
3. Name two things TableGen can do that the C preprocessor fundamentally cannot.
4. Point at a file in LLVM, Clang, or Gazprea and justify why an X-macro (or
   TableGen) is a good fit there.

When you're ready to compare the two techniques head-to-head, analyze real
constructs, and extend them yourself, continue to **Lesson 2**
(`../lesson-02-synthesis/`).

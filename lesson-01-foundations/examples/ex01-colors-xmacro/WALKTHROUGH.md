# ex01 -- The X-macro (walkthrough & documented solution)

## 1. What this example teaches

This example maps to three Lesson 1 objectives at once:

- **Define the term X-macro.**
- **Relate the use of X-macros to code hygiene.**
- **Identify instances where an X-macro contributes to
  good code quality.**

The whole idea fits in one sentence: **an X-macro is a
list you write down once and expand many different
ways.** You define the list in a `.def` file, then
`#include` it several times -- each time with the macro
defined to mean something different.

## 2. The problem (look at `starter/main.cpp` first)

The starter keeps a list of colours in **three**
places:

1. the `enum Color { ... }`,
2. the `colorName()` switch,
3. the `colorHex()` switch.

Nothing forces those three lists to agree. Someone
added `Cyan` to the enum and forgot the two switches.
Build and run it:

```bash
$CXX -std=c++17 -Wall -Wextra starter/main.cpp -o colors && ./colors
```

You get (verified):

```
There are 5 colors:
  Red        #FF0000
  Green      #00FF00
  Blue       #0000FF
  White      #FFFFFF
  <unknown>  #000000     <-- Cyan drifted: wrong name AND wrong colour
```

This "parallel lists that must be kept in sync by hand"
situation is one of the most common sources of quiet
bugs in C and C++. It is exactly the *code hygiene*
problem X-macros solve.

## 3. The fix (the X-macro)

An X-macro file has three tell-tale features. Open
`solution/colors.def`:

```tablegen
#ifndef COLOR
#define COLOR(Name, Hex)   // 1. a no-op fallback so a careless includer compiles
#endif

COLOR(Red,   0xFF0000)     // 2. the ONE list of data, as macro "calls"
COLOR(Green, 0x00FF00)
...
#undef COLOR               // 3. clean up so no macro state leaks out
```

Notice there is **no include guard** -- that's
deliberate. A normal header is meant to be included
once; an X-macro file is meant to be included *many*
times.

Now the consumer (`solution/main.cpp`) defines `COLOR`
to mean whatever it needs, then includes the list. Each
`#include` "stamps out" one line per row:

```cpp
// Build the enum: each row becomes `Name,`
enum Color {
#define COLOR(Name, Hex) Name,
#include "colors.def"
  NumColors
};

// Build the name table: each row becomes a string
static const char *ColorNames[] = {
#define COLOR(Name, Hex) #Name,
#include "colors.def"
};

// Build the hex table: each row becomes its number
static const unsigned ColorHex[] = {
#define COLOR(Name, Hex) Hex,
#include "colors.def"
};
```

`#Name` is the preprocessor's *stringize* operator --
it turns the token `Red` into the string literal
`"Red"`. That one trick is why the same list can
produce both identifiers (for the enum) and strings
(for the name table).

Build and run the solution:

```bash
$CXX -std=c++17 -Wall -Wextra solution/main.cpp -o colors && ./colors
```

Verified output -- `Cyan` is now correct everywhere:

```
There are 5 colors:
  Red        #FF0000
  Green      #00FF00
  Blue       #0000FF
  White      #FFFFFF
  Cyan       #00FFFF
```

## 4. Your exercise

Add a new colour -- say `Magenta` (`0xFF00FF`) -- to
the program.

- In the **starter**, notice you'd have to edit the
  enum *and* both switches, and it's easy to forget
  one.
- In the **solution**, add a single line to
  `colors.def`: `COLOR(Magenta, 0xFF00FF)` and rebuild.
  The enum, the name table, the hex table, and
  `NumColors` all update themselves. **One edit, zero
  drift.**

That difference -- *one line vs. three edits that can
silently disagree* -- is the entire point.

## 5. Why this is "good code hygiene"

- **Single source of truth.** The list of colours lives
  in exactly one place. Two lists that must agree are a
  bug waiting to happen; one list cannot disagree with
  itself.
- **Adding an item is local and mechanical.** New
  colour = new row. You can't half-add one.
- **The count comes for free.** `NumColors` sits right
  after the generated entries, so it's always exactly
  right.

## 6. Where you'll see this for real

This is not a toy trick -- it is everywhere in LLVM and
Clang:

- `llvm/include/llvm/IR/Instruction.def` lists every
  LLVM IR instruction once, and `Instruction.h` /
  `InstVisitor.h` include it to build enums, a visitor
  interface, *and* a dispatch switch -- all from that
  one list.
- `clang/include/clang/Basic/TokenKinds.def` lists
  every C/C++ token and keyword once.
- The Gazprea compiler in this repo uses the same idiom
  for `include/Keywords.def`, `include/Operators.def`,
  `include/Sema/Builtins.def`, and more.

In **ex02** you'll take this very same colour list and
express it in TableGen -- a tool built to scale this
idea up to lists that are far too rich for the
preprocessor to handle.

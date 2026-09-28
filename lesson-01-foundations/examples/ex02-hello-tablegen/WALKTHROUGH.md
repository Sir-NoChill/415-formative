# ex02 — Hello, TableGen (walkthrough & documented solution)

## 1. What this example teaches

This example maps to the Lesson 1 objective **"Explain
the role of TableGen in the LLVM infrastructure as an
advanced X-macro system."**

In ex01 you kept a list of colors as *rows in an
X-macro table* (`colors.def`), and the C preprocessor
stamped those rows into an enum, a name array, and a
hex array. Here we keep **the exact same list of
colors** — but written as **TableGen records** in
`colors.td` instead of preprocessor rows. Same data,
completely different (and more powerful) machinery
underneath.

The one-sentence version: *an X-macro is a list of text
the preprocessor pastes; a TableGen file is a list of
structured records that a real program can read,
compute over, and turn into anything.*

## 2. Prerequisite

TableGen is a language that ships **inside LLVM**. Its
interpreter/driver is a command-line tool called
`llvm-tblgen`. You don't install anything extra — if
you have an LLVM build, you have `llvm-tblgen`.

This example finds it via the `LLVM_DIR` environment
variable (default `~/Code/Compilers/LLVM/22.1.7`), the
same convention the Gazprea project uses. You don't
have to set anything by hand: **`run.sh` locates the
tool for you**. If your LLVM lives elsewhere, run
`LLVM_DIR=/path/to/llvm ./run.sh`.

## 3. How to read a `.td` file

Open `solution/colors.td`. Two keywords do almost all
the work:

- **`class`** is a *schema* (a template): it lists the
  fields every record of that kind will have. `class
  Color<string name, int r, int g, int b>` says "a
  Color has a name and three colour channels," and
  inside the braces it declares fields like `int R =
  r;`. Think of it like a `struct` definition — it
  describes shape, it is not itself a value.

- **`def`** is a *concrete record* (an instance) built
  from a class: `def Red : Color<"Red", 255, 0, 0>;` is
  one actual colour. Think of it as one row of data, or
  one object.

- **`: PrimaryColor<...>`** is **inheritance**. `class
  PrimaryColor<...> : Color<...>` means "a PrimaryColor
  *is a* Color, plus a bit more." Every field of
  `Color` is inherited; `PrimaryColor` just pins one
  extra field.

- **`let`** *overrides* an inherited field. Inside
  `PrimaryColor` you'll see `let IsPrimary = 1;`, which
  changes the default `bit IsPrimary = 0;` that `Color`
  declared.

So the file reads as: one schema (`Color`), one
specialised schema (`PrimaryColor`), and five records
(`Red`, `Green`, `Blue`, `White`, `Cyan`).

## 4. The two exercises (do them in `starter/colors.td`)

`starter/colors.td` is the solution with two things
removed. Your job:

**TASK 1 — add a computed field.** Inside `class
Color`, add:

```tablegen
int Packed = !add(!shl(r, 16), !add(!shl(g, 8), b));
```

`!shl` (shift-left) and `!add` are **TableGen
operators**. `!shl(r, 16)` shifts red into the top
byte, and the `!add`s combine the three channels into
one packed integer `0xRRGGBB`. Here is the key point:
**the C preprocessor cannot do arithmetic at all** — it
only pastes text, so an X-macro could never compute
this for you. TableGen *evaluating* an expression like
this is one concrete reason it is called an "advanced
X-macro."

**TASK 2 — add the missing colour.** After `def White
...`, add:

```tablegen
def Cyan : Color<"Cyan", 0, 255, 255>;
```

That's it. You never touch any C++, any enum, or any
switch statement.

## 5. Running it

```bash
./run.sh starter     # run llvm-tblgen on the (incomplete) starter file
./run.sh solution    # run it on the finished file
```

`run.sh` shows three views of the same `.td` file:

**(a) `llvm-tblgen --print-records`** — the parsed
records in human-readable form. Once you've done TASK 1
and 2, `Cyan` appears with its computed field:

```
def Cyan {	// Color
  string Name = "Cyan";
  int R = 0;
  int Green = 255;
  int B = 255;
  int Packed = 65535;    // TableGen computed this: 65535 == 0x00FFFF
  bit IsPrimary = 0;
}
```

Notice `Packed = 65535` — **TableGen did the math for
you** at generation time.

**(b) `llvm-tblgen --dump-json`** — the *same* records
as a machine-readable JSON data model. This is the
thing an X-macro can never give you: the data exists as
inspectable structured output, not just as tokens
buried inside the preprocessor.

**(c) `show.py`** — a ~10-line "backend" (in plain
Python) that reads that JSON and prints one tidy line
per colour. With the finished file you get exactly:

```
  Blue     #0000FF  primary=1
  Cyan     #00FFFF  primary=0
  Green    #00FF00  primary=1
  Red      #FF0000  primary=1
  White    #FFFFFF  primary=0
```

Run it against the **starter before you do TASK 1** and
every line instead reads:

```
  Blue     <missing: do TASK 1>  primary=1
```

because `Packed` doesn't exist yet. Great feedback
loop: the tool tells you exactly what's missing.

## 6. TableGen vs X-macro — the "advanced" part

Put ex01 and ex02 side by side. Both describe one list
in one place. Here is what TableGen adds:

1. **The data is real, queryable output — not just
   preprocessor state.** An X-macro's rows only ever
   exist *inside the C preprocessor, at compile time,
   in one translation unit*; nothing else can look at
   them. A TableGen file produces a persistent,
   structured model. Proof: `show.py` is plain Python
   with **no C++ compiler involved at all**, and it
   reads every field.

2. **TableGen can compute, inherit, and constrain.**
   `!add`/`!shl` compute a field; `PrimaryColor :
   Color` reuses a schema; TableGen can also validate
   values. The preprocessor can do *none* of this — it
   only pastes text.

3. **In real LLVM/MLIR the "backend" is C++, not
   Python.** Our `show.py` is a toy backend. In
   production, the generator is a C++ program compiled
   *into* `llvm-tblgen` / `mlir-tblgen`, and it emits
   `.inc` files full of C++ — enums, classes, tables.
   You'll see exactly this in **ex04**, where
   `mlir-tblgen` turns an operations `.td` file into
   fully-formed C++ classes for a compiler dialect.

## 7. When to reach for TableGen instead of an X-macro

Prefer **TableGen** when:
- the data is rich or structured (many fields, nested
  relationships),
- several different tools or output files need to
  consume the same list,
- you need computed fields, inheritance, or validated
  values.

Prefer a plain **X-macro** when the data is a simple
flat list and only your own C/C++ code consumes it — it
needs no extra toolchain and is trivially readable.

Be honest about the cost: TableGen is *a whole extra
language and a build-time tool* to learn and depend on.
That price is easily worth it for something the size of
LLVM; it's overkill for a five-line enum. Choosing the
lighter tool when it's enough is itself part of the
"code hygiene" story from ex01.

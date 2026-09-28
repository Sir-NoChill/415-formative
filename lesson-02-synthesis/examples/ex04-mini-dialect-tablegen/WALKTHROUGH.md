# ex04 — A real MLIR dialect built with mlir-tblgen

## 1. What this example teaches

This is the capstone Lesson 2 example. It targets:

- **Analyze the effect of a given TableGen construct.**
- **Adapt an existing set of TableGen tools to your own custom project.**

You'll build a small but **real** MLIR *dialect* — a set of custom compiler
operations — where the C++ is generated from TableGen by `mlir-tblgen`, exactly
the way the `gaz` dialect in the Gazprea compiler is built. This is the "advanced
X-macro" idea from ex02 taken all the way to production: instead of a Python
script printing colour lines, a C++ generator emits **complete C++ classes**.

> **Vocabulary.** MLIR is an LLVM sub-project for building compilers. A *dialect*
> is a namespaced group of *operations* (think: custom instructions). Here the
> dialect is called `demo` and its ops are `demo.add` (and, after the exercise,
> `demo.mul`).

## 2. Prerequisites

You need the local MLIR build. This example finds it via `LLVM_DIR` (default
`~/Code/Compilers/LLVM/22.1.7`) and `MLIR_DIR`, the same convention as the Gazprea
project's `.envrc.template`. **`run.sh` sets both for you.**

```bash
./run.sh            # configure + build + run the tool on the test inputs
./run.sh clean      # delete the build directory
```

The first build links MLIR and takes a little while; later builds are fast.

## 3. The project layout (and where TableGen sits)

```
include/Demo/DemoDialect.td   # the dialect + a base op class      <- TableGen
include/Demo/DemoOps.td       # the operations (STARTER: only demo.add)  <- TableGen
include/Demo/DemoDialect.h    # #includes the generated *.h.inc
include/Demo/DemoOps.h        #   "
lib/Demo/DemoDialect.cpp      # registers the ops (uses generated GET_OP_LIST)
lib/Demo/DemoOps.cpp          # #includes the generated *.cpp.inc
demo-opt/demo-opt.cpp         # a tiny opt-style driver that parses/prints MLIR
solution/DemoOps.td           # the finished ops file (demo.add + demo.mul)
test/add.mlir, test/mul.mlir  # inputs to feed the tool
```

The single CMake line that runs the generator is in `include/Demo/CMakeLists.txt`:

```cmake
add_mlir_dialect(DemoOps demo)
```

That one call runs `mlir-tblgen` several times over `DemoOps.td` and writes, into
the build directory:

- `DemoOps.h.inc` / `DemoOps.cpp.inc` — the C++ **op classes**,
- `DemoOpsDialect.h.inc` / `DemoOpsDialect.cpp.inc` — the C++ **dialect class**.

## 4. Analyzing the TableGen construct: read one op

Open `include/Demo/DemoOps.td` and read `Demo_AddOp`:

```tablegen
def Demo_AddOp : Demo_Op<"add", [Pure, SameOperandsAndResultType]> {
  let summary = "Integer addition.";
  let arguments = (ins I32:$lhs, I32:$rhs);
  let results   = (outs I32:$result);
  let assemblyFormat = "$lhs `,` $rhs attr-dict `:` type($result)";
}
```

Line by line — this is the "analyze a TableGen construct" skill:

- `def Demo_AddOp : Demo_Op<"add", ...>` — define an op with the textual mnemonic
  `demo.add`. `Demo_Op` is the base class from `DemoDialect.td`.
- `[Pure, SameOperandsAndResultType]` — **traits**. `Pure` = no side effects.
  `SameOperandsAndResultType` = all three types are equal, which is what lets the
  printer show a single `: i32` and the parser infer the operand types from it.
- `arguments = (ins I32:$lhs, I32:$rhs)` — two `i32` inputs, named. `(ins ...)` is
  a *dag*, TableGen's parenthesised list notation.
- `results = (outs I32:$result)` — one `i32` output.
- `assemblyFormat = "..."` — a **declarative grammar**. From this one string,
  `mlir-tblgen` generates *both* the parser and the printer for the textual form
  `demo.add %a, %b : i32`. No hand-written C++.

From those five lines the generator produces a full C++ class: a builder,
accessors (`getLhs()`, `getRhs()`), a verifier, a parser, and a printer. That is
an enormous amount of correct, consistent C++ from a tiny declaration — the payoff
of TableGen.

### See it actually work

```bash
./run.sh
```

`demo.add` round-trips (verified):

```
demo-opt test/add.mlir
module {
  func.func @use_add(%arg0: i32, %arg1: i32) -> i32 {
    %0 = demo.add %arg0, %arg1 : i32
    return %0 : i32
  }
}
```

If the tool parsed the file, the generated parser worked — proof the TableGen did
its job. `test/mul.mlir`, meanwhile, fails in the starter state:

```
error: custom op 'demo.mul' is unknown
```

## 5. Your exercise: adapt the tools — add `demo.mul`

Teach the dialect a multiply op **without writing any C++**.

1. Open `include/Demo/DemoOps.td` and, where the `// TASK` comment is, add a second
   op identical in shape to `Demo_AddOp` but named `mul`:

   ```tablegen
   def Demo_MulOp : Demo_Op<"mul", [Pure, SameOperandsAndResultType]> {
     let summary = "Integer multiplication.";
     let arguments = (ins I32:$lhs, I32:$rhs);
     let results   = (outs I32:$result);
     let assemblyFormat = "$lhs `,` $rhs attr-dict `:` type($result)";
   }
   ```

   (The complete answer is in `solution/DemoOps.td`; you can diff against it.)

2. Rebuild and test:

   ```bash
   ./run.sh
   ```

`test/mul.mlir` now round-trips (verified against the solution):

```
module {
  func.func @use_mul(%arg0: i32, %arg1: i32) -> i32 {
    %0 = demo.mul %arg0, %arg1 : i32
    return %0 : i32
  }
}
```

### What you did *not* have to touch — the whole point

- No parser, printer, verifier, or builder — `mlir-tblgen` generated them.
- No registration edit in `DemoDialect.cpp`. Look at it:

  ```cpp
  addOperations<
  #define GET_OP_LIST
  #include "Demo/DemoOps.cpp.inc"
      >();
  ```

  `GET_OP_LIST` is generated **from the same `.td`**, so your new op registered
  itself. Notice that this include is the exact `.def`/X-macro idiom from Lesson 1
  — the *same* `.inc` file is included in `DemoOps.h` (with `GET_OP_CLASSES`) to
  get declarations and here to get the op list. TableGen produced the file; the
  classic include protocol consumes it.

You extended a compiler's instruction set by adding one declaration. That is
"adapting an existing set of TableGen tools to your own project."

## 6. Connect it back to Gazprea

This mini dialect is a scaled-down copy of the real thing:

- `gazc/include/Gazprea/GazpreaOps.td` defines `gaz.constant`, `gaz.binary`, etc.
  with the *same* `arguments` / `results` / `assemblyFormat` vocabulary you just
  used.
- `gazc/include/Gazprea/CMakeLists.txt` calls `add_mlir_dialect(GazpreaOps gaz)` —
  the same one-liner as ours.
- Gazprea even ships a **custom** TableGen tool, `gazprea-tblgen`, with
  hand-written backends that generate its AST visitor and traversal from
  `include/AST/ASTNodes.td`. That is the natural next step beyond this example:
  once you understand that TableGen is "a data model plus a code generator," you
  can write your *own* generator backend, not just use the ones LLVM ships.

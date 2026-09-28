//===- DemoOps.td - Operations of the `demo` dialect -----------*- tablegen -*-===//
//
// STARTER STATE: this file defines exactly ONE operation, `demo.add`.
//
// Your exercise (see ../WALKTHROUGH.md) is to add a second operation, `demo.mul`,
// by writing ONE more `def` here -- no C++ required. The reference answer is in
// ../solution/DemoOps.td.
//
// Each `def` below is fed to mlir-tblgen, which generates a full C++ class
// (parser, printer, verifier, accessors, builders) into DemoOps.h.inc /
// DemoOps.cpp.inc. That generated code is what the hand-written .cpp files use.
//
//===----------------------------------------------------------------------===//

#ifndef DEMO_OPS
#define DEMO_OPS

include "Demo/DemoDialect.td"
include "mlir/Interfaces/InferTypeOpInterface.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

// demo.add : add two i32 values, producing an i32.
//
//   * `Pure` (a Trait) tells MLIR the op has no side effects.
//   * `SameOperandsAndResultType` lets the printer show a single type and lets
//     the parser infer the operand types from it -- that is why the assembly
//     format below only mentions `type($result)`.
//   * `arguments` / `results` are DAGs listing typed operands and results.
//   * `assemblyFormat` is a declarative grammar: from this one line mlir-tblgen
//     generates BOTH the textual parser and the printer. No hand-written C++.
def Demo_AddOp : Demo_Op<"add", [Pure, SameOperandsAndResultType]> {
  let summary = "Integer addition.";
  let description = [{
    Adds two 32-bit integers.

    Example:
    ```mlir
    %2 = demo.add %0, %1 : i32
    ```
  }];

  let arguments = (ins I32:$lhs, I32:$rhs);
  let results = (outs I32:$result);

  let assemblyFormat = "$lhs `,` $rhs attr-dict `:` type($result)";
}

// TASK: add `Demo_MulOp` here (mnemonic "mul"), identical in shape to AddOp.

#endif // DEMO_OPS

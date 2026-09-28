//===- DemoOps.td - SOLUTION -----------------------------------*- tablegen -*-===//
//
// The finished operations file: `demo.add` AND `demo.mul`. To apply the answer,
// copy this over the starter file and rebuild:
//     cp solution/DemoOps.td include/Demo/DemoOps.td
//     cmake --build build
//     ./build/bin/demo-opt test/mul.mlir      # now round-trips
//
// The only change from the starter is the second `def` (Demo_MulOp). Note what
// you did NOT have to touch: no C++, no parser, no printer, no registration list
// -- mlir-tblgen regenerated all of it, and GET_OP_LIST re-registered the op.
//
//===----------------------------------------------------------------------===//

#ifndef DEMO_OPS
#define DEMO_OPS

include "Demo/DemoDialect.td"
include "mlir/Interfaces/InferTypeOpInterface.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

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

def Demo_MulOp : Demo_Op<"mul", [Pure, SameOperandsAndResultType]> {
  let summary = "Integer multiplication.";
  let description = [{
    Multiplies two 32-bit integers.

    Example:
    ```mlir
    %2 = demo.mul %0, %1 : i32
    ```
  }];

  let arguments = (ins I32:$lhs, I32:$rhs);
  let results = (outs I32:$result);

  let assemblyFormat = "$lhs `,` $rhs attr-dict `:` type($result)";
}

#endif // DEMO_OPS

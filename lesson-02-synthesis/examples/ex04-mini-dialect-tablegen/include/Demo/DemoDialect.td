//===- DemoDialect.td - The `demo` dialect --------------------*- tablegen -*-===//
//
// This defines the dialect itself (its name and C++ namespace) and a base class
// that every operation in the dialect inherits from. mlir-tblgen turns the
// `Dialect` record below into a C++ class `DemoDialect`, and the CMake rule
// `add_mlir_dialect(DemoOps demo)` runs that generation for us.
//
//===----------------------------------------------------------------------===//

#ifndef DEMO_DIALECT
#define DEMO_DIALECT

include "mlir/IR/OpBase.td"

// The dialect record. Compare this to the `gaz` dialect in the Gazprea compiler
// (include/Gazprea/GazpreaDialect.td) -- same shape, different name.
def Demo_Dialect : Dialect {
  let name = "demo";
  let summary = "A tiny arithmetic dialect for the TableGen lesson.";
  let cppNamespace = "::mlir::demo";
}

// Base class: "a Demo op is an Op in the Demo dialect." Every op def below uses
// this so it does not repeat the dialect wiring.
class Demo_Op<string mnemonic, list<Trait> traits = []> :
    Op<Demo_Dialect, mnemonic, traits>;

#endif // DEMO_DIALECT

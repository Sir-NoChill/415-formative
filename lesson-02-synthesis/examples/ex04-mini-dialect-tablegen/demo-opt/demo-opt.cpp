//===- demo-opt.cpp - The `demo` dialect's opt-style driver -----*- C++ -*-===//
//
// A minimal MLIR tool that registers the `demo` dialect (plus the core dialects
// its operands use) and hands control to MlirOptMain. That gives us a real
// command-line tool that PARSES and PRINTS textual MLIR:
//
//   demo-opt test/add.mlir            # round-trips the IR back out
//
// If parsing succeeds, every op in the file was recognized -- which means the
// C++ parser mlir-tblgen generated from DemoOps.td actually works.
//
//===----------------------------------------------------------------------===//

#include "mlir/IR/DialectRegistry.h"
#include "mlir/InitAllDialects.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"

#include "Demo/DemoDialect.h"

int main(int argc, char **argv) {
  mlir::DialectRegistry registry;
  // Register OUR dialect...
  registry.insert<mlir::demo::DemoDialect>();
  // ...plus all core dialects, so builtin ops like func.func / arith.constant
  // that appear in the test inputs also parse.
  mlir::registerAllDialects(registry);

  return mlir::asMainReturnCode(mlir::MlirOptMain(
      argc, argv, "demo optimizer driver\n", registry));
}

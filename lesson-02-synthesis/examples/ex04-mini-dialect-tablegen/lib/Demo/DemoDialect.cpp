//===- DemoDialect.cpp - `demo` dialect registration ------------*- C++ -*-===//
//
// The only hand-written glue in the whole dialect: tell MLIR which operations
// belong to the dialect. The list of ops is itself generated (GET_OP_LIST), so
// adding an op in DemoOps.td automatically registers it here too.
//
//===----------------------------------------------------------------------===//

#include "Demo/DemoDialect.h"
#include "Demo/DemoOps.h"

using namespace mlir;
using namespace mlir::demo;

// Definitions of the generated dialect class (constructor, name, etc.).
#include "Demo/DemoOpsDialect.cpp.inc"

void DemoDialect::initialize() {
  addOperations<
#define GET_OP_LIST
#include "Demo/DemoOps.cpp.inc"
      >();
}

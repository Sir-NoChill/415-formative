//===- DemoOps.cpp - `demo` dialect ops definitions -------------*- C++ -*-===//
//
// Brings in the DEFINITIONS of every generated op class (parsers, printers,
// verifiers, builders). Because DemoOps.td used a declarative `assemblyFormat`
// and only standard traits, there is nothing left to write by hand here.
//
//===----------------------------------------------------------------------===//

#include "Demo/DemoOps.h"
#include "Demo/DemoDialect.h"

#define GET_OP_CLASSES
#include "Demo/DemoOps.cpp.inc"

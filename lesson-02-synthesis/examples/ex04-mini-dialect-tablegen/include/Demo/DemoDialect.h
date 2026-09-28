//===- DemoDialect.h - `demo` dialect C++ interface -------------*- C++ -*-===//
//
// This header pulls in the DIALECT class that mlir-tblgen generated from
// DemoDialect.td. The generated declaration lives in DemoOpsDialect.h.inc, which
// only exists after the build has run mlir-tblgen. That is the whole point of
// this lesson: the .h.inc is machine-written from the .td.
//
//===----------------------------------------------------------------------===//

#ifndef DEMO_DEMODIALECT_H
#define DEMO_DEMODIALECT_H

#include "mlir/Bytecode/BytecodeOpInterface.h"
#include "mlir/IR/Dialect.h"

#include "Demo/DemoOpsDialect.h.inc"

#endif // DEMO_DEMODIALECT_H

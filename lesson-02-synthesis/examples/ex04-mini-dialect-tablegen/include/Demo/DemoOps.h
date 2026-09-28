//===- DemoOps.h - `demo` dialect ops C++ interface -------------*- C++ -*-===//
//
// Pulls in the OPERATION classes generated from DemoOps.td. The macro dance
//     #define GET_OP_CLASSES
//     #include "Demo/DemoOps.h.inc"
// is itself an X-macro-style include: the SAME .inc file is included here with
// GET_OP_CLASSES defined (to get class declarations) and again in DemoOps.cpp
// (to get their definitions). TableGen generated the .inc; the include protocol
// is the classic .def idiom you saw in Lesson 1.
//
//===----------------------------------------------------------------------===//

#ifndef DEMO_DEMOOPS_H
#define DEMO_DEMOOPS_H

#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/Interfaces/InferTypeOpInterface.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"

#define GET_OP_CLASSES
#include "Demo/DemoOps.h.inc"

#endif // DEMO_DEMOOPS_H

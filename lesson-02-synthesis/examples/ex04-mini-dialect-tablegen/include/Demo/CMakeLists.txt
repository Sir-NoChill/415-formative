# This ONE line is the heart of the lesson. `add_mlir_dialect` runs mlir-tblgen
# on DemoOps.td several times, producing (in the build dir, next to this path):
#   DemoOps.h.inc / DemoOps.cpp.inc            (the op classes)
#   DemoOpsDialect.h.inc / DemoOpsDialect.cpp.inc  (the dialect class)
# and defines a target `MLIRDemoOpsIncGen` that other targets depend on.
#
# Arguments: (td-file-basename  dialect-name)
add_mlir_dialect(DemoOps demo)

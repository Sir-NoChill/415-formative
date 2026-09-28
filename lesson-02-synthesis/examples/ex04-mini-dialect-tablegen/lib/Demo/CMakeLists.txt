# Build the dialect as an MLIR library. The DEPENDS line forces mlir-tblgen to
# run (generating the *.inc files) BEFORE these .cpp files are compiled -- they
# #include those generated headers.
add_mlir_dialect_library(MLIRDemo
  DemoDialect.cpp
  DemoOps.cpp

  ADDITIONAL_HEADER_DIRS
  ${PROJECT_SOURCE_DIR}/include/Demo

  DEPENDS
  MLIRDemoOpsIncGen

  LINK_LIBS PUBLIC
  MLIRIR
  MLIRInferTypeOpInterface
)

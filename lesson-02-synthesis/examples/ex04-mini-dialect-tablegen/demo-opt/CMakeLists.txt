# The demo-opt driver. MLIROptLib provides MlirOptMain; the RegisterAll* libs and
# our MLIRDemo library provide the dialects it registers.
set(LIBS
  MLIRDemo
  MLIROptLib
  MLIRRegisterAllDialects
  MLIRRegisterAllPasses
)

add_llvm_executable(demo-opt demo-opt.cpp)
llvm_update_compile_flags(demo-opt)
target_link_libraries(demo-opt PRIVATE ${LIBS})
mlir_check_all_link_libraries(demo-opt)

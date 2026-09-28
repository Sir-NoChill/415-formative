# Find the local MLIR/LLVM build and pull in its CMake helpers.
#
# Discovery uses $MLIR_DIR (same convention as the Gazprea project's
# .envrc.template). Point it at your LLVM 22 build's lib/cmake/mlir, e.g.
#   export LLVM_DIR="$HOME/Code/Compilers/LLVM/22.1.7"
#   export MLIR_DIR="$LLVM_DIR/lib/cmake/mlir"
find_package(MLIR REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using MLIRConfig.cmake in: ${MLIR_DIR}")

set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/bin)
set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/lib)
set(MLIR_BINARY_DIR ${CMAKE_BINARY_DIR})

list(APPEND CMAKE_MODULE_PATH "${MLIR_CMAKE_DIR}")
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")

# These modules provide add_mlir_dialect(), mlir_tablegen(), add_llvm_executable().
include(TableGen)
include(AddLLVM)
include(AddMLIR)
include(HandleLLVMOptions)

# Upstream headers as SYSTEM includes so their warnings stay quiet.
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
include_directories(SYSTEM ${MLIR_INCLUDE_DIRS})
# Our own headers, and the build dir where TableGen writes the *.inc files.
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_BINARY_DIR}/include)

link_directories(${LLVM_BUILD_LIBRARY_DIR})

separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND "${LLVM_DEFINITIONS}")
add_definitions(${LLVM_DEFINITIONS_LIST})

#===----------------------------------------------------------------------===//
# ex04 - a minimal out-of-tree MLIR dialect built with mlir-tblgen.
#
# Build (from this directory):
#   export LLVM_DIR="$HOME/Code/Compilers/LLVM/22.1.7"
#   export MLIR_DIR="$LLVM_DIR/lib/cmake/mlir"
#   cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Release
#   cmake --build build
#   ./build/bin/demo-opt test/add.mlir
#
# See ../run.sh for a scripted version and WALKTHROUGH.md for the exercise.
#===----------------------------------------------------------------------===//
cmake_minimum_required(VERSION 3.20)
project(DemoDialect LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/get_mlir.cmake")

add_subdirectory(include/Demo)
add_subdirectory(lib/Demo)
add_subdirectory(demo-opt)

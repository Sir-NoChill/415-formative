// Round-trips with the STARTER dialect (demo.add exists).
//   demo-opt test/add.mlir
func.func @use_add(%a: i32, %b: i32) -> i32 {
  %0 = demo.add %a, %b : i32
  return %0 : i32
}

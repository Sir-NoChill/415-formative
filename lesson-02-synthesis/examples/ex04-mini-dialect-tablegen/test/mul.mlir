// Uses demo.mul, which does NOT exist in the starter dialect.
// Before the exercise: `demo-opt test/mul.mlir` fails with
//     error: 'demo.mul' op created with unregistered dialect / custom op not found
// After you add Demo_MulOp to include/Demo/DemoOps.td and rebuild: it round-trips.
func.func @use_mul(%a: i32, %b: i32) -> i32 {
  %0 = demo.mul %a, %b : i32
  return %0 : i32
}

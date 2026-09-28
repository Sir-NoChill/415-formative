# ex04 -- mini-dialect-tablegen

**Lesson 2.** Analyze a TableGen construct; adapt a set
of TableGen tools. Needs a local **MLIR 22** build
(`mlir-tblgen` + MLIR libs), CMake >= 3.20, and Ninja.

```bash
./run.sh          # configure + build the `demo` dialect, then run demo-opt on the tests
./run.sh clean    # remove build/
```

A real, minimal out-of-tree MLIR **dialect** (`demo`)
whose C++ op classes are generated from TableGen by
`mlir-tblgen` -- the same machinery as the `gaz`
dialect in `gazc`. In the starter it has one op,
`demo.add`; the exercise is to add `demo.mul` by
writing a single `def`.

- `include/Demo/*.td` -- the dialect and ops in
  TableGen (starter state: `demo.add` only).
- `lib/Demo/`, `demo-opt/` -- the (tiny) hand-written
  C++ glue and an opt-style driver.
- `test/add.mlir` (round-trips) and `test/mul.mlir`
  (fails until you do the exercise).
- `solution/DemoOps.td` -- the finished ops file
  (`demo.add` + `demo.mul`).
- **`WALKTHROUGH.md`** -- reading an MLIR op def, and
  the add-an-operation exercise.

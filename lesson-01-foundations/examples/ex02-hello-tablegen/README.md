# ex02 — hello-tablegen

**Lesson 1.** TableGen as an advanced X-macro system.
Needs `llvm-tblgen` (via `LLVM_DIR`).

```bash
./run.sh starter     # the incomplete file (two TODO tasks)
./run.sh solution    # the finished file
```

`run.sh` shows the same `.td` three ways:
`--print-records`, `--dump-json`, and `show.py` (a tiny
Python "backend" over the JSON).

- `starter/colors.td`  — missing a computed field and a
  color (your tasks).
- `solution/colors.td` — the same color list as ex01,
  now as TableGen records with a computed `Packed`
  field and inheritance.
- **`WALKTHROUGH.md`** — how to read a `.td` file, the
  tasks, and the X-macro-vs-TableGen contrast.

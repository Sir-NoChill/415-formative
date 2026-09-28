# ex01 — colors-xmacro

**Lesson 1.** Define an X-macro; relate it to code hygiene. No dependencies (plain C++17).

```bash
g++ -std=c++17 -Wall -Wextra starter/main.cpp  -o colors && ./colors   # the drift bug
g++ -std=c++17 -Wall -Wextra solution/main.cpp -o colors && ./colors   # fixed by an X-macro
```

- `starter/`  — three parallel lists that have drifted (`Cyan` renders wrong).
- `solution/` — one `colors.def` X-macro drives the enum, name table, and hex table.
- **`WALKTHROUGH.md`** — the documented solution, the exercise, and why it's good hygiene.

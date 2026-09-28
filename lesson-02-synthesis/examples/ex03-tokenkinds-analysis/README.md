# ex03 — tokenkinds-analysis

**Lesson 2.** Analyze an X-macro; adapt an X-macro template. No dependencies (plain C++17).

```bash
g++ -std=c++17 -Wall -Wextra solution/main.cpp -o lexdemo && ./lexdemo "1+2*(3-4)"
g++ -std=c++17 -Wall -Wextra starter/main.cpp  -o lexdemo && ./lexdemo "5%2=1"   # before the task
```

A lexer token table (mirrors the *ParserByHand* lab and Clang's `TokenKinds.def`).
The same six-row `tokens.def` is included **four** times — an enum, two name
switches, and the lexer's character switch.

- `solution/` — full token table + a toy lexer.
- `starter/`  — same `main.cpp` (don't edit it!); `tokens.def` is missing two punctuators for you to add.
- **`WALKTHROUGH.md`** — how to trace each expansion, and the extend-the-lexer exercise.

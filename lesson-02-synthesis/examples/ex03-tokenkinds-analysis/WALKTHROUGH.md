# ex03 — Analyzing and adapting a token-kinds X-macro

## 1. What this example teaches

This is a Lesson 2 (synthesis) example. It targets:

- **Analyze the effect of a given X-macro.**
- **Adapt an existing X-macro template to your own
  custom repository.**

It deliberately mirrors the token X-macro from the real
CMPUT 415 *ParserByHand* lab and, in spirit,
`clang/include/clang/Basic/TokenKinds.def`. So the
skill you practice here is exactly the skill you'll use
reading a real compiler front end.

## 2. The `.def` file, and the "layering" trick

Open `solution/tokens.def`. It defines **two** macros:

```tablegen
#ifndef TOK
#define TOK(Name)                       // token with no fixed spelling
#endif
#ifndef PUNCTUATOR
#define PUNCTUATOR(Spelling, Name) TOK(Name)   // a punctuator IS-A token
#endif

TOK(eof)  TOK(unknown)  TOK(identifier)  TOK(number)
PUNCTUATOR('+', plus)   PUNCTUATOR('-', minus)   ...   PUNCTUATOR(')', r_paren)
```

The clever part is the fallback `#define
PUNCTUATOR(Spelling, Name) TOK(Name)`. It means: *if a
consumer only knows about `TOK`, every `PUNCTUATOR`
still shows up as a plain `TOK`.* So a consumer that
wants **all** tokens defines only `TOK`; a consumer
that cares **specifically about punctuators** (like the
lexer's character switch) defines `PUNCTUATOR` and lets
the plain `TOK`s vanish. One list, two audiences.

## 3. Analyzing the effect: trace each expansion

`solution/main.cpp` includes `tokens.def` **four
times**. The core analysis skill is: *mentally
substitute the macro body into every row.* Do that for
each site.

1. **The `TokenKind` enum.** `#define TOK(Name)
   tok_##Name,` (the `##` pastes tokens together, so
   `TOK(plus)` → `tok_plus,`). Because `PUNCTUATOR`
   falls back to `TOK`, punctuators are included too.
   Result: one enumerator per row, plus a free
   `NUM_TOKENS` count at the end.

2. **`tokenTypeName()`** uses `#Name` (stringize) to
   give every token a readable name for debugging.

3. **`describeToken()`** defines *only* `PUNCTUATOR`,
   so plain `TOK`s expand to nothing and only the six
   punctuators produce `case` arms returning their
   literal spelling (`'+'`, `'('`, …).

4. **`punctuatorFor(char c)`** — the actual lexer step
   — again defines only `PUNCTUATOR`, turning each row
   into `case '+': return tok_plus;`.

Build and run (verified):

```bash
g++ -std=c++17 -Wall -Wextra solution/main.cpp -o lexdemo && ./lexdemo "1+2*(3-4)"
```
```
There are 10 token kinds.
Lexing: 1+2*(3-4)
  number     "1"
  plus       '+'
  star       '*'
  l_paren    '('
  ...
```

**The analysis payoff:** every one of those four
constructs is driven by the same six-line list. Change
the list and all four change together — that is what
you are learning to *see* when you read code like this.

## 4. Your exercise: adapt the template

Switch to `starter/`. Its `main.cpp` is **identical**
to the solution's — you will not touch it. Only
`starter/tokens.def` differs: two punctuators are
missing.

Run it first to see the "before":

```bash
g++ -std=c++17 -Wall -Wextra starter/main.cpp -o lexdemo && ./lexdemo "5%2=1"
```
Verified "before" output — `%` and `=` are not
recognised:
```
  number     "5"
  unknown    unknown
  number     "2"
  unknown    unknown
  number     "1"
```

**Task:** add two rows to `starter/tokens.def`:

```tablegen
PUNCTUATOR('=', equal)
PUNCTUATOR('%', percent)
```

Rebuild (do **not** edit `main.cpp`) and re-run. Now
`5%2=1` lexes as `number percent number equal number`.

The thing to notice — and the reason this exercise
exists — is that you extended the lexer's vocabulary in
**four** places (enum, two name switches, the character
switch) by editing **one** file and adding **two**
lines. You never opened the switch statements. *That*
is adapting an X-macro template.

## 5. Cross-reference to a real compiler

You have now built, in miniature, the exact structure
used in production:

- **ParserByHand**
  (`../../../../415-labs/ParserByHand/`) —
  `TokenKinds.def` drives its `TokenKind` enum plus the
  lexer's spelling/description switches.
- **Clang** —
  `clang/include/clang/Basic/TokenKinds.def` does the
  same at full scale, with extra layered macros for
  keywords, C++11 keywords, and language availability
  flags.
- **Gazprea** (this repo's compiler) —
  `include/Operators.def` and `include/Keywords.def`
  are the lexer's hand-written X-macro tables.

Open any of those now; you'll recognise the shape
immediately. In **ex04** you'll see what happens when a
compiler decides a list has outgrown the preprocessor
and moves it to TableGen instead.

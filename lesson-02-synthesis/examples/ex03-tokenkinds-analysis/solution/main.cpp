// ============================================================================
// ex03 SOLUTION  --  Analyzing (and extending) a token-kinds X-macro
// ============================================================================
//
// tokens.def is included FOUR times below, each with a different meaning for the
// TOK / PUNCTUATOR macros. Read each expansion and note what one line of the
// .def file turns into in each case. This is the "analyze the effect of a given
// X-macro" skill: mentally substitute the macro body into every row.
//
// Build & run:
//   g++ -std=c++17 -Wall -Wextra main.cpp -o lexdemo && ./lexdemo "1+2*(3-4)"
// ============================================================================

#include <cstdio>

// --- Expansion 1: the TokenKind enum --------------------------------------
// TOK(x) -> tok_x,   and PUNCTUATOR falls back to TOK, so punctuators are
// included too. Result: one enumerator per row, plus a NUM_TOKENS count.
enum TokenKind : unsigned {
#define TOK(Name) tok_##Name,
#include "tokens.def"
  NUM_TOKENS
};

// --- Expansion 2: a human-readable name for EVERY token -------------------
static const char *tokenTypeName(TokenKind K) {
  switch (K) {
#define TOK(Name)                                                              \
  case tok_##Name:                                                             \
    return #Name;
#include "tokens.def"
  default:
    return "???";
  }
}

// --- Expansion 3: describe punctuators by their literal spelling -----------
// Here we ONLY define PUNCTUATOR, so plain TOKs expand to nothing and only the
// six punctuator rows produce case arms.
static const char *describeToken(TokenKind K) {
  switch (K) {
#define PUNCTUATOR(Spelling, Name)                                            \
  case tok_##Name:                                                            \
    return #Spelling;
#include "tokens.def"
  default:
    return tokenTypeName(K);
  }
}

// --- Expansion 4: the lexer's character -> token switch --------------------
// Map a single character to its punctuator token. Again only PUNCTUATOR is
// defined, so only punctuator rows appear.
static TokenKind punctuatorFor(char c) {
  switch (c) {
#define PUNCTUATOR(Spelling, Name)                                            \
  case Spelling:                                                              \
    return tok_##Name;
#include "tokens.def"
  default:
    return tok_unknown;
  }
}

// A toy lexer that recognizes numbers and the punctuators above.
int main(int argc, char **argv) {
  const char *src = (argc > 1) ? argv[1] : "1+2*(3-4)";
  printf("There are %u token kinds.\n", NUM_TOKENS);
  printf("Lexing: %s\n", src);

  for (const char *p = src; *p;) {
    if (*p == ' ') { ++p; continue; }
    if (*p >= '0' && *p <= '9') {
      const char *start = p;
      while (*p >= '0' && *p <= '9') ++p;
      printf("  %-10s \"%.*s\"\n", tokenTypeName(tok_number),
             (int)(p - start), start);
      continue;
    }
    TokenKind k = punctuatorFor(*p);
    printf("  %-10s %s\n", tokenTypeName(k), describeToken(k));
    ++p;
  }
  return 0;
}

// ============================================================================
// ex01 STARTER  --  "The two-list problem"
// ============================================================================
//
// This program keeps a list of colors in THREE places:
//   1. the `Color` enum,
//   2. the colorName() switch,
//   3. the colorHex() switch.
//
// These three lists must be kept in sync BY HAND. Someone just added `Cyan`
// to the enum but forgot to update the two switches. That is exactly the class
// of bug X-macros exist to make impossible.
//
// Build & run:
//   $CXX -std=c++17 -Wall -Wextra main.cpp -o colors && ./colors
//
// Your task (see ../WALKTHROUGH.md): replace all three hand-maintained lists
// with a single X-macro table so they can never drift apart again. The finished
// version lives in ../solution/.
// ============================================================================

#include <cstdio>

enum Color {
  Red,
  Green,
  Blue,
  White,
  Cyan, // <-- newly added to the enum ONLY
  NumColors
};

const char *colorName(Color c) {
  switch (c) {
  case Red:
    return "Red";
  case Green:
    return "Green";
  case Blue:
    return "Blue";
  case White:
    return "White";
  // BUG: no `case Cyan:` -- falls through to the default.
  default:
    return "<unknown>";
  }
}

unsigned colorHex(Color c) {
  switch (c) {
  case Red:
    return 0xFF0000;
  case Green:
    return 0x00FF00;
  case Blue:
    return 0x0000FF;
  case White:
    return 0xFFFFFF;
  // BUG: no `case Cyan:` -- returns 0 (black) instead of cyan.
  default:
    return 0x000000;
  }
}

int main() {
  printf("There are %d colors:\n", NumColors);
  for (int i = 0; i < NumColors; ++i) {
    Color c = static_cast<Color>(i);
    printf("  %-10s #%06X\n", colorName(c),
           colorHex(c));
  }
  // Notice Cyan prints as "<unknown> #000000" -- the lists have drifted.
  return 0;
}

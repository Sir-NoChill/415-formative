// ============================================================================
// ex01 SOLUTION  --  One list, many expansions
// ============================================================================
//
// Every list of colors below is generated from the SINGLE table in colors.def.
// Adding a color there updates the enum, the name table, the hex table, and the
// count all at once. Drift is now impossible: there is only one list.
//
// Build & run:
//   $CXX -std=c++17 -Wall -Wextra main.cpp -o colors && ./colors
// ============================================================================

#include <cstdio>

// --- Expansion 1: build the enum ------------------------------------------
// Each COLOR(Name, Hex) becomes `Name,`. The trailing NumColors is a free
// "how many?" constant because it sits right after the last generated entry.
enum Color {
#define COLOR(Name, Hex) Name,
#include "colors.def"
  NumColors
};

// --- Expansion 2: build the name lookup table -----------------------------
// Each row becomes a string literal, indexed by the enum value.
static const char *ColorNames[] = {
#define COLOR(Name, Hex) #Name,
#include "colors.def"
};

// --- Expansion 3: build the hex lookup table ------------------------------
static const unsigned ColorHex[] = {
#define COLOR(Name, Hex) Hex,
#include "colors.def"
};

const char *colorName(Color c) {
  return ColorNames[c];
}
unsigned colorHex(Color c) { return ColorHex[c]; }

int main() {
  printf("There are %d colors:\n", NumColors);
  for (int i = 0; i < NumColors; ++i) {
    Color c = static_cast<Color>(i);
    printf("  %-10s #%06X\n", colorName(c),
           colorHex(c));
  }
  // Cyan now prints correctly -- it was declared in exactly one place.
  return 0;
}

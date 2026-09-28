// ============================================================================
// preprocess-demo.cpp  --  watch the preprocessor expand an X-macro
// ============================================================================
//
// This file exists to be run through the PREPROCESSOR ALONE, so you can see the
// exact code an X-macro produces. It has no <cstdio> or other headers, so the
// -E output is short and readable -- just the three generated tables.
//
//   g++     -E -P preprocess-demo.cpp      # stop after preprocessing, no #line noise
//   clang++ -E -P preprocess-demo.cpp      # identical output
//
//   -E  = run only the preprocessor, print the result and stop.
//   -P  = omit the "# 12 file" line markers, so the output is clean.
//
// (#include "colors.def" resolves relative to THIS file, so no -I is needed.)
// ============================================================================

enum Color {
#define COLOR(Name, Hex) Name,
#include "colors.def"
  NumColors
};

static const char *ColorNames[] = {
#define COLOR(Name, Hex) #Name,
#include "colors.def"
};

static const unsigned ColorHex[] = {
#define COLOR(Name, Hex) Hex,
#include "colors.def"
};

//===- colors.td - The SAME color list, now as TableGen records -----------===//
//
// Compare this file to ex01's colors.def. Both describe one list of colors in
// one place. But an X-macro row is just text the preprocessor pastes; a TableGen
// `def` is a structured RECORD that a tool can read, query, and compute over.
//
// Run it:
//   llvm-tblgen --print-records colors.td      # human-readable record dump
//   llvm-tblgen --dump-json     colors.td      # machine-readable data model
//
// (See ../run.sh for the exact commands wired to your local LLVM build.)
//
//===----------------------------------------------------------------------===//

// A `class` is a template/schema: it names the fields every color record has.
class Color<string name, int r, int g, int b> {
  string Name = name;
  int R = r;
  int G = g;
  int B = b;

  // A DERIVED field. TableGen evaluates this expression for every record.
  // An X-macro CANNOT do this -- the C preprocessor only pastes text, it can
  // not do arithmetic. This is one concrete sense in which TableGen is an
  // "advanced X-macro": the list carries computed data, not just tokens.
  int Packed = !add(!shl(r, 16), !shl(g, 8), b);

  bit IsPrimary = 0; // default; overridden by the PrimaryColor subclass below
}

// Inheritance: a PrimaryColor IS a Color with one field pinned to 1. Reusing a
// schema like this is another thing plain X-macros do not give you.
class PrimaryColor<string name, int r, int g, int b> : Color<name, r, g, b> {
  let IsPrimary = 1;
}

//    def-name        class          "Name"    R    G    B
def Red   : PrimaryColor<"Red",     255,   0,   0>;
def Green : PrimaryColor<"Green",     0, 255,   0>;
def Blue  : PrimaryColor<"Blue",      0,   0, 255>;
def White : Color       <"White",   255, 255, 255>;
def Cyan  : Color       <"Cyan",      0, 255, 255>;

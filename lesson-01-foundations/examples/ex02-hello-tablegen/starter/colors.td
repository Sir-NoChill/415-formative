//===- colors.td - STARTER --------------------------------------------------===//
//
// This is the same idea as ex01's colors.def, but expressed as TableGen records.
// It is INCOMPLETE. Your tasks (see ../WALKTHROUGH.md):
//
//   TASK 1: Add a derived field `Packed` to the Color class that TableGen
//           computes as (R<<16 | G<<8 | B). Hint: use !add and !shl.
//   TASK 2: Add the missing `Cyan` color (0, 255, 255).
//
// Check your work by running ../run.sh -- Cyan should appear with the right
// Packed value, and you never had to touch any C++ or any switch statement.
//
//===----------------------------------------------------------------------===//

class Color<string name, int r, int g, int b> {
  string Name = name;
  int R = r;
  int G = g;
  int B = b;

  // TASK 1: add the derived `Packed` field here.

  bit IsPrimary = 0;
}

class PrimaryColor<string name, int r, int g, int b> : Color<name, r, g, b> {
  let IsPrimary = 1;
}

def Red   : PrimaryColor<"Red",     255,   0,   0>;
def Green : PrimaryColor<"Green",     0, 255,   0>;
def Blue  : PrimaryColor<"Blue",      0,   0, 255>;
def White : Color       <"White",   255, 255, 255>;
// TASK 2: add Cyan here.

#!/usr/bin/env python3
"""A ~10-line "TableGen backend" written in Python.

It reads the JSON that `llvm-tblgen --dump-json` produces and prints one line
per color. The point: because TableGen exposes its records as data, ANY tool can
consume them -- here Python, but in a real compiler it is a C++ backend inside
llvm-tblgen/mlir-tblgen. An X-macro's data can only ever be read by the C
preprocessor, at compile time, in one translation unit.

Usage:  llvm-tblgen --dump-json colors.td | python3 show.py
"""
import json
import sys

records = json.load(sys.stdin)
for name in records["!instanceof"].get("Color", []):
    rec = records[name]
    packed = rec.get("Packed")
    packed_str = f"#{packed:06X}" if isinstance(packed, int) else "<missing: do TASK 1>"
    print(f"  {name:8} {packed_str}  primary={rec['IsPrimary']}")

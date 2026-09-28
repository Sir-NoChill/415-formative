# ex06 -- An ANTLR frontend: a homogeneous parse tree, typed handles over it

## 1. What this example teaches

Some of you will build your parsing frontend with
**ANTLR** rather than by hand. This example shows
exactly what ANTLR hands you, and where it sits on
Lesson 3's homogeneous/heterogeneous axis. It drives
two objectives:

- **Define a homogeneous AST** (objective 1) -- ANTLR's
  parse tree *is* one.
- **Compare and contrast homo vs. hetero** (objective
  4) -- the same language is parsed here into a
     homogeneous tree and, in the ParserByHand lab,
     into a heterogeneous one.

The tiny language (`Expr.g4`) is assignments over
integer arithmetic: `x = 5; y = 2 * (x + 1); z = 17 %
4;`.

## 2. What ANTLR generated

`run.sh` runs the ANTLR tool over `Expr.g4` with
`-Dlanguage=Cpp -visitor -no-listener -package calc`.
From the grammar it emits, into `build/<variant>/gen/`:

- `ExprLexer` -- turns text into tokens (`ID`, `INT`,
  `*`, `%`, `;`, ...).
- `ExprParser` -- builds the parse tree; also holds the
  nested **`*Context` classes**, one per labeled
  alternative.
- `ExprBaseVisitor` -- a visitor base with one `visitX`
  per labeled alternative, each defaulting to "visit my
  children."

The **labeled alternatives** are the `# Name` tags in
the grammar:

```antlr
expr : expr op=('*'|'/'|'%') expr   # MulDiv
     | expr op=('+'|'-') expr       # AddSub
     | '(' expr ')'                 # Paren
     | ID                           # Var
     | INT                          # Int
     ;
```

Each tag makes ANTLR generate a typed context class --
`MulDivContext`, `AddSubContext`, `ParenContext`,
`VarContext`, `IntContext` -- and a matching
`visitMulDiv`, `visitAddSub`, ... Without the labels
you would get one nameless `ExprContext` and have to
index children positionally. The labels are what give
you *typed handles* onto an otherwise untyped tree.

## 3. The tree is homogeneous -- read it

`main.cpp` parses the program, then prints the tree
with `tree->toStringTree(&parser, true)`. Verified
output of `./run.sh`:

```
Homogeneous parse tree (every node is a generic RuleContext):
  (prog 
        (stat x = 
            (expr 5) ;) 
        (stat y = 
            (expr 
                (expr 2) * 
                (expr ( 
                    (expr 
                        (expr x) + 
                        (expr 1)) ))) ;) 
        (stat z = 
            (expr 
                (expr 17) % 
                (expr 4)) ;) <EOF>)
```

Every interior node prints as a bare `(expr ...)` or
`(stat ...)`. The multiply `(expr (expr 2) * (expr
...))`, the add `(expr (expr x) + (expr 1))`, the paren
group, and the variable reference `(expr x)` are all
the **same static C++ type** --
`antlr4::tree::RuleContext`. Nothing in the *storage*
distinguishes a multiply from an add; only the runtime
rule index (and, for us, the operator token) does. That
is the definition of a **homogeneous** tree.

## 4. The visitor is the walk

Evaluation is a pass over that tree. `Eval` subclasses
`calc::ExprBaseVisitor` and overrides one method per
alternative. Because the labeled alternatives gave us
typed contexts, each override pulls out *named* parts
with compile-time checking:

```cpp
std::any visitMulDiv(calc::ExprParser::MulDivContext *ctx) override {
  long l = std::any_cast<long>(visit(ctx->expr(0)));
  long r = std::any_cast<long>(visit(ctx->expr(1)));
  const std::string op = ctx->op->getText();
  if (op == "*") return l * r;
  if (op == "/") return l / r;
  return l % r;
}
```

`ctx->expr(0)` / `ctx->expr(1)` are the two operand
sub-trees; `ctx->op` is the operator token. This is the
**normalized heterogeneous** feel Lesson 3 describes:
typed method + typed context over a homogeneous tree.
Verified output of `./run.sh`:

```
Evaluating via the generated visitor:
  x = 5
  y = 12
  z = 1
```

## 5. The exercise, explained

The starter grammar's `MulDiv` alternative omits `%`:

```antlr
expr op=('*'|'/') expr   # MulDiv   // starter: no '%'
```

Because `%` is not a token, the lexer cannot even
recognise it. Run `./run.sh starter` and the damage is
visible -- verified output:

```
line 1:31 token recognition error at: '%'
line 1:33 extraneous input '4' expecting ';'
```

The `z` statement mis-parses (`(stat z = (expr 17) 4
;)` -- the `% 4` is lost) and `z` evaluates to `17`
instead of `1`. The fix is one token:

```antlr
expr op=('*'|'/'|'%') expr   # MulDiv   // solution
```

Rerun `./run.sh starter` (or just `./run.sh`) and `z =
1` again. Note what you did **not** edit: `main.cpp` is
byte-for-byte identical between `starter/` and
`solution/`. `%` routes to the *same* labeled
alternative, `MulDiv`, and `visitMulDiv` already
returns `l % r`. Widening an operator's token set adds
no node type, so it never ripples into the visitor -- a
grammar-only change.

## 6. The two poles, one language

The CMPUT 415 **ANTLR** lab
(`415-labs/ANTLR/demo-finished/`) parses a small
"Config" language -- its `grammar/Config.g4` uses the
very same labeled-alternative trick (`# mulDivExpr`, `#
idExpr`, ...), and `ConfigEvaluator` subclasses
`ConfigBaseVisitor` exactly as `Eval` does here. Its
`main.cpp` even grabs the root the same way:
`antlr4::tree::ParseTree *tree = parser.config();`.
That is a **homogeneous** parse tree.

The **ParserByHand** lab
(`415-labs/ParserByHand/demo-finished/`) parses the
*same Config language* -- but into a **heterogeneous**
AST: real classes `BinaryExpr`, `IdExpr`, `Assignment`,
each with named, typed fields and LLVM-style
`isa<>`/`dyn_cast<>`. Same language, opposite
representations. Reading the two side by side is the
fastest way to feel objective 4: the homogeneous tree
is cheap to produce and walk generically but has no
compile-time field safety; the heterogeneous AST is the
reverse.

## 7. The bridge to Lesson 4

A real compiler rarely stops at the parse tree. It
commonly runs **one more pass** -- a visitor whose job
is to *build a heterogeneous AST* from the homogeneous
parse tree (an `Eval`-shaped walk that, instead of
computing a value, constructs a `BinaryExpr` node).
That visitor is the seam between an ANTLR frontend and
the hand-built ASTs of Lesson 4, where you design,
decorate, and extend such a tree yourself.

To see the real thing, open
`415-labs/ANTLR/demo-finished/`: `grammar/Config.g4`,
`src/main.cpp`, `include/ConfigEvaluator.h`, and
`src/ConfigEvaluator.cpp`.

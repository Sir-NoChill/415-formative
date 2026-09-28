// ex07 - Designing an AST from scratch, and a semantic-analysis pass over it.
//
// "Mini" is a tiny imperative language:
//
//     let a = 3;              // declaration (introduces a name)
//     a = a + 1;             // assignment (name must already exist)
//     if (a < 10) { ... }    // condition must be Bool
//     while (a < 10) { ... }
//     print a;
//
// This file shows a *well-designed* heterogeneous AST for Mini and a real
// semantic pass built on it. The design choices that make the pass small and
// its diagnostics precise are called out with (1)..(5) below; ex07's README
// contrasts them against a deliberately naive AST you are asked to critique.
//
// The pass does two classic analyses in one walk:
//   - name resolution: bind each use/assignment to a declaration in scope,
//     reporting use-before-declaration and redeclaration;
//   - type checking: arithmetic needs Int operands; a condition needs Bool.
//
//   $CXX -std=c++17 -Wall -Wextra solution/main.cpp -o ex07 && ./ex07
//
//===----------------------------------------------------------------------===//

#include <cstdio>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace mini {

// (2) A source location on every node -- so diagnostics can point at real code.
struct SourceLoc {
  int line = 0, col = 0;
};

enum class Type { Int, Bool, Error };
static const char *typeName(Type t) {
  return t == Type::Int    ? "int"
         : t == Type::Bool ? "bool"
                           : "<error>";
}

//===----------------------------------------------------------------------===//
// (1) The node-kind table is an X-MACRO -- the Lesson 1 idiom, applied to an
// AST. `mini_nodes.def` is the single source of truth; we #include it three
// times to generate the Kind enum, the category-range predicate isExprKind(),
// and a kindName() debug string, so the three can never drift. This is exactly
// how ParserByHand generates its AST (ASTNodes.def) and how gazc does it with
// TableGen (ASTNodes.td). Expressions are listed contiguously and last in the
// table, so isExprKind() is an O(1) range check, not a chain of comparisons.
//===----------------------------------------------------------------------===//
enum class Kind {
#define MINI_NODE(NAME) NAME,
#include "mini_nodes.def"
};

// isExprKind() (and, symmetrically, any other category) generated from the
// range.
#define MINI_NODE_RANGE(BASE, FIRST, LAST)            \
  static bool is##BASE##Kind(Kind k) {                \
    return k >= Kind::FIRST && k <= Kind::LAST;       \
  }
#include "mini_nodes.def"

// A second consumer of the SAME table: a debug name for each kind, via
// stringize. (Marked maybe_unused so it needn't be called to demonstrate the
// point.)
#define MINI_NODE(NAME)                               \
  case Kind::NAME:                                    \
    return #NAME;
[[maybe_unused]] static const char *kindName(Kind k) {
  switch (k) {
#include "mini_nodes.def"
  }
  return "?";
}

struct Node {
  const Kind kind;
  SourceLoc loc;
  virtual ~Node() = default;

protected:
  Node(Kind k, SourceLoc l) : kind(k), loc(l) {}
};

// LLVM-style RTTI (see ParserByHand's AST.h; here inline for one file).
template <class To> bool isa(Node *n) {
  return To::classof(n);
}
template <class To> To *dyn_cast(Node *n) {
  return isa<To>(n) ? static_cast<To *>(n) : nullptr;
}
template <class To> To *cast(Node *n) {
  return static_cast<To *>(n);
}

//===----------------------------------------------------------------------===//
// (3) Expr and Stmt are distinct categories, so the type system itself forbids
// putting a statement where an expression belongs. (4) Expr carries a `type`
// annotation slot the checker fills in.
//===----------------------------------------------------------------------===//
struct Expr : Node {
  Type type = Type::Error; // (4) decorated by Sema
  Expr(Kind k, SourceLoc l) : Node(k, l) {}
  static bool classof(Node *n) {
    return isExprKind(n->kind);
  }
};
struct Stmt : Node {
  using Node::Node;
  static bool classof(Node *n) {
    return !isExprKind(n->kind);
  }
};

struct IntLit : Expr {
  long value;
  IntLit(SourceLoc l, long v)
      : Expr(Kind::IntLit, l), value(v) {}
  static bool classof(Node *n) {
    return n->kind == Kind::IntLit;
  }
};
struct BoolLit : Expr {
  bool value;
  BoolLit(SourceLoc l, bool v)
      : Expr(Kind::BoolLit, l), value(v) {}
  static bool classof(Node *n) {
    return n->kind == Kind::BoolLit;
  }
};
struct VarRef : Expr {
  std::string name;
  VarRef(SourceLoc l, std::string n)
      : Expr(Kind::VarRef, l), name(std::move(n)) {}
  static bool classof(Node *n) {
    return n->kind == Kind::VarRef;
  }
};
struct Binary : Expr {
  std::string op; // "+","-","*","/","<","=="
  Expr *
      lhs; // (5) named, typed children -- no kids[0]/kids[1] convention
  Expr *rhs;
  Binary(SourceLoc l, std::string o, Expr *a, Expr *b)
      : Expr(Kind::Binary, l), op(std::move(o)),
        lhs(a), rhs(b) {}
  static bool classof(Node *n) {
    return n->kind == Kind::Binary;
  }
};

struct Let : Stmt {
  std::string name;
  Expr *init;
  Let(SourceLoc l, std::string n, Expr *e)
      : Stmt(Kind::Let, l), name(std::move(n)),
        init(e) {}
  static bool classof(Node *n) {
    return n->kind == Kind::Let;
  }
};
struct Assign : Stmt {
  std::string name;
  Expr *value;
  Assign(SourceLoc l, std::string n, Expr *e)
      : Stmt(Kind::Assign, l), name(std::move(n)),
        value(e) {}
  static bool classof(Node *n) {
    return n->kind == Kind::Assign;
  }
};
struct Block : Stmt {
  std::vector<Stmt *> stmts;
  explicit Block(SourceLoc l) : Stmt(Kind::Block, l) {}
  static bool classof(Node *n) {
    return n->kind == Kind::Block;
  }
};
struct If : Stmt {
  Expr *cond;
  Block *thenB;
  Block *elseB; // may be null
  If(SourceLoc l, Expr *c, Block *t, Block *e)
      : Stmt(Kind::If, l), cond(c), thenB(t),
        elseB(e) {}
  static bool classof(Node *n) {
    return n->kind == Kind::If;
  }
};
struct While : Stmt {
  Expr *cond;
  Block *body;
  While(SourceLoc l, Expr *c, Block *b)
      : Stmt(Kind::While, l), cond(c), body(b) {}
  static bool classof(Node *n) {
    return n->kind == Kind::While;
  }
};
struct Print : Stmt {
  Expr *value;
  Print(SourceLoc l, Expr *e)
      : Stmt(Kind::Print, l), value(e) {}
  static bool classof(Node *n) {
    return n->kind == Kind::Print;
  }
};

// Arena: owns every node; the tree holds raw pointers (Clang's ASTContext
// idea).
struct Arena {
  std::vector<std::unique_ptr<Node>> pool;
  template <class T, class... A> T *make(A &&...args) {
    auto p =
        std::make_unique<T>(std::forward<A>(args)...);
    T *raw = p.get();
    pool.push_back(std::move(p));
    return raw;
  }
};

//===----------------------------------------------------------------------===//
// The semantic-analysis pass.
//===----------------------------------------------------------------------===//
class Sema {
  // A stack of scopes; each maps a name to its declared type.
  std::vector<std::unordered_map<std::string, Type>>
      scopes;
  int errors = 0;

  void error(SourceLoc l, const std::string &msg) {
    std::printf("  %d:%d: error: %s\n", l.line, l.col,
                msg.c_str());
    ++errors;
  }
  Type *lookup(const std::string &n) {
    for (auto it = scopes.rbegin();
         it != scopes.rend(); ++it) {
      auto f = it->find(n);
      if (f != it->end())
        return &f->second;
    }
    return nullptr;
  }

public:
  int run(Block &program) {
    checkBlock(program);
    return errors;
  }

private:
  void checkBlock(Block &b) {
    scopes.emplace_back();
    for (Stmt *s : b.stmts)
      checkStmt(s);
    scopes.pop_back();
  }

  void checkStmt(Stmt *s) {
    switch (s->kind) {
    case Kind::Let: {
      auto *d = cast<Let>(s);
      Type t = checkExpr(d->init);
      if (scopes.back().count(d->name))
        error(d->loc,
              "redeclaration of '" + d->name + "'");
      else
        scopes.back()[d->name] =
            t; // name enters scope AFTER its initializer
      break;
    }
    case Kind::Assign: {
      auto *a = cast<Assign>(s);
      Type vt = checkExpr(a->value);
      Type *decl = lookup(a->name);
      if (!decl)
        error(a->loc, "assignment to undeclared '" +
                          a->name + "'");
      else if (*decl != vt && vt != Type::Error)
        error(a->loc, "cannot assign " +
                          std::string(typeName(vt)) +
                          " to '" + a->name +
                          "' of type " +
                          typeName(*decl));
      break;
    }
    case Kind::If: {
      auto *i = cast<If>(s);
      if (checkExpr(i->cond) != Type::Bool)
        error(i->cond->loc,
              "if condition must be bool");
      checkBlock(*i->thenB);
      if (i->elseB)
        checkBlock(*i->elseB);
      break;
    }
    case Kind::While: {
      auto *w = cast<While>(s);
      if (checkExpr(w->cond) != Type::Bool)
        error(w->cond->loc,
              "while condition must be bool");
      checkBlock(*w->body);
      break;
    }
    case Kind::Print:
      checkExpr(cast<Print>(s)->value);
      break;
    case Kind::Block:
      checkBlock(*cast<Block>(s));
      break;
    default:
      break;
    }
  }

  // Returns the expression's type and records it in the node's `type` slot.
  Type checkExpr(Expr *e) {
    switch (e->kind) {
    case Kind::IntLit:
      return e->type = Type::Int;
    case Kind::BoolLit:
      return e->type = Type::Bool;
    case Kind::VarRef: {
      auto *v = cast<VarRef>(e);
      if (Type *t = lookup(v->name))
        return e->type = *t;
      error(v->loc,
            "use of undeclared '" + v->name + "'");
      return e->type = Type::Error;
    }
    case Kind::Binary: {
      auto *b = cast<Binary>(e);
      Type l = checkExpr(b->lhs),
           r = checkExpr(b->rhs);
      bool compare = (b->op == "<" || b->op == "==");
      if ((l != Type::Int || r != Type::Int) &&
          l != Type::Error && r != Type::Error)
        error(b->loc, "operator '" + b->op +
                          "' requires int operands");
      return e->type =
                 compare ? Type::Bool : Type::Int;
    }
    default:
      return Type::Error;
    }
  }
};

} // namespace mini

//===----------------------------------------------------------------------===//
// Driver. We build two Mini programs by hand (a parser is ex06's job) with
// explicit source locations, so the diagnostics look exactly like a real
// compiler's line:col messages.
//===----------------------------------------------------------------------===//
using namespace mini;

// A correct program:
//   1: let a = 3;
//   2: let b = a + 4;
//   3: if (b < 20) { print b; }
static Block *buildGood(Arena &a) {
  auto *prog = a.make<Block>(SourceLoc{1, 1});
  prog->stmts.push_back(
      a.make<Let>(SourceLoc{1, 1}, "a",
                  a.make<IntLit>(SourceLoc{1, 9}, 3)));
  prog->stmts.push_back(a.make<Let>(
      SourceLoc{2, 1}, "b",
      a.make<Binary>(
          SourceLoc{2, 9}, "+",
          a.make<VarRef>(SourceLoc{2, 9}, "a"),
          a.make<IntLit>(SourceLoc{2, 13}, 4))));
  auto *then = a.make<Block>(SourceLoc{3, 14});
  then->stmts.push_back(a.make<Print>(
      SourceLoc{3, 16},
      a.make<VarRef>(SourceLoc{3, 22}, "b")));
  prog->stmts.push_back(a.make<If>(
      SourceLoc{3, 1},
      a.make<Binary>(
          SourceLoc{3, 5}, "<",
          a.make<VarRef>(SourceLoc{3, 5}, "b"),
          a.make<IntLit>(SourceLoc{3, 9}, 20)),
      then, nullptr));
  return prog;
}

// A program with three deliberate bugs:
//   1: let a = 3;
//   2: c = a + 1;        // (A) assignment to undeclared 'c'
//   3: print a + d;      // (B) use of undeclared 'd'
//   4: if (a + 1) { }    // (C) if condition is int, not bool
static Block *buildBad(Arena &a) {
  auto *prog = a.make<Block>(SourceLoc{1, 1});
  prog->stmts.push_back(
      a.make<Let>(SourceLoc{1, 1}, "a",
                  a.make<IntLit>(SourceLoc{1, 9}, 3)));
  prog->stmts.push_back(a.make<Assign>(
      SourceLoc{2, 1}, "c",
      a.make<Binary>(
          SourceLoc{2, 5}, "+",
          a.make<VarRef>(SourceLoc{2, 5}, "a"),
          a.make<IntLit>(SourceLoc{2, 9}, 1))));
  prog->stmts.push_back(a.make<Print>(
      SourceLoc{3, 1},
      a.make<Binary>(
          SourceLoc{3, 7}, "+",
          a.make<VarRef>(SourceLoc{3, 7}, "a"),
          a.make<VarRef>(SourceLoc{3, 11}, "d"))));
  auto *empty = a.make<Block>(SourceLoc{4, 14});
  prog->stmts.push_back(a.make<If>(
      SourceLoc{4, 1},
      a.make<Binary>(
          SourceLoc{4, 5}, "+",
          a.make<VarRef>(SourceLoc{4, 5}, "a"),
          a.make<IntLit>(SourceLoc{4, 9}, 1)),
      empty, nullptr));
  return prog;
}

int main() {
  Arena a;

  std::printf("=== Checking the good program ===\n");
  int e1 = Sema{}.run(*buildGood(a));
  std::printf(e1 == 0 ? "  OK: no semantic errors.\n"
                      : "");

  std::printf(
      "\n=== Checking the buggy program ===\n");
  int e2 = Sema{}.run(*buildBad(a));
  std::printf("  %d semantic error(s).\n", e2);

  return e2 == 0 ? 0 : 1;
}

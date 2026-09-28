// ex08 - Adapting an existing AST: adding a `for` loop to Mini.
//
// You inherit the Mini AST from ex07 (Let/Assign/If/While/Print/Block +
// IntLit/VarRef/Binary) and a small tree-walking interpreter. The task is to
// add
//
//     for (init; cond; step) body
//
// There are two honest strategies, and choosing between them IS the lesson:
//
//   A. Add a first-class `For` NODE. New expressive power in the tree, but
//   every
//      pass that walks statements must grow a `For` case (here: the
//      interpreter; in a real compiler also the type checker, the
//      pretty-printer, lowering).
//
//   B. DESUGAR `for` into nodes that already exist:
//        for (init; cond; step) body  ==>  { init; while (cond) { body; step; }
//        }
//      Zero passes change -- but the tree no longer records that the user wrote
//      a `for`, so `for`-specific diagnostics become impossible.
//
// This file implements BOTH and runs the same loop each way to show they agree.
//
//   g++ -std=c++17 -Wall -Wextra solution/main.cpp -o ex08 && ./ex08
//
//===----------------------------------------------------------------------===//

#include <cstdio>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace mini {

// The node-kind table is an X-macro (`mini_nodes.def`, the Lesson 1 idiom). The
// `For` kind that strategy A needs was added there as one row; the enum below
// is generated from it, as is kindName().
enum class Kind {
#define MINI_NODE(NAME) NAME,
#include "mini_nodes.def"
};

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
  virtual ~Node() = default;

protected:
  explicit Node(Kind k) : kind(k) {}
};
template <class To> bool isa(Node *n) {
  return To::classof(n);
}
template <class To> To *dyn_cast(Node *n) {
  return isa<To>(n) ? static_cast<To *>(n) : nullptr;
}
template <class To> To *cast(Node *n) {
  return static_cast<To *>(n);
}

struct Expr : Node {
  using Node::Node;
};
struct Stmt : Node {
  using Node::Node;
};

struct IntLit : Expr {
  long value;
  explicit IntLit(long v)
      : Expr(Kind::IntLit), value(v) {}
  static bool classof(Node *n) {
    return n->kind == Kind::IntLit;
  }
};
struct VarRef : Expr {
  std::string name;
  explicit VarRef(std::string n)
      : Expr(Kind::VarRef), name(std::move(n)) {}
  static bool classof(Node *n) {
    return n->kind == Kind::VarRef;
  }
};
struct Binary : Expr {
  std::string op;
  Expr *lhs, *rhs;
  Binary(std::string o, Expr *a, Expr *b)
      : Expr(Kind::Binary), op(std::move(o)), lhs(a),
        rhs(b) {}
  static bool classof(Node *n) {
    return n->kind == Kind::Binary;
  }
};

struct Let : Stmt {
  std::string name;
  Expr *init;
  Let(std::string n, Expr *e)
      : Stmt(Kind::Let), name(std::move(n)), init(e) {}
  static bool classof(Node *n) {
    return n->kind == Kind::Let;
  }
};
struct Assign : Stmt {
  std::string name;
  Expr *value;
  Assign(std::string n, Expr *e)
      : Stmt(Kind::Assign), name(std::move(n)),
        value(e) {}
  static bool classof(Node *n) {
    return n->kind == Kind::Assign;
  }
};
struct Block : Stmt {
  std::vector<Stmt *> stmts;
  Block() : Stmt(Kind::Block) {}
  static bool classof(Node *n) {
    return n->kind == Kind::Block;
  }
};
struct While : Stmt {
  Expr *cond;
  Block *body;
  While(Expr *c, Block *b)
      : Stmt(Kind::While), cond(c), body(b) {}
  static bool classof(Node *n) {
    return n->kind == Kind::While;
  }
};
struct Print : Stmt {
  Expr *value;
  explicit Print(Expr *e)
      : Stmt(Kind::Print), value(e) {}
  static bool classof(Node *n) {
    return n->kind == Kind::Print;
  }
};

// STRATEGY A: the new first-class node.
struct For : Stmt {
  Stmt *init;
  Expr *cond;
  Stmt *step;
  Block *body;
  For(Stmt *i, Expr *c, Stmt *s, Block *b)
      : Stmt(Kind::For), init(i), cond(c), step(s),
        body(b) {}
  static bool classof(Node *n) {
    return n->kind == Kind::For;
  }
};

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
// A tree-walking interpreter (one flat environment, enough for the demo).
//===----------------------------------------------------------------------===//
struct Interp {
  std::map<std::string, long> env;

  long eval(Expr *e) {
    if (auto *i = dyn_cast<IntLit>(e))
      return i->value;
    if (auto *v = dyn_cast<VarRef>(e))
      return env[v->name];
    auto *b = cast<Binary>(e);
    long l = eval(b->lhs), r = eval(b->rhs);
    if (b->op == "+")
      return l + r;
    if (b->op == "-")
      return l - r;
    if (b->op == "*")
      return l * r;
    if (b->op == "/")
      return l / r;
    if (b->op == "<")
      return l < r ? 1 : 0;
    if (b->op == "==")
      return l == r ? 1 : 0;
    return 0;
  }

  void exec(Stmt *s) {
    switch (s->kind) {
    case Kind::Let: {
      auto *d = cast<Let>(s);
      env[d->name] = eval(d->init);
      break;
    }
    case Kind::Assign: {
      auto *a = cast<Assign>(s);
      env[a->name] = eval(a->value);
      break;
    }
    case Kind::Print: {
      auto *p = cast<Print>(s);
      std::printf("%ld\n", eval(p->value));
      break;
    }
    case Kind::Block: {
      for (Stmt *c : cast<Block>(s)->stmts)
        exec(c);
      break;
    }
    case Kind::While: {
      auto *w = cast<While>(s);
      while (eval(w->cond))
        exec(w->body);
      break;
    }
    case Kind::
        For: { // STRATEGY A: interpret the For node directly.
      auto *f = cast<For>(s);
      for (exec(f->init); eval(f->cond); exec(f->step))
        exec(f->body);
      break;
    }
    default:
      break;
    }
  }
};

//===----------------------------------------------------------------------===//
// STRATEGY B: desugar a For into existing nodes. Returns a Block equivalent to
//     { init; while (cond) { <body stmts>; step; } }
// Note it allocates only Block/While -- nodes the interpreter already handled
// before `For` ever existed.
//===----------------------------------------------------------------------===//
Stmt *desugarFor(Arena &a, For *f) {
  auto *innerBody = a.make<Block>();
  for (Stmt *s : f->body->stmts)
    innerBody->stmts.push_back(s);
  innerBody->stmts.push_back(f->step);

  auto *loop = a.make<While>(f->cond, innerBody);

  auto *outer = a.make<Block>();
  outer->stmts.push_back(f->init);
  outer->stmts.push_back(loop);
  return outer;
}

} // namespace mini

using namespace mini;

// Build:  for (let i = 1; i < 6; i = i + 1) { sum = sum + i; }
static For *buildForLoop(Arena &a) {
  auto *body = a.make<Block>();
  body->stmts.push_back(a.make<Assign>(
      "sum", a.make<Binary>("+", a.make<VarRef>("sum"),
                            a.make<VarRef>("i"))));
  return a.make<For>(
      a.make<Let>("i", a.make<IntLit>(1)), // init
      a.make<Binary>("<", a.make<VarRef>("i"),
                     a.make<IntLit>(6)), // cond
      a.make<Assign>(
          "i",
          a.make<Binary>("+", a.make<VarRef>("i"),
                         a.make<IntLit>(1))), // step
      body);                                  // body
}

int main() {
  Arena a;

  // --- Strategy A: interpret the For node directly. ---
  {
    Interp ip;
    ip.env["sum"] = 0;
    std::printf(
        "Strategy A (first-class For node): sum = ");
    ip.exec(buildForLoop(a));
    ip.exec(a.make<Print>(a.make<VarRef>("sum")));
  }

  // --- Strategy B: desugar to while, then interpret with NO For support. ---
  {
    Interp ip;
    ip.env["sum"] = 0;
    Stmt *desugared = desugarFor(a, buildForLoop(a));
    std::printf(
        "Strategy B (desugared to while): sum = ");
    ip.exec(desugared);
    ip.exec(a.make<Print>(a.make<VarRef>("sum")));
  }

  std::printf(
      "\nBoth strategies compute 1+2+3+4+5 = 15.\n");
  return 0;
}

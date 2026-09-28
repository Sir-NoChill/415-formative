// ex05 - The SAME tiny program, represented two ways.
//
// Lesson 3 is about how an AST is *typed*. This file makes the two poles
// concrete by building the exact same three assignments
//
//     x = 5;
//     z = 3 * (4 + 5);
//     y = 2 * (x + 1);
//
// twice: once as a HOMOGENEOUS tree (one node type, a runtime tag, a generic
// child list) and once as a HETEROGENEOUS tree (a typed class per construct
// with named fields and LLVM-style RTTI). Then we run the SAME three passes --
// pretty-print, constant-fold, evaluate -- over each, as walks over the tree,
// and show the outputs are identical.
//
// Read both halves side by side. The point is not that one is shorter; it is
// *where the risk lives*: the homogeneous walks index into kids[] and switch on
// a tag (typos compile, arity is a convention); the heterogeneous walks name
// their fields and dyn_cast (a wrong field access does not compile).
//
//   g++ -std=c++17 -Wall -Wextra solution/main.cpp -o ex05 && ./ex05
//
//===----------------------------------------------------------------------===//

#include <cstdio>
#include <map>
#include <memory>
#include <string>
#include <vector>

//===----------------------------------------------------------------------===//
// PART A -- Homogeneous AST: one Node type for the whole language.
//===----------------------------------------------------------------------===//
namespace homo {

enum Kind { K_Assign, K_Int, K_Var, K_Add, K_Sub, K_Mul, K_Div };

// Every construct is a Node. Meaning lives in `kind`; payload is overloaded
// (ival for K_Int, name for K_Var/K_Assign); children are a generic list whose
// expected length is a *convention* per kind, not something the type enforces.
struct Node {
  Kind kind;
  long ival = 0;            // valid when kind == K_Int
  std::string name;         // valid when kind == K_Var or K_Assign
  std::vector<Node *> kids; // Assign: [value]; binops: [lhs, rhs]
};

// A trivial arena so we never worry about freeing individual nodes.
struct Arena {
  std::vector<std::unique_ptr<Node>> pool;
  Node *make(Node n) {
    pool.push_back(std::make_unique<Node>(std::move(n)));
    return pool.back().get();
  }
};

bool isBinop(Kind k) { return k >= K_Add && k <= K_Div; }
char opChar(Kind k) {
  return k == K_Add ? '+' : k == K_Sub ? '-' : k == K_Mul ? '*' : '/';
}

// Pass 1: pretty-print as an s-expression. A walk that switches on the tag.
void print(Node *n) {
  switch (n->kind) {
  case K_Assign:
    std::printf("(= %s ", n->name.c_str());
    print(n->kids[0]); // kids[0] is "the value" -- by convention
    std::printf(")");
    break;
  case K_Int:
    std::printf("%ld", n->ival);
    break;
  case K_Var:
    std::printf("%s", n->name.c_str());
    break;
  case K_Add:
  case K_Sub:
  case K_Mul:
  case K_Div:
    std::printf("(%c ", opChar(n->kind));
    print(n->kids[0]); // lhs -- by convention
    std::printf(" ");
    print(n->kids[1]); // rhs -- by convention
    std::printf(")");
    break;
  }
}

// Pass 2: constant-fold. Rewrites a binop of two K_Int children into one K_Int.
Node *fold(Arena &a, Node *n) {
  if (isBinop(n->kind)) {
    Node *l = fold(a, n->kids[0]);
    Node *r = fold(a, n->kids[1]);
    if (l->kind == K_Int && r->kind == K_Int) {
      long v = 0;
      switch (n->kind) {
      case K_Add:
        v = l->ival + r->ival;
        break;
      case K_Sub:
        v = l->ival - r->ival;
        break;
      case K_Mul:
        v = l->ival * r->ival;
        break;
      case K_Div:
        v = l->ival / r->ival;
        break;
      default:
        break;
      }
      return a.make(Node{K_Int, v, "", {}});
    }
    n->kids[0] = l;
    n->kids[1] = r;
  }
  return n;
}

// Pass 3: evaluate against an environment of already-assigned variables.
long eval(Node *n, std::map<std::string, long> &env) {
  switch (n->kind) {
  case K_Int:
    return n->ival;
  case K_Var:
    return env[n->name];
  case K_Add:
    return eval(n->kids[0], env) + eval(n->kids[1], env);
  case K_Sub:
    return eval(n->kids[0], env) - eval(n->kids[1], env);
  case K_Mul:
    return eval(n->kids[0], env) * eval(n->kids[1], env);
  case K_Div:
    return eval(n->kids[0], env) / eval(n->kids[1], env);
  case K_Assign:
    return eval(n->kids[0], env);
  }
  return 0;
}

// Build:  x = 5;  z = 3 * (4 + 5);  y = 2 * (x + 1);
std::vector<Node *> buildProgram(Arena &a) {
  auto num = [&](long v) { return a.make(Node{K_Int, v, "", {}}); };
  auto var = [&](std::string s) {
    return a.make(Node{K_Var, 0, std::move(s), {}});
  };
  auto bin = [&](Kind k, Node *l, Node *r) {
    return a.make(Node{k, 0, "", {l, r}});
  };
  auto asn = [&](std::string s, Node *v) {
    return a.make(Node{K_Assign, 0, std::move(s), {v}});
  };

  return {
      asn("x", num(5)),
      asn("z", bin(K_Mul, num(3), bin(K_Add, num(4), num(5)))),
      asn("y", bin(K_Mul, num(2), bin(K_Add, var("x"), num(1)))),
  };
}

void run() {
  Arena a;
  std::map<std::string, long> env;
  std::printf("--- Homogeneous (one Node type, runtime tag) ---\n");
  for (Node *stmt : buildProgram(a)) {
    stmt->kids[0] = fold(a, stmt->kids[0]); // constant-fold the value
    long v = eval(stmt, env);
    env[stmt->name] = v;
    std::printf("  ");
    print(stmt);
    std::printf("   ==> %s = %ld\n", stmt->name.c_str(), v);
  }
}

} // namespace homo

//===----------------------------------------------------------------------===//
// PART B -- Heterogeneous AST: a typed class per construct + LLVM-style RTTI.
//===----------------------------------------------------------------------===//
namespace hetero {

enum class Kind { IntLit, VarRef, Binary, Assign };

// Shared base. `kind` drives isa<>/dyn_cast<>; a real AST would also carry a
// SourceRange here (see ex07). Subclasses add exactly the fields they need.
struct Node {
  const Kind kind;
  virtual ~Node() = default;

protected:
  explicit Node(Kind k) : kind(k) {}
};

// Minimal hand-rolled LLVM-style RTTI (mirrors ParserByHand's AST.h).
template <class To> bool isa(Node *n) { return To::classof(n); }
template <class To> To *dyn_cast(Node *n) {
  return isa<To>(n) ? static_cast<To *>(n) : nullptr;
}
template <class To> To *cast(Node *n) { return static_cast<To *>(n); }

struct Expr : Node {
  using Node::Node;
};

struct IntLit : Expr {
  long value;
  explicit IntLit(long v) : Expr(Kind::IntLit), value(v) {}
  static bool classof(Node *n) { return n->kind == Kind::IntLit; }
};

struct VarRef : Expr {
  std::string name;
  explicit VarRef(std::string n) : Expr(Kind::VarRef), name(std::move(n)) {}
  static bool classof(Node *n) { return n->kind == Kind::VarRef; }
};

struct Binary : Expr {
  char op;   // '+', '-', '*', '/'
  Expr *lhs; // NAMED -- no index guessing
  Expr *rhs;
  Binary(char o, Expr *l, Expr *r)
      : Expr(Kind::Binary), op(o), lhs(l), rhs(r) {}
  static bool classof(Node *n) { return n->kind == Kind::Binary; }
};

struct Assign : Node {
  std::string name;
  Expr *value;
  Assign(std::string n, Expr *v)
      : Node(Kind::Assign), name(std::move(n)), value(v) {}
  static bool classof(Node *n) { return n->kind == Kind::Assign; }
};

struct Arena {
  std::vector<std::unique_ptr<Node>> pool;
  template <class T, class... A> T *make(A &&...args) {
    auto p = std::make_unique<T>(std::forward<A>(args)...);
    T *raw = p.get();
    pool.push_back(std::move(p));
    return raw;
  }
};

// Pass 1: pretty-print. dyn_cast recovers the type; fields are named.
void print(Node *n) {
  if (auto *a = dyn_cast<Assign>(n)) {
    std::printf("(= %s ", a->name.c_str());
    print(a->value);
    std::printf(")");
  } else if (auto *b = dyn_cast<Binary>(n)) {
    std::printf("(%c ", b->op);
    print(b->lhs);
    std::printf(" ");
    print(b->rhs);
    std::printf(")");
  } else if (auto *i = dyn_cast<IntLit>(n)) {
    std::printf("%ld", i->value);
  } else if (auto *v = dyn_cast<VarRef>(n)) {
    std::printf("%s", v->name.c_str());
  }
}

// Pass 2: constant-fold two IntLit operands into one.
Expr *fold(Arena &a, Expr *e) {
  auto *b = dyn_cast<Binary>(e);
  if (!b)
    return e;
  Expr *l = fold(a, b->lhs);
  Expr *r = fold(a, b->rhs);
  auto *li = dyn_cast<IntLit>(l);
  auto *ri = dyn_cast<IntLit>(r);
  if (li && ri) {
    long v = 0;
    switch (b->op) {
    case '+':
      v = li->value + ri->value;
      break;
    case '-':
      v = li->value - ri->value;
      break;
    case '*':
      v = li->value * ri->value;
      break;
    case '/':
      v = li->value / ri->value;
      break;
    }
    return a.make<IntLit>(v);
  }
  b->lhs = l;
  b->rhs = r;
  return b;
}

// Pass 3: evaluate against an environment.
long eval(Expr *e, std::map<std::string, long> &env) {
  if (auto *i = dyn_cast<IntLit>(e))
    return i->value;
  if (auto *v = dyn_cast<VarRef>(e))
    return env[v->name];
  auto *b = cast<Binary>(e);
  long l = eval(b->lhs, env), r = eval(b->rhs, env);
  switch (b->op) {
  case '+':
    return l + r;
  case '-':
    return l - r;
  case '*':
    return l * r;
  case '/':
    return l / r;
  }
  return 0;
}

std::vector<Assign *> buildProgram(Arena &a) {
  return {
      a.make<Assign>("x", a.make<IntLit>(5)),
      a.make<Assign>("z", a.make<Binary>('*', a.make<IntLit>(3),
                                         a.make<Binary>('+', a.make<IntLit>(4),
                                                        a.make<IntLit>(5)))),
      a.make<Assign>("y",
                     a.make<Binary>('*', a.make<IntLit>(2),
                                    a.make<Binary>('+', a.make<VarRef>("x"),
                                                   a.make<IntLit>(1)))),
  };
}

void run() {
  Arena a;
  std::map<std::string, long> env;
  std::printf("--- Heterogeneous (typed class per construct + RTTI) ---\n");
  for (Assign *stmt : buildProgram(a)) {
    stmt->value = fold(a, stmt->value);
    long v = eval(stmt->value, env);
    env[stmt->name] = v;
    std::printf("  ");
    print(stmt);
    std::printf("   ==> %s = %ld\n", stmt->name.c_str(), v);
  }
}

} // namespace hetero

int main() {
  homo::run();
  std::printf("\n");
  hetero::run();
  std::printf("\nSame program, same passes, identical results -- only the "
              "typing of the tree differs.\n");
  return 0;
}

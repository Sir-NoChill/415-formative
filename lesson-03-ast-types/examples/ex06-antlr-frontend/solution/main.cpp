// ex06 - The ANTLR frontend: a HOMOGENEOUS parse tree, and a visitor over it.
//
// Some students build their parser with ANTLR instead of by hand. This example
// shows what ANTLR hands you: a parse tree whose nodes are all the same static
// type -- antlr4::tree::ParseTree / RuleContext -- i.e. a HOMOGENEOUS tree, the
// exact pole Lesson 3 introduces. We
//
//   1. parse a little program into that tree and print it with toStringTree(),
//      so you can see every node is a generic (rule ...) node; and
//   2. evaluate it with a generated visitor, so you can see how ANTLR's labeled
//      alternatives (MulDivContext, VarContext, ...) give you *typed handles*
//      onto that homogeneous tree -- the "normalized heterogeneous" middle
//      ground between ex05's two extremes.
//
// This file is IDENTICAL in starter/ and solution/. The only difference between
// the two is one token in Expr.g4 (the '%' exercise): because '%' routes to the
// same MulDiv alternative, visitMulDiv already handles it with no code change.
//
// Build & run via ./run.sh (it invokes ANTLR then $CXX); see run.sh for flags.
//
//===----------------------------------------------------------------------===//

#include <any>
#include <iostream>
#include <string>
#include <unordered_map>

#include "ExprBaseVisitor.h"
#include "ExprLexer.h"
#include "ExprParser.h"
#include "antlr4-runtime.h"

// A visitor that evaluates the tree. It subclasses the GENERATED base visitor;
// each visitX corresponds to one labeled alternative in Expr.g4. The parse tree
// itself is homogeneous, but these typed contexts let us pull out named parts
// (ctx->expr(0), ctx->op, ctx->ID()) with compile-time checking.
class Eval : public calc::ExprBaseVisitor {
  std::unordered_map<std::string, long> env;

public:
  std::any visitStat(
      calc::ExprParser::StatContext *ctx) override {
    long v = std::any_cast<long>(visit(ctx->expr()));
    env[ctx->ID()->getText()] = v;
    std::cout << "  " << ctx->ID()->getText() << " = "
              << v << "\n";
    return v;
  }
  std::any visitMulDiv(
      calc::ExprParser::MulDivContext *ctx) override {
    long l = std::any_cast<long>(visit(ctx->expr(0)));
    long r = std::any_cast<long>(visit(ctx->expr(1)));
    const std::string op = ctx->op->getText();
    if (op == "*")
      return l * r;
    if (op == "/")
      return l / r;
    return l %
           r; // handles '%' the moment the grammar produces it (the exercise)
  }
  std::any visitAddSub(
      calc::ExprParser::AddSubContext *ctx) override {
    long l = std::any_cast<long>(visit(ctx->expr(0)));
    long r = std::any_cast<long>(visit(ctx->expr(1)));
    return ctx->op->getText() == "+" ? l + r : l - r;
  }
  std::any visitParen(
      calc::ExprParser::ParenContext *ctx) override {
    return visit(ctx->expr());
  }
  std::any visitVar(
      calc::ExprParser::VarContext *ctx) override {
    return env[ctx->ID()->getText()];
  }
  std::any visitInt(
      calc::ExprParser::IntContext *ctx) override {
    return static_cast<long>(
        std::stol(ctx->INT()->getText()));
  }
};

int main(int argc, char **argv) {
  // The program to parse: argv[1] if given, else a default that mirrors ex05
  // (plus one '%' use, so the starter vs solution grammar difference shows).
  std::string src =
      argc > 1 ? argv[1]
               : "x = 5; y = 2 * (x + 1); z = 17 % 4;";

  antlr4::ANTLRInputStream input(src);
  calc::ExprLexer lexer(&input);
  antlr4::CommonTokenStream tokens(&lexer);
  calc::ExprParser parser(&tokens);
  antlr4::tree::ParseTree *tree = parser.prog();

  std::cout << "Input:  " << src << "\n\n";
  std::cout << "Homogeneous parse tree (every node is "
               "a generic RuleContext):\n";
  std::cout << "  "
            << tree->toStringTree(&parser, true)
            << "\n\n";

  std::cout
      << "Evaluating via the generated visitor:\n";
  Eval e;
  e.visit(tree);
  return 0;
}

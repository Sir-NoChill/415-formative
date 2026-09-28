// Expr.g4 - a tiny assignment/arithmetic language for the ANTLR frontend example.
//
// This is the SOLUTION grammar. It differs from starter/Expr.g4 by exactly one
// token: the '%' added to the MulDiv alternative (that is ex06's exercise).
//
// Labeled alternatives (the `# Name` tags) are the important part for Lesson 3:
// each one makes ANTLR emit a typed context class (MulDivContext, VarContext, ...)
// and a typed visitor method (visitMulDiv, visitVar, ...). That is a "normalized
// heterogeneous" middle ground -- typed handles over a fundamentally homogeneous
// ParseTree of generic RuleContext nodes.
grammar Expr;

prog : stat+ EOF ;
stat : ID '=' expr ';' ;

expr : expr op=('*'|'/'|'%') expr   # MulDiv   // '%' is the added token
     | expr op=('+'|'-') expr       # AddSub
     | '(' expr ')'                 # Paren
     | ID                           # Var
     | INT                          # Int
     ;

ID  : [a-zA-Z]+ ;
INT : [0-9]+ ;
WS  : [ \t\r\n]+ -> skip ;

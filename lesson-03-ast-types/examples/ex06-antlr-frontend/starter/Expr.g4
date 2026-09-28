// Expr.g4 - a tiny assignment/arithmetic language for the ANTLR frontend example.
//
// This is the STARTER grammar. Compared to solution/Expr.g4 it is missing one
// token: the '%' (modulo) operator in the MulDiv alternative.
//
// EX06 EXERCISE: add '%' to the MulDiv alternative's operator set, i.e. change
//     expr op=('*'|'/') expr        # MulDiv
// into
//     expr op=('*'|'/'|'%') expr    # MulDiv
// Then rerun `./run.sh starter`. Notice you do NOT touch main.cpp: because '%'
// routes to the SAME labeled alternative (MulDiv), the typed visitMulDiv method
// already handles it. Extending the token set is a grammar-only change.
grammar Expr;

prog : stat+ EOF ;
stat : ID '=' expr ';' ;

expr : expr op=('*'|'/') expr   # MulDiv   // TODO(ex06): add '%' here
     | expr op=('+'|'-') expr   # AddSub
     | '(' expr ')'             # Paren
     | ID                       # Var
     | INT                      # Int
     ;

ID  : [a-zA-Z]+ ;
INT : [0-9]+ ;
WS  : [ \t\r\n]+ -> skip ;

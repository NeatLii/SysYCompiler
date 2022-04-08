%option noyywrap nounput noinput
%option nodefault
%option yylineno

%{

#include "sysy_verify.tab.h"
#include <iostream>

int yycolumnno = 1;
#define YY_USER_ACTION                               \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumnno;                \
    yylloc.last_column = yycolumnno + yyleng - 1;    \
    yycolumnno += yyleng;

extern void yyerror(const char *format, ...);
void yylerror(YYLTYPE location, const char *format, ...);

%}

%x COMMENT

%%

"/*"                { BEGIN(COMMENT); }
<COMMENT>"*/"       { BEGIN(INITIAL); }
<COMMENT>\n         { yycolumnno = 1; }
<COMMENT>[^*]+|.    ;
"//".*\n            { yycolumnno = 1; }

"const"     { return CONST; }
"void"      { return VOID; }
"int"       { return INT; }

"if"        { return IF; }
"else"      { return ELSE; }
"while"     { return WHILE; }
"break"     { return BREAK; }
"continue"  { return CONTINUE; }
"return"    { return RETURN; }

[a-zA-Z_][a-zA-Z0-9_]*      { return IDENT; }
0[0-7]*                     { return NUMBER; }
[1-9][0-9]*                 { return NUMBER; }
0[xX][0-9a-fA-F]+           { return NUMBER; }

"||"                        { return OR; }
"&&"                        { return AND; }
"=="                        { return EQ; }
"!="                        { return NE; }
"<="                        { return LE; }
">="                        { return GE; }
[,;=+\-*/%!<>()[\]{}]       { return *yytext; }

\n              { yycolumnno = 1; }
[ \t\r]         ;
.               { yylerror(yylloc, "Lexer: unknown input %s", yytext); }

%%

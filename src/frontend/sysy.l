%option noyywrap nounput noinput
%option nodefault
%option yylineno

%{

// include the definition of YYLTYPE
#include "frontend/source_manager.h"
using YYLTYPE = ast::SourceRange;
#define YYLTYPE_IS_DECLARED 1

// manage column counts
int yycolumnno = 1;
#define YY_USER_ACTION                              \
    yylloc.begin_line = yylloc.end_line = yylineno; \
    yylloc.begin_column = yycolumnno;               \
    yylloc.end_column = yycolumnno + yyleng - 1;    \
    yycolumnno += yyleng;

// include the definition of YYTOKENTYPE
// include the definition of YYSTYPE
#include "sysy.tab.h"

extern void yyerror(const char *format, ...);
extern void yylerror(YYLTYPE location, const char *format, ...);

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

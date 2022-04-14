%option noyywrap nounput noinput
%option nodefault

%{

#include <stdlib.h>

/* Include the definition of YYLTYPE, import SourceManager. */
#include "frontend/frontend.h"

/* Location type.  */
using YYLTYPE = ast::SourceRange;
#define YYLTYPE_IS_DECLARED 1

/* Include the definitions of YYTOKENTYPE and YYSTYPE. */
#include "sysy.tab.h"

int yyline_no = 1;
int yycolumn_no = 1;
void newline();

/* Maintain counts of line and column numbers, and automatically add tokens
 * to token table.
 */
#define YY_USER_ACTION                               \
    yylloc.begin_line = yylloc.end_line = yyline_no; \
    yylloc.begin_column = yycolumn_no;               \
    yylloc.end_column = yycolumn_no + yyleng - 1;    \
    yycolumn_no += yyleng;                           \
    yylval.token = src_manager.AddToken(yytext, yylloc);

extern void yyerror(const char *format, ...);
extern void yylerror(YYLTYPE location, const char *format, ...);

%}

%x COMMENT

%%

"/*"                    { BEGIN(COMMENT); }
<COMMENT>[^*\n]*        ;
<COMMENT>\n             { newline(); }
<COMMENT>[*]+[^/\n]*    ;
<COMMENT>[*]+[/]        { BEGIN(INITIAL); }

"//".*\n            { newline(); }

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
0[0-7]*                     { yylval.integer = (int)strtol(yytext, nullptr, 8); return NUMBER; }
[1-9][0-9]*                 { yylval.integer = atoi(yytext); return NUMBER; }
0[xX][0-9a-fA-F]+           { yylval.integer = (int)strtol(yytext, nullptr, 16); return NUMBER; }

"||"                        { return OR; }
"&&"                        { return AND; }
"=="                        { return EQ; }
"!="                        { return NE; }
"<="                        { return LE; }
">="                        { return GE; }
[,;=+\-*/%!<>()[\]{}]       { return *yytext; }

\n              { newline(); }
[ \t\r]+        ;
.               { yylerror(yylloc, "Lexer: unknown input '%s'", yytext); }

%%

void newline() {
    ++yyline_no;
    yycolumn_no = 1;
}

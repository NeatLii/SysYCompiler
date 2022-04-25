#ifndef __sysycompiler_frontend_parser_h__
#define __sysycompiler_frontend_parser_h__

#include "frontend/ast_manager.h"
#include "ir/ir.h"

extern FILE *yyin;
extern int yylex();
extern int yyparse();

#endif

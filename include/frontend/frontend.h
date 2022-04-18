#ifndef __sysycompiler_frontend_frontend_h__
#define __sysycompiler_frontend_frontend_h__

#include "frontend/ast_manager.h"

extern FILE *yyin;
extern int yylex();
extern int yyparse();

extern ast::SourceManager src_manager;
extern ast::ASTManager ast_manager;

#endif

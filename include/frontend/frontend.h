#ifndef __sysycompiler_frontend_frontend_h__
#define __sysycompiler_frontend_frontend_h__

#include "frontend/parser.h"

extern ast::SourceManager src_manager;
extern ast::ASTManager ast_manager;
extern std::shared_ptr<ir::Module> module;

int Parse(const char *filename);
int AstToIR();

#endif

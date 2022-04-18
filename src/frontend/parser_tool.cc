#include <bits/types/FILE.h>

#include <iostream>

#include "frontend/ast_manager.h"
#include "frontend/frontend.h"

ast::SourceManager src_manager;
ast::ASTManager ast_manager(src_manager);

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cout << "need filename" << std::endl;
        return 1;
    }
    FILE *src = fopen(argv[1], "r");
    if (src == nullptr) {
        std::cout << "open file '" << argv[1] << "' failed" << std::endl;
        return 1;
    }

    src_manager.SetFileName(argv[1]);
    yyin = src;
    int result = yyparse();

    ast_manager.GetRoot().Visit();

    ast_manager.Dump(std::cout);
    return result;
}

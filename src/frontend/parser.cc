#include "frontend/parser.h"

ast::SourceManager src_manager;
ast::ASTManager ast_manager(src_manager);

int Parse(const char *filename) {
    FILE *src = fopen(filename, "r");
    if (src == nullptr) {
        std::cout << "open file '" << filename << "' failed" << std::endl;
        return 1;
    }

    src_manager.SetFileName(filename);
    yyin = src;
    int result = yyparse();
    if (result != 0) return result;

    ast_manager.GetRoot().Visit();

    return result;
}

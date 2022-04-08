#include <bits/types/FILE.h>

#include <iostream>

extern FILE *yyin;
extern int yyparse();

/* print reduce steps of parser */

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cout << "need filename" << std::endl;
        return 1;
    }
    FILE *f = fopen(argv[1], "r");
    if (f == nullptr) {
        std::cout << "open file '" << argv[1] << "' failed" << std::endl;
        return 1;
    }
    yyin = f;
    return yyparse();
}

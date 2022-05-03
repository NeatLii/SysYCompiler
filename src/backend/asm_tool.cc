#include <iostream>

#include "backend/backend.h"
#include "frontend/frontend.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cout << "need filename" << std::endl;
        return 1;
    }

    int result = Parse(argv[1]);
    if (result != 0) return result;

    result = AstToIR();
    if (result != 0) return result;

    result = Assembling();
    if (result != 0) return result;

    assembly.Dump(std::cout);

    return result;
}

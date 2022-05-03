# assembly lib

add_library(assembly SHARED
    operand.cc
    instruction.cc
)

# asm lib
add_library(asm SHARED
    asm.cc
)
target_link_libraries(asm
    assembly
    ir
    util
)

# asm_tool
add_executable(asm_tool asm_tool.cc)
target_link_libraries(asm_tool
    parser
    ast_to_ir
    asm
    util
)

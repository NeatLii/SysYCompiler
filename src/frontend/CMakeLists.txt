# flex and bison lib

find_package(BISON)
find_package(FLEX)
BISON_TARGET(SysYParser
    sysy.y
    ${CMAKE_CURRENT_BINARY_DIR}/sysy.tab.cc
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/sysy.tab.h
)
FLEX_TARGET(SysYLexer
    sysy.l
    ${CMAKE_CURRENT_BINARY_DIR}/sysy.lex.cc
)
ADD_FLEX_BISON_DEPENDENCY(SysYLexer SysYParser)
add_library(parser SHARED
    ${BISON_SysYParser_OUTPUTS}
    ${FLEX_SysYLexer_OUTPUTS}
)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# ast lib

add_library(ast SHARED
    source_manager.cc
    ast_manager.cc
)
target_link_libraries(parser ast)

# parser tool

add_executable(parser_tool
    parser_tool.cc
)
target_link_libraries(parser_tool
    parser
    util
)

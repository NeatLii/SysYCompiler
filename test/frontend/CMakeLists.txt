# flex and bison verify lib

find_package(BISON)
find_package(FLEX)
BISON_TARGET(SysYParserVerify
    sysy_verify.y
    ${CMAKE_CURRENT_BINARY_DIR}/sysy_verify.tab.cc
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/sysy_verify.tab.h
    VERBOSE ${CMAKE_CURRENT_SOURCE_DIR}/sysy_verify.output
)
FLEX_TARGET(SysYLexerVerify
    sysy_verify.l
    ${CMAKE_CURRENT_BINARY_DIR}/sysy_verify.lex.cc
)
ADD_FLEX_BISON_DEPENDENCY(SysYLexerVerify SysYParserVerify)
add_library(parser_verify SHARED
    ${BISON_SysYParserVerify_OUTPUTS}
    ${FLEX_SysYLexerVerify_OUTPUTS}
)

# parser verify tool

add_executable(parser_verify_tool
    parser_verify_tool.cc
)
target_link_libraries(parser_verify_tool
    parser_verify
    util
)

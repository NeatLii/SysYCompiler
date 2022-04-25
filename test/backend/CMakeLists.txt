add_executable(operand_test
    operand_test.cc
)
target_link_libraries(operand_test
    gtest_main
    assembly
)
gtest_discover_tests(operand_test)

add_executable(instruction_test
    instruction_test.cc
)
target_link_libraries(instruction_test
    gtest_main
    assembly
)
gtest_discover_tests(instruction_test)

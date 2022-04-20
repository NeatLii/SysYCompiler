add_executable(type_test type_test.cc)

target_link_libraries(type_test
    gtest_main
    ir
)

gtest_discover_tests(type_test)

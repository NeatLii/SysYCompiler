add_executable(util_test util_test.cc)

target_link_libraries(util_test
    gtest_main
    util
)

gtest_discover_tests(util_test)

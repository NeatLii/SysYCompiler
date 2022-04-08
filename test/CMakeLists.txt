include(GoogleTest)

add_subdirectory(frontend)
add_subdirectory(ir)
add_subdirectory(opt)
add_subdirectory(backend)

add_executable(utils_test utils_test.cc)

target_link_libraries(utils_test
    gtest_main
    utils
)

gtest_discover_tests(utils_test)

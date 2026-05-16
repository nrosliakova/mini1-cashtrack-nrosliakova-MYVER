add_executable(cashtrack_tests
    test_basic.cpp
)
target_link_libraries(cashtrack_tests
    PRIVATE
    cashtrack_core
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(cashtrack_tests)

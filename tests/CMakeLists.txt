# Add test executable
add_executable(AuroraTests
    test_example.cpp
)

# Link against GTest
target_link_libraries(AuroraTests PRIVATE
    GTest::GTest
    GTest::Main
)

# Discover and add tests to CTest
include(GoogleTest)
gtest_discover_tests(AuroraTests)
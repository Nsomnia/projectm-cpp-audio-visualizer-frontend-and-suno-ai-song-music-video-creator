#include <gtest/gtest.h>

TEST(ExampleSuite, TrivialAssertion) {
    ASSERT_EQ(1, 1);
}

TEST(ExampleSuite, StringComparison) {
    const char* str = "hello";
    ASSERT_STRNE("world", str);
}
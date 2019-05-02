#include "gtest/gtest.h"
#include "common_defs.h"
#include "url.h"
#include <string>
#include <ios>

namespace test_url {

    // The fixture for testing class Foo.

    class test_url : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.

        test_url() {
            // You can do set-up work for each test here.
        }

        virtual ~test_url() {
            // You can do clean-up work that doesn't throw exceptions here.
        }

        // If the constructor and destructor are not enough for setting up
        // and cleaning up each test, you can define the following methods:

        virtual void SetUp() {
            // Code here will be called immediately after the constructor (right
            // before each test).
        }

        virtual void TearDown() {
            // Code here will be called immediately after each test (right
            // before the destructor).
        }

        // Objects declared here can be used by all tests in the test case for Foo.
    };

    // Tests that Foo does Xyz.

    TEST_F(test_url, is_https) {
        ASSERT_EQ(is_https("hello"), false);
        ASSERT_EQ(is_https("https://www.google.com"), true);
        ASSERT_EQ(is_https("http://www.google.com"), false);
        ASSERT_EQ(is_https("xhttps://www.google.com"), false);
    }

} // namespace

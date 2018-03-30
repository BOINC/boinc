#include "gtest/gtest.h"
#include "common_defs.h"
#include "str_util.h"
#include <string>
#include <ios>

namespace str_util_test {

    // The fixture for testing class Foo.

    class str_util_test : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.

        str_util_test() {
            // You can do set-up work for each test here.
        }

        virtual ~str_util_test() {
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

    TEST_F(str_util_test, path_to_filename) {
        std::string fname = "";
        ASSERT_EQ(path_to_filename("/home/blah", fname), 0);
        ASSERT_EQ(fname, "blah");
        ASSERT_EQ(path_to_filename("hellokeith", fname), 0);
        ASSERT_EQ(fname, "hellokeith");
        fname = "";
        ASSERT_EQ(path_to_filename("/home/blah/", fname), -2);
        ASSERT_EQ(fname, "");
        ASSERT_EQ(path_to_filename("", fname), -1);
        ASSERT_EQ(fname, "");
    }

    TEST_F(str_util_test, nbytes_to_string) {
        char buf[256];
        nbytes_to_string(1024, 0, buf, sizeof (buf));
        ASSERT_STREQ(buf, "1.00 KB");
        nbytes_to_string(1024, 1024 * 1024, buf, sizeof (buf));
        ASSERT_STREQ(buf, "0.00/1.00 MB");
        nbytes_to_string(512, 1024, buf, sizeof (buf));
        ASSERT_STREQ(buf, "0.50/1.00 KB");
        nbytes_to_string(50000000000000, 0, buf, sizeof (buf));
        ASSERT_STREQ(buf, "45.47 TB");
    }
    
    TEST_F(str_util_test, strip_whitespace) {
        std::string tmp = "     white space   ";
        strip_whitespace(tmp);
        ASSERT_EQ(tmp, "white space");
        tmp = "nospaces";
        strip_whitespace(tmp);
        ASSERT_EQ(tmp, "nospaces");
    }

} // namespace

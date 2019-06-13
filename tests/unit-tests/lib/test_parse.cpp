#include "gtest/gtest.h"
#include "common_defs.h"
#include "url.h"
#include <string>
#include <ios>

using namespace std;

namespace test_parse {

    // The fixture for testing class Foo.

    class test_parse : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.

        test_parse() {
            // You can do set-up work for each test here.
        }

        virtual ~test_parse() {
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

    TEST_F(test_parse, xml_unescape) {
        string test = "&lt;&gt;&quot;&apos;&amp;&#xD;&#xd;&#xA;&#xa;&#75;";
        string answer = "<>\"\'&\r\r\n\nK";
        xml_unescape(test);
        EXPECT_EQ(test, answer);

        //Note: this is to check that partial values don't pass strncmp for previously bad compares.
        test = "&quoYIKES&apoBOO";
        answer = "&quoYIKES&apoBOO";
        xml_unescape(test);
        EXPECT_EQ(test, answer);

        //Testing the ascii conversion unknown.
        test = "&#9s3;&#694312532&#;eq&#1234;&#75";
        answer = "&#9s3;&#694312532&#;eq&#1234;&#75";
        xml_unescape(test);
        EXPECT_EQ(test, answer);
    }

} // namespace

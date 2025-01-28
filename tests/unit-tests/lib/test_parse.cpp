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

    TEST_F(test_parse, XML_PARSER) {

        MIOFILE mf;

        XML_PARSER xp(&mf);

        mf.init_buf_read("<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\n"
"<blah>\n"
"    <x>\n"
"    asdlfkj\n"
"      <x> fj</x>\n"
"    </x>\n"
"    <str>blah</str>\n"
"    <int>  6\n"
"    </int>\n"
"    <double>6.555</double>\n"
"    <bool>0</bool>\n"
"</blah>");

        EXPECT_TRUE(xp.parse_start("blah"));

        int success = false;
        int expects = 0;

        char name[64];
        strncpy(name, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", sizeof(name));

        int val;

        double x;

        bool flag;

        while (!xp.get_tag()) {
            if (!xp.is_tag) {
                continue;
            }
            if (xp.match_tag("/blah")) {
                success = true;
            } else if (xp.parse_str("str", name, 64)) {
                EXPECT_STREQ(name, "blah");
                expects++;
            } else if (xp.parse_int("int", val)) {
                EXPECT_EQ(val, 6);
                expects++;
            } else if (xp.parse_double("double", x)) {
                EXPECT_EQ(x, 6.555);
                expects++;
            } else if (xp.parse_bool("bool", flag)) {
                EXPECT_FALSE(flag);
                expects++;
            } else {
                xp.skip_unexpected(false, "xml test");
                EXPECT_STREQ(xp.parsed_tag, "x");
                expects++;
            }
        }

        EXPECT_TRUE(success);
        EXPECT_EQ(expects, 5);

    }

} // namespace

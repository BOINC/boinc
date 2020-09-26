#include "gtest/gtest.h"
#include "common_defs.h"
#include "url.h"
#include <string>
#include <ios>

#define PARSER_CHOICE TINYXML_WRAPPER

const int SML_BUF_LEN = 32;
const int MED_BUF_LEN = 64;
const int LRG_BUF_LEN = 1024;
const std::string TEST_XML_FILE_NAME_GOOD = "test_good.xml";
const std::string TEST_XML_FILE_NAME_BAD = "test_bad.xml";
const std::string TEST_XML_DATA_GOOD = "<?xml version=\"1.0\" encoding=\"UTF - 8\" ?>       \
                            <mystarttag>this is the start</mystarttag>                      \
                            <mystring>foo data</mystring>                                   \
                            <mynumber>12345</mynumber>                                      \
                            <mynegnum>-54321</mynegnum>                                     \
                            <myfloat>1.2345</myfloat>                                       \
                            <mynegfloat>-5.4321</mynegfloat>                                \
                            <mybool>1</mybool>                                              \
                            <myimplicitbool/>                                               \
                            <mylonglong>8589934592</mylonglong>                             \
                            <venue name=\"myvenue\">FOO</venue>";

using namespace std;

namespace test_parse {

    // The fixture for testing parse.h/parse.cpp
    class test_parse : public ::testing::Test {
    protected:
        FILE* xml_test_file_good;   // The file with perfect data for testing correctness
        FILE* xml_test_file_bad;   // The file with bad data for testing error handling
        // MIOFILES are needed for XML_PARSERs
        MIOFILE* good_miofile;
        MIOFILE* bad_miofile;
        PARSER_CHOICE* good_data_parser;
        PARSER_CHOICE* bad_data_parser;

        test_parse() {
            // Make testing files for the XML parser
            string file_data_good = TEST_XML_DATA_GOOD;
            //string file_data_bad = TEST_XML_DATA_BAD;

            xml_test_file_good = fopen(TEST_XML_FILE_NAME_GOOD.c_str(), "w+");
            //xml_test_file_bad = fopen(TEST_XML_FILE_NAME_BAD, "w+");

            assert(xml_test_file_good);
            assert(EOF != fputs(file_data_good.c_str(), xml_test_file_good));
            //assert(xml_test_file_bad);
            //assert(EOF != fputs(file_data_bad.c_str(), xml_test_file_bad));

            // Create the MIOFILE objects
            good_miofile = new MIOFILE;
            //bad_miofile = new MIOFILE;
            good_miofile->init_file(xml_test_file_good);
            //bad_miofile.init_file(xml_test_file_bad);

            //fseek(xml_test_file_good, 0, SEEK_SET);
            //fseek(xml_test_file_bad, 0, SEEK_SET);
            good_data_parser = new PARSER_CHOICE(good_miofile);
            //bad_data_parser = new PARSER_CHOICE(bad_miofile);
        }

        virtual ~test_parse() {
            fclose(xml_test_file_good);
            //fclose(xml_test_file_bad);
            delete good_miofile;
            //delete bad_miofile;

            // Remove the files after testing
            remove(TEST_XML_FILE_NAME_GOOD.c_str());
            //remove(TEST_XML_FILE_NAME_BAD);

            delete good_data_parser;
            //delete bad_data_parser;
        }

        // If the constructor and destructor are not enough for setting up
        // and cleaning up each test, you can define the following methods:

        virtual void SetUp() {
            
        }

        virtual void TearDown() {
            
        }
    };


    TEST_F(test_parse, parse_bool) {
        // Test explicit boolean for false and true
        string tag = "bool";
        string test = "<bool>0</bool>";
        bool answer = false;

        parse_bool(test.c_str(), tag.c_str(), answer);
        EXPECT_FALSE(answer);

        answer = false;
        test = "<bool>1</bool>";
        parse_bool(test.c_str(), tag.c_str(), answer);
        EXPECT_TRUE(answer);

        // Test for implicit boolean tags
        answer = false;
        test = "<bool/>";
        parse_bool(test.c_str(), tag.c_str(), answer);
        EXPECT_TRUE(answer);

        // Test for implicit boolean with spaces
        answer = false;
        test = "<bool />";
        parse_bool(test.c_str(), tag.c_str(), answer);
        EXPECT_TRUE(answer);

        answer = false;
        test = "<bool  />";
        parse_bool(test.c_str(), tag.c_str(), answer);
        EXPECT_FALSE(answer);

        // Test with incorrect tag
        answer = false;
        test = "<foo/>";
        parse_bool(test.c_str(), tag.c_str(), answer);
        EXPECT_FALSE(answer);

        // Test for explicit without a trailer
        answer = false;
        test = "<bool>";
        parse_bool(test.c_str(), tag.c_str(), answer);
        EXPECT_FALSE(answer);
    }


    TEST_F(test_parse, parse_str) {
        string tag = "str";
        // Test normal strings
        string test = "<str>This is my string</str>";
        string answer = "This is my string";
        string result;

        parse_str(test.c_str(), tag.c_str(), result);
        EXPECT_EQ(result, answer);

        // Test strings with attributes
        test = "<str attr=\"myattr\">This is my string with attrs</str>";
        answer = "This is my string with attrs";
        result = "";
        parse_str(test.c_str(), tag.c_str(), result);
        EXPECT_EQ(result, answer);

        // Test empty string
        test = "<str></str>";
        answer = "";
        result = "";
        parse_str(test.c_str(), tag.c_str(), result);
        EXPECT_EQ(result, answer);

        // Test bad input
        test = "<str>/str";
        EXPECT_FALSE(parse_str(test.c_str(), tag.c_str(), result));
    }


    TEST_F(test_parse, parse_attr) {
        string attr = "myattr";
        // Test normal attrs
        string test = "<foo myattr=\"FOO\"></foo>";
        string answer = "FOO";
        char result[SML_BUF_LEN];

        parse_attr(test.c_str(), attr.c_str(), result, SML_BUF_LEN);
        EXPECT_STREQ(result, answer.c_str());

        // Test empty attr
        test = "<foo myattr=\"\"></foo>";
        answer = "";
        memset(result, 0, SML_BUF_LEN);

        parse_attr(test.c_str(), attr.c_str(), result, SML_BUF_LEN);
        EXPECT_STREQ(result, answer.c_str());

        // Test bad input
        test = "<foo myattr=\"needs another quote></foo>";
        answer = "";
        memset(result, 0, SML_BUF_LEN);

        parse_attr(test.c_str(), attr.c_str(), result, SML_BUF_LEN);
        EXPECT_STREQ(result, answer.c_str());

        // Test only attrs
        test = "myattr=\"FOO\"";
        answer = "FOO";
        result[SML_BUF_LEN];

        parse_attr(test.c_str(), attr.c_str(), result, SML_BUF_LEN);
        EXPECT_STREQ(result, answer.c_str());
    }


    TEST_F(test_parse, extract_venue) {
        string venue = "myvenue";
        // Test normal venues with explicit names
        string test = "<venue name=\"myvenue\">FOO</venue>";
        string answer = "FOO";
        char result[SML_BUF_LEN];

        extract_venue(test.c_str(), venue.c_str(), result, SML_BUF_LEN);
        EXPECT_STREQ(result, answer.c_str());

        // Test a single venue without a name
        test = "<venue>FOO</venue>";
        answer = "";
        memset(result, 0, SML_BUF_LEN);

        extract_venue(test.c_str(), venue.c_str(), result, SML_BUF_LEN);
        EXPECT_STREQ(result, answer.c_str());

        // Test named after unnamed venue
        test = "<venue>NOTFOO</venue><venue name=\"myvenue\">FOO</venue>";
        answer = "FOO";
        memset(result, 0, SML_BUF_LEN);

        extract_venue(test.c_str(), venue.c_str(), result, SML_BUF_LEN);
        EXPECT_STREQ(result, answer.c_str());
    }


    TEST_F(test_parse, xml_escape) {
        string test = "<>\"\'&\r\nK";
        string answer = "&lt;>\"'&amp;&#13;&#10;K";
        char result[MED_BUF_LEN];

        xml_escape(test.c_str(), result, MED_BUF_LEN);
        EXPECT_STREQ(result, answer.c_str());
    }


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


    TEST_F(test_parse, XML_PARSER_parse_start) {
        string tag = "mystarttag";

        EXPECT_TRUE(good_data_parser->parse_start(tag.c_str()));
    }


    TEST_F(test_parse, XML_PARSER_parse_str) {
        string tag = "mystring";
        string answer = "foo data";
        char result[SML_BUF_LEN];

        // Find the appropriate tag
        do {} while (!good_data_parser->get_tag() && !good_data_parser->match_tag(tag.c_str()));

        // Parse one of the strings in the XML file
        good_data_parser->parse_str(tag.c_str(), result, SML_BUF_LEN);
        EXPECT_STREQ(result, answer.c_str());

        // Attempt to parse a string tag that doesn't exist
        EXPECT_FALSE(good_data_parser->parse_str("Faketag", result, SML_BUF_LEN));
    }


    TEST_F(test_parse, XML_PARSER_parse_string) {
        string tag = "mystring";
        string answer = "foo data";
        string result;

        do {} while (!good_data_parser->get_tag() && !good_data_parser->match_tag(tag.c_str()));

        // Test normal string parsing
        EXPECT_TRUE(good_data_parser->parse_string(tag.c_str(), result));
        EXPECT_EQ(result, answer);

        // Test bad tag input
        EXPECT_FALSE(good_data_parser->parse_string("faketag", result));
    }


    TEST_F(test_parse, XML_PARSER_parse_int) {
        string tag = "mynumber";
        int answer = 12345;
        int result;

        do {} while (!good_data_parser->get_tag() && !good_data_parser->match_tag(tag.c_str()));

        // Test a normal positive integer
        EXPECT_TRUE(good_data_parser->parse_int(tag.c_str(), result));
        EXPECT_EQ(result, answer);

        // Test a normal negative integer
        tag = "mynegnum";
        answer = -54321;

        do {} while (!good_data_parser->get_tag() && !good_data_parser->match_tag(tag.c_str()));

        EXPECT_TRUE(good_data_parser->parse_int(tag.c_str(), result));
        EXPECT_EQ(result, answer);

        // Attempt to parse an int tag that doesn't exist
        EXPECT_FALSE(good_data_parser->parse_int("faketag", result));
    }


    TEST_F(test_parse, XML_PARSER_parse_long) {
        string tag = "mynumber";
        long answer = 12345;
        long result;

        do {} while (!good_data_parser->get_tag() && !good_data_parser->match_tag(tag.c_str()));

        // Test a normal positive integer
        EXPECT_TRUE(good_data_parser->parse_long(tag.c_str(), result));
        EXPECT_EQ(result, answer);

        // Test a normal negative integer
        tag = "mynegnum";
        answer = -54321;

        do {} while (!good_data_parser->get_tag() && !good_data_parser->match_tag(tag.c_str()));

        EXPECT_TRUE(good_data_parser->parse_long(tag.c_str(), result));
        EXPECT_EQ(result, answer);

        // Attempt to parse a long tag that doesn't exist
        EXPECT_FALSE(good_data_parser->parse_long("faketag", result));
    }


    TEST_F(test_parse, XML_PARSER_parse_double) {
        string tag = "myfloat";
        double answer = 1.2345;
        double result;

        do {} while (!good_data_parser->get_tag() && !good_data_parser->match_tag(tag.c_str()));

        // Test a normal positive floating point number
        EXPECT_TRUE(good_data_parser->parse_double(tag.c_str(), result));
        EXPECT_EQ(result, answer);

        // Test a normal negative floating point number
        tag = "mynegfloat";
        answer = -5.4321;

        do {} while (!good_data_parser->get_tag() && !good_data_parser->match_tag(tag.c_str()));

        EXPECT_TRUE(good_data_parser->parse_double(tag.c_str(), result));
        EXPECT_EQ(result, answer);

        // Attempt to parse a floating point tag that doesn't exist
        EXPECT_FALSE(good_data_parser->parse_double("faketag", result));
    }


    TEST_F(test_parse, XML_PARSER_parse_ulong) {
        string tag = "mynumber";
        unsigned long answer = 12345;
        unsigned long result;

        do {} while (!good_data_parser->get_tag() && !good_data_parser->match_tag(tag.c_str()));

        // Test a normal positive integer
        EXPECT_TRUE(good_data_parser->parse_ulong(tag.c_str(), result));
        EXPECT_EQ(result, answer);

        // Attempt to parse an unsigned long tag that doesn't exist
        EXPECT_FALSE(good_data_parser->parse_ulong("faketag", result));
    }


    TEST_F(test_parse, XML_PARSER_parse_ulonglong) {
        string tag = "mylonglong";
        unsigned long long answer = 8589934592;
        unsigned long long result;

        do {} while (!good_data_parser->get_tag() && !good_data_parser->match_tag(tag.c_str()));

        // Test a normal positive integer
        EXPECT_TRUE(good_data_parser->parse_ulonglong(tag.c_str(), result));
        EXPECT_EQ(result, answer);

        // Attempt to parse an unsigned long tag that doesn't exist
        EXPECT_FALSE(good_data_parser->parse_ulonglong("faketag", result));
    }


    TEST_F(test_parse, XML_PARSER_parse_bool) {
        string tag = "mybool";
        bool result;

        do {} while (!good_data_parser->get_tag() && !good_data_parser->match_tag(tag.c_str()));

        // Test a normal explicit boolean in form <bool>1</bool>
        EXPECT_TRUE(good_data_parser->parse_bool(tag.c_str(), result));
        EXPECT_TRUE(result);
    }


    TEST_F(test_parse, XML_PARSER_copy_element) {
        char tag[SML_BUF_LEN] = "mystring";
        string answer = "<mystring>foo data</mystring>";
        string result;

        do {} while (!good_data_parser->get_tag() && !good_data_parser->match_tag(tag));

        good_data_parser->copy_element(result);

        EXPECT_EQ(result, answer);
    }


    // Tests for the collection of attributes in an element
    TEST_F(test_parse, XML_PARSER_get_tag) {
        string attr = "name";
        string tag = "venue";
        string answer = "name=\"myvenue\"";
        char attrs[SML_BUF_LEN];

        do {} while (!good_data_parser->get_tag(attrs, SML_BUF_LEN) && !good_data_parser->match_tag(tag.c_str()));

        EXPECT_STREQ(attrs, answer.c_str());
    }


    // Tests only for whether any adverse effects occur
    TEST_F(test_parse, XML_PARSER_skip_unexpected) {
        good_data_parser->skip_unexpected(true, "test xml");
    }

} // namespace

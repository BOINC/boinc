// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2026 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

#ifndef _WIN32
#include <string>
#include <ios>
#include "gtest/gtest.h"
#endif

#include "common_defs.h"
#include "str_util.h"
#include "error_numbers.h"

extern size_t strlcpy(char*, const char*, size_t);

namespace test_libboinc {
    class test_str_util : public ::testing::Test {
    protected:
#ifdef _WIN32
        int setenv(const char* name, const char* value, int overwrite)
        {
            int errcode = 0;
            if (!overwrite) {
                size_t envsize = 0;
                errcode = getenv_s(&envsize, NULL, 0, name);
                if (errcode || envsize) return errcode;
            }
            return _putenv_s(name, value);
        }
#endif
    };

    TEST_F(test_str_util, secs_to_hmsf) {
        char buf[128];
        secs_to_hmsf(0.0, buf, 128);
        EXPECT_STREQ(buf, "0h00m00s00");
        secs_to_hmsf(1.0, buf, 128);
        EXPECT_STREQ(buf, "0h00m01s00");
        secs_to_hmsf(1.23456789, buf, 128);
        EXPECT_STREQ(buf, "0h00m01s23");
        secs_to_hmsf(12345.6789, buf, 128);
        EXPECT_STREQ(buf, "3h25m45s67");
        secs_to_hmsf(123456.789, buf, 128);
        EXPECT_STREQ(buf, "34h17m36s78");
    }

    TEST_F(test_str_util, nbytes_to_string) {
        char buf[256];
        nbytes_to_string(5.0, 0.0, buf, sizeof(buf));
        EXPECT_STREQ(buf, "5 bytes");
        nbytes_to_string(1024.0, 0.0, buf, sizeof(buf));
        EXPECT_STREQ(buf, "1.00 KB");
        nbytes_to_string(5.0 * 1024 * 1024, 0.0, buf, sizeof(buf));
        EXPECT_STREQ(buf, "5.00 MB");
        nbytes_to_string(15.0 * 1024 * 1024 * 1024, 0.0, buf, sizeof(buf));
        EXPECT_STREQ(buf, "15.00 GB");
        nbytes_to_string(50000000000000.0, 0.0, buf, sizeof(buf));
        EXPECT_STREQ(buf, "45.47 TB");
        nbytes_to_string(2.0, 48.0, buf, sizeof(buf));
        EXPECT_STREQ(buf, "2/48 bytes");
        nbytes_to_string(512.0, 1024.0, buf, sizeof(buf));
        EXPECT_STREQ(buf, "0.50/1.00 KB");
        nbytes_to_string(1024.0, 1.0 * 1024 * 1024, buf, sizeof(buf));
        EXPECT_STREQ(buf, "0.00/1.00 MB");
        nbytes_to_string(6.0 * 1024 * 1024 * 1024, 6.0 * 1024 * 1024 * 1024,
            buf, sizeof(buf));
        EXPECT_STREQ(buf, "6.00/6.00 GB");
        nbytes_to_string(24.0 * 1024 * 1024 * 1024 * 1024,
            48.0 * 1024 * 1024 * 1024 * 1024, buf, sizeof(buf));
        EXPECT_STREQ(buf, "24.00/48.00 TB");
    }

    TEST_F(test_str_util, parse_command_line) {
        ;
        char buf[256];
        char* argv[100];
        int ret;
        char* nilbuf{ 0 };
        snprintf(buf, sizeof(buf), "one two three");
        ret = parse_command_line(buf, argv);
        EXPECT_EQ(ret, 3);
        EXPECT_STREQ(argv[0], "one");
        EXPECT_STREQ(argv[1], "two");
        EXPECT_STREQ(argv[2], "three");
        EXPECT_STREQ(argv[3], nilbuf);
        EXPECT_STREQ(buf, "one");
        snprintf(buf, sizeof(buf), "four \'five\' \"six\"");
        ret = parse_command_line(buf, argv);
        EXPECT_EQ(ret, 3);
        EXPECT_STREQ(argv[0], "four");
        EXPECT_STREQ(argv[1], "five");
        EXPECT_STREQ(argv[2], "six");
        EXPECT_STREQ(argv[3], nilbuf);
        EXPECT_STREQ(buf, "four");
        snprintf(buf, sizeof(buf), "seven \'eig ht\' \"ni ne\"");
        ret = parse_command_line(buf, argv);
        EXPECT_EQ(ret, 3);
        EXPECT_STREQ(argv[0], "seven");
        EXPECT_STREQ(argv[1], "eig ht");
        EXPECT_STREQ(argv[2], "ni ne");
        EXPECT_STREQ(argv[3], nilbuf);
        EXPECT_STREQ(buf, "seven");
        snprintf(buf, sizeof(buf), "tän \'elèv én\' \"tŵelv e\"");
        ret = parse_command_line(buf, argv);
        EXPECT_EQ(ret, 3);
        EXPECT_STREQ(argv[0], "tän");
        EXPECT_STREQ(argv[1], "elèv én");
        EXPECT_STREQ(argv[2], "tŵelv e");
        EXPECT_STREQ(argv[3], nilbuf);
        EXPECT_STREQ(buf, "tän");
        // function doesn't check syntax so this works too
        snprintf(buf, sizeof(buf), "13\" \'\"4teen\'\"   ");
        ret = parse_command_line(buf, argv);
        EXPECT_EQ(ret, 3);
        EXPECT_STREQ(argv[0], "13\"");
        EXPECT_STREQ(argv[1], "\"4teen");
        EXPECT_STREQ(argv[2], "   ");
        EXPECT_STREQ(argv[3], nilbuf);
        EXPECT_STREQ(buf, "13\"");
    }

    TEST_F(test_str_util, strip_whitespace) {
        std::string tmp = "     white space   ";
        strip_whitespace(tmp);
        EXPECT_EQ(tmp, "white space");
        tmp = "nospaces";
        strip_whitespace(tmp);
        EXPECT_EQ(tmp, "nospaces");
        char buf[128] = "     char space   ";
        strip_whitespace(buf);
        EXPECT_STREQ(buf, "char space");
    }

    TEST_F(test_str_util, strip_quotes) {
        std::string tmp = "\"\' white\' \"space\'\" ";
        strip_quotes(tmp);
        EXPECT_EQ(tmp, "white\' \"space");
        tmp = "no\"space\'s";
        strip_quotes(tmp);
        EXPECT_EQ(tmp, "no\"space\'s");
        tmp = "\"elèv\' én\"";
        strip_quotes(tmp);
        EXPECT_EQ(tmp, "elèv\' én");
        char buf[128] = "\"\' char\' \"space\'\" ";
        strip_quotes(buf);
        EXPECT_STREQ(buf, "char\' \"space");
    }

    TEST_F(test_str_util, collapse_whitespace) {
        std::string tmp = "     white space   ";
        collapse_whitespace(tmp);
        EXPECT_EQ(tmp, " white space ");
        tmp = "nospaces";
        collapse_whitespace(tmp);
        EXPECT_EQ(tmp, "nospaces");
        tmp = "inner     spaces";
        collapse_whitespace(tmp);
        EXPECT_EQ(tmp, "inner spaces");
        char buf[128] = "  char     spaces ";
        collapse_whitespace(buf);
        EXPECT_STREQ(buf, " char spaces ");
    }

    TEST_F(test_str_util, unescape_os_release) {
        //a-f get unescaped, g-j not
        char buf[128] = "a\\\\b\\$c\\\'d\\\"e\\`f\\\?g\\\th\\\bi\\12j";
        unescape_os_release(buf);
        EXPECT_STREQ(buf, "a\\b$c\'d\"e`f\\\?g\\\th\\\bi\\12j");
    }

    TEST_F(test_str_util, time_to_string) {
        char* buf;
        setenv("TZ", "UTC", 1);
#ifdef _MSC_VER
        _tzset();
#else
        tzset();
#endif
        buf = time_to_string(false);
        EXPECT_STREQ(buf, "---");
        buf = time_to_string(1.0);
        EXPECT_STREQ(buf, "01-Jan-1970 00:00:01");
        buf = time_to_string(12345678910.0);
        EXPECT_STREQ(buf, "21-Mar-2361 19:15:10");
    }

    TEST_F(test_str_util, precision_time_to_string) {
        char* buf;
        setenv("TZ", "UTC", 1);
#ifdef _MSC_VER
        _tzset();
#else
        tzset();
#endif
        buf = precision_time_to_string(false);
        EXPECT_STREQ(buf, "1970-01-01 00:00:00.0000");
        buf = precision_time_to_string(1.0);
        EXPECT_STREQ(buf, "1970-01-01 00:00:01.0000");
        buf = precision_time_to_string(1555876749.1234);
        EXPECT_STREQ(buf, "2019-04-21 19:59:09.1233");
        buf = precision_time_to_string(12345678910.10000);
        // here we have an undefined behavior that gives
        // negative value on x64 and positive value on arm64
#ifdef __arm64__
        EXPECT_STREQ(buf, "2361-03-21 19:15:10.2147483647");
#else
        EXPECT_STREQ(buf, "2361-03-21 19:15:10.-2147483648");
#endif
    }

    TEST_F(test_str_util, timediff_format) {
        std::string tmp;
        tmp = timediff_format(false);
        EXPECT_EQ(tmp, "00:00:00");
        tmp = timediff_format(59.0);
        EXPECT_EQ(tmp, "00:00:59");
        tmp = timediff_format(3599.0);
        EXPECT_EQ(tmp, "00:59:59");
        tmp = timediff_format(3600.0);
        EXPECT_EQ(tmp, "01:00:00");
        tmp = timediff_format(123456.7);
        EXPECT_EQ(tmp, "1 days 10:17:36");
    }

    TEST_F(test_str_util, string_substitute) {
        std::string tmp = "The quick brown fox jumps over the lazy dog";
        char buf[256];
        int ret = string_substitute(tmp.c_str(), buf, sizeof(buf), "brown",
            "red");
        EXPECT_EQ(ret, 0);
        EXPECT_STREQ(buf, "The quick red fox jumps over the lazy dog");
        ret = string_substitute(tmp.c_str(), buf, 13, "brown", "red");
        EXPECT_EQ(ret, ERR_BUFFER_OVERFLOW);
        ret = string_substitute(tmp.c_str(), buf, 16, "brown", "red");
        EXPECT_EQ(ret, ERR_BUFFER_OVERFLOW);
        tmp = "Falsches Üben von Xylophonmusik quält jeden größeren Zwerg";
        ret = string_substitute(tmp.c_str(), buf, sizeof(buf), "quält",
            "ärgert");
        EXPECT_EQ(ret, 0);
        EXPECT_STREQ(buf,
            "Falsches Üben von Xylophonmusik ärgert jeden größeren Zwerg");
    }

    TEST_F(test_str_util, strip_translation) {
        char buf[256] = "_(\"The quick brown fox jumps over the lazy dog\")";
        strip_translation(buf);
        EXPECT_STREQ(buf, "The quick brown fox jumps over the lazy dog");
        snprintf(buf, sizeof(buf),
            "The _(\"quick brown\") fox jumps over the _(\"lazy\") dog");
        strip_translation(buf);
        EXPECT_STREQ(buf, "The quick brown fox jumps over the lazy dog");
        snprintf(buf, sizeof(buf),
            "The _\"quick brown\" ) fox jumps over the (_\"lazy\") dog");
        strip_translation(buf);
        EXPECT_STREQ(buf,
            "The _\"quick brown\" ) fox jumps over the (_\"lazy dog");
    }

    TEST_F(test_str_util, lf_terminate) {
        char* buf;
        const size_t buf_size = 256;
        buf = (char*)malloc(buf_size);
        strlcpy(buf, "no\nlf ending", buf_size);
        buf = lf_terminate(buf);
        EXPECT_STREQ(buf, "no\nlf ending\n");
        strlcpy(buf, "lf\n ending\n", buf_size);
        buf = lf_terminate(buf);
        EXPECT_STREQ(buf, "lf\n ending\n");
    }

    TEST_F(test_str_util, is_valid_filename) {
        //char tmp = "filename.txt";
        bool ret = is_valid_filename("filename.txt");
        EXPECT_TRUE(ret);
        ret = is_valid_filename("../filename.txt");
        EXPECT_FALSE(ret);
        ret = is_valid_filename("../file\nname.txt");
        EXPECT_FALSE(ret);
        ret = is_valid_filename("/filename.txt");
        EXPECT_FALSE(ret);
    }

    TEST_F(test_str_util, path_to_filename) {
        std::string fname = "";
        EXPECT_EQ(path_to_filename("/home/blah", fname), 0);
        EXPECT_EQ(fname, "blah");
        EXPECT_EQ(path_to_filename("hellokeith", fname), 0);
        EXPECT_EQ(fname, "hellokeith");
        fname = "";
        EXPECT_EQ(path_to_filename("/home/blah/", fname), -2);
        EXPECT_EQ(fname, "");
        EXPECT_EQ(path_to_filename("", fname), -1);
        EXPECT_EQ(fname, "");
    }

    TEST_F(test_str_util, split_string) {
        std::string str = "one,two,three";
        auto tokens = split_string(str, ",");
        ASSERT_EQ(3, tokens.size());
        EXPECT_EQ("one", tokens[0]);
        EXPECT_EQ("two", tokens[1]);
        EXPECT_EQ("three", tokens[2]);

        str = "one,,two,,,three,";
        tokens = split_string(str, ",");
        ASSERT_EQ(3, tokens.size());
        EXPECT_EQ("one", tokens[0]);
        EXPECT_EQ("two", tokens[1]);
        EXPECT_EQ("three", tokens[2]);

        str = ",one,two,three,";
        tokens = split_string(str, ",");
        ASSERT_EQ(3, tokens.size());
        EXPECT_EQ("one", tokens[0]);
        EXPECT_EQ("two", tokens[1]);
        EXPECT_EQ("three", tokens[2]);

        str = ",,,one,,two,,,,,three,,,,";
        tokens = split_string(str, ",");
        ASSERT_EQ(3, tokens.size());
        EXPECT_EQ("one", tokens[0]);
        EXPECT_EQ("two", tokens[1]);
        EXPECT_EQ("three", tokens[2]);

        str = "";
        tokens = split_string(str, ",");
        ASSERT_EQ(0, tokens.size());

        str = "one";
        tokens = split_string(str, ",");
        ASSERT_EQ(1, tokens.size());
        EXPECT_EQ("one", tokens[0]);

        str = "one;two;three";
        tokens = split_string(str, ";");
        ASSERT_EQ(3, tokens.size());
        EXPECT_EQ("one", tokens[0]);
        EXPECT_EQ("two", tokens[1]);
        EXPECT_EQ("three", tokens[2]);

        str = "one two three";
        tokens = split_string(str, " ");
        ASSERT_EQ(3, tokens.size());
        EXPECT_EQ("one", tokens[0]);
        EXPECT_EQ("two", tokens[1]);
        EXPECT_EQ("three", tokens[2]);

        str = "one\ntwo\nthree";
        tokens = split_string(str, "\n");
        ASSERT_EQ(3, tokens.size());
        EXPECT_EQ("one", tokens[0]);
        EXPECT_EQ("two", tokens[1]);
        EXPECT_EQ("three", tokens[2]);

        str = "onetesttwotestthreetest";
        tokens = split_string(str, "test");
        ASSERT_EQ(3, tokens.size());
        EXPECT_EQ("one", tokens[0]);
        EXPECT_EQ("two", tokens[1]);
        EXPECT_EQ("three", tokens[2]);

        str = ";one;,;two;,;three;";
        tokens = split_string(str, ",");
        ASSERT_EQ(3, tokens.size());
        EXPECT_EQ(";one;", tokens[0]);
        EXPECT_EQ(";two;", tokens[1]);
        EXPECT_EQ(";three;", tokens[2]);

        str = "nodelimiter";
        tokens = split_string(str, "");
        ASSERT_EQ(1, tokens.size());
        EXPECT_EQ("nodelimiter", tokens[0]);

        str = "";
        tokens = split_string(str, "");
        ASSERT_EQ(0, tokens.size());
    }

}

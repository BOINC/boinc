// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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

#include "gtest/gtest.h"
#include "common_defs.h"
#include "url.h"
#include <string>
#include <ios>

using namespace std;

namespace test_url {
    class test_url : public ::testing::Test {};

    TEST_F(test_url, parse_url) {
        string url;
        PARSED_URL purl;

        //First is a simple basic test
        url = "http://www.example.com";
        parse_url(url.c_str(), purl);

        EXPECT_EQ(purl.protocol, URL_PROTOCOL_HTTP);
        EXPECT_STREQ(purl.user, "");
        EXPECT_STREQ(purl.passwd, "");
        EXPECT_EQ(purl.port, 80);
        EXPECT_STREQ(purl.file, "");
        EXPECT_STREQ(purl.host, "www.example.com");

        //Test Getting https and file name
        url = "https://secure.example.com/test/file.txt";
        parse_url(url.c_str(), purl);

        EXPECT_EQ(purl.protocol, URL_PROTOCOL_HTTPS);
        EXPECT_STREQ(purl.user, "");
        EXPECT_STREQ(purl.passwd, "");
        EXPECT_EQ(purl.port, 443);
        EXPECT_STREQ(purl.file, "test/file.txt");
        EXPECT_STREQ(purl.host, "secure.example.com");

        //Test Socks, different port and user being passed
        url = "socks://user@sock.example.com:5005/";
        parse_url(url.c_str(), purl);

        EXPECT_EQ(purl.protocol, URL_PROTOCOL_SOCKS);
        EXPECT_STREQ(purl.user, "user");
        EXPECT_STREQ(purl.passwd, "");
        EXPECT_EQ(purl.port, 5005);
        EXPECT_STREQ(purl.file, "");
        EXPECT_STREQ(purl.host, "sock.example.com");

        //Test bad protocol passed, and password being set.  port not set.
        url = "user:mypass@yikes.example.com/";
        parse_url(url.c_str(), purl);

        EXPECT_EQ(purl.protocol, URL_PROTOCOL_UNKNOWN);
        EXPECT_STREQ(purl.user, "user");
        EXPECT_STREQ(purl.passwd, "mypass");
        EXPECT_EQ(purl.port, 80);
        EXPECT_STREQ(purl.file, "");
        EXPECT_STREQ(purl.host, "yikes.example.com");
    }

    TEST_F(test_url, is_https) {
        EXPECT_EQ(is_https("hello"), false);
        EXPECT_EQ(is_https("https://www.example.com"), true);
        EXPECT_EQ(is_https("http://www.example.com"), false);
        EXPECT_EQ(is_https("xhttps://www.example.com"), false);
    }

    TEST_F(test_url, escape_url) {
        string temp1 = "aeiou äëïöü áéíóú àèìòù";
        string temp1_answer = "aeiou%20%C3%A4%C3%AB%C3%AF%C3%B6%C3%BC%20%C3%A1%C3%A9%C3%AD%C3%B3%C3%BA%20%C3%A0%C3%A8%C3%AC%C3%B2%C3%B9";
        string temp2 = "Epäjärjestelmällistyttämättömyydellänsäkäänköhän";
        string temp2_answer = "Ep%C3%A4j%C3%A4rjestelm%C3%A4llistytt%C3%A4m%C3%A4tt%C3%B6myydell%C3%A4ns%C3%A4k%C3%A4%C3%A4nk%C3%B6h%C3%A4n";

        escape_url(temp1);
        EXPECT_EQ(temp1, temp1_answer);

        escape_url(temp2);
        EXPECT_EQ(temp2, temp2_answer);
    }

    TEST_F(test_url, unescape_url) {
        string temp1 = "aeiou%20%C3%A4%C3%AB%C3%AF%C3%B6%C3%BC%20%C3%A1%C3%A9%C3%AD%C3%B3%C3%BA%20%C3%A0%C3%A8%C3%AC%C3%B2%C3%B9";
        string temp1_answer = "aeiou äëïöü áéíóú àèìòù";
        string temp2 = "Ep%C3%A4j%C3%A4rjestelm%C3%A4llistytt%C3%A4m%C3%A4tt%C3%B6myydell%C3%A4ns%C3%A4k%C3%A4%C3%A4nk%C3%B6h%C3%A4n";
        string temp2_answer = "Epäjärjestelmällistyttämättömyydellänsäkäänköhän";

        unescape_url(temp1);
        EXPECT_EQ(temp1, temp1_answer);

        unescape_url(temp2);
        EXPECT_EQ(temp2, temp2_answer);
    }

    TEST_F(test_url, escape_url_readable) {
        char buf[1024];
        char url[1024];

        strncpy(url, "https://secure.example.com", sizeof (url));
        escape_url_readable(url, buf);
        EXPECT_STREQ(buf, "secure.example.com");

	//Convert the extra / and $ to _
        strncpy(url, "https://money.example.com/Dollar$", sizeof (url));
        escape_url_readable(url, buf);
        EXPECT_STREQ(buf, "money.example.com_Dollar_");

	//Change all spaces and @ % to _
        strncpy(url, "nothing@ should %", sizeof (url));
        escape_url_readable(url, buf);
        EXPECT_STREQ(buf, "nothing__should__");
    }

    TEST_F(test_url, canonicalize_master_url) {
        //Test to make sure an already good result comes back the same.
        string url = "http://secure.example.com/";
        canonicalize_master_url(url);
        EXPECT_STREQ(url.c_str(), "http://secure.example.com/");

        //Test to make sure an already good mixed-case result comes back lower-cased.
        url = "http://SeCuRe.eXamPle.coM/";
        canonicalize_master_url(url);
        EXPECT_STREQ(url.c_str(), "http://secure.example.com/");

        //Test that https works, also adds trailing /.
        url = "https://secure.example.com";
        canonicalize_master_url(url);
        EXPECT_STREQ(url.c_str(), "https://secure.example.com/");

        //Test that https works, in a mixed-case scenario, also adds trailing /.
        url = "https://sEcUre.exaMple.cOm";
        canonicalize_master_url(url);
        EXPECT_STREQ(url.c_str(), "https://secure.example.com/");

        //Test if someone forgot the leading http://.
        url = "www.example.com/";
        canonicalize_master_url(url);
        EXPECT_STREQ(url.c_str(), "http://www.example.com/");

        //Test omitted http:// and mixed case.
        url = "wwW.exaMple.cOm/";
        canonicalize_master_url(url);
        EXPECT_STREQ(url.c_str(), "http://www.example.com/");

        //Test removing extra slashes and changing socks to https.
        url = "socks://sock.example.com////////hello";
        canonicalize_master_url(url);
        EXPECT_STREQ(url.c_str(), "https://sock.example.com/hello/");

        //Test removing extra slashes and changing socks to https, in a mixed-case scenario.
        //Mixed-case characters after the domain name remain unaffected.
        url = "sOcks://Sock.exaMPle.com////////hElLO";
        canonicalize_master_url(url);
        EXPECT_STREQ(url.c_str(), "https://sock.example.com/hElLO/");

        //Test invalid protocol.
        url = "h://bad.example.com/";
        canonicalize_master_url(url);
        EXPECT_STREQ(url.c_str(), "http://bad.example.com/");

        //Test invalid protocol and mixed case.
        url = "H://baD.exampLE.Com/";
        canonicalize_master_url(url);
        EXPECT_STREQ(url.c_str(), "http://bad.example.com/");
    }

    TEST_F(test_url, valid_master_url) {
        char url[1024];

        //Check for a good unsecure url.
        strncpy(url, "http://www.example.com/", sizeof (url));
        EXPECT_EQ(valid_master_url(url), true);

        //Check for a good secure url
        strncpy(url, "https://www.example.com/", sizeof (url));
        EXPECT_EQ(valid_master_url(url), true);

        //Check for no http or https.
        strncpy(url, "hxxp://www.example.com/", sizeof (url));
        EXPECT_EQ(valid_master_url(url), false);

        //Check if missing final slash.
        strncpy(url, "http://www.example.com", sizeof (url));
        EXPECT_EQ(valid_master_url(url), false);

        //Check if it has no . in the name
        strncpy(url, "http://example/", sizeof (url));
        EXPECT_EQ(valid_master_url(url), false);
    }

    TEST_F(test_url, escape_project_url) {
        char buf[1024];
        char url[1024];

        //testing a good url.
        strncpy(url, "https://secure.example.com", sizeof (url));
        escape_project_url(url, buf);
        EXPECT_STREQ(buf, "secure.example.com");

        //Testing url with bad character at the end removed.
        strncpy(url, "https://secure.example.com/Dollar$", sizeof (url));
        escape_project_url(url, buf);
        EXPECT_STREQ(buf, "secure.example.com_Dollar");
    }

}

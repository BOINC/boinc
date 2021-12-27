// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2020 University of California
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

#include <iostream>
#include <fstream>
#include "gtest/gtest.h"
#include "project_init.h"

using namespace std;

namespace test_project_init {

    // The fixture for testing class Foo.

    class test_project_init : public ::testing::Test {
    protected:
        std::string emptyFile = R"xxx(<project_init>
  <url></url>
  <name></name>
  <account_key></account_key>
  <embedded>0</embedded>
</project_init>
)xxx";
        std::string exampleFile1 = R"xxx(<project_init>
  <url>http://www.example.com</url>
  <name>Example Project</name>
  <account_key>1234567890abcdefghijklmnopqrstuvwxyz</account_key>
  <embedded>0</embedded>
</project_init>
)xxx";
        std::string exampleFile2 = R"xxx(<project_init>
  <url>https://secure.example.com</url>
  <name>Secure Example Project</name>
  <account_key>zyxwvutsrqponmlkjihgfedcba0987654321</account_key>
  <embedded>1</embedded>
</project_init>
)xxx";
        // You can remove any or all of the following functions if its body
        // is empty.

        test_project_init() {
            // You can do set-up work for each test here.

        }

        virtual ~test_project_init() {
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

    TEST_F(test_project_init, init) {
        PROJECT_INIT pi;
        int result = pi.init();
        EXPECT_EQ(result, 0);
        EXPECT_STREQ(pi.url, "");
        EXPECT_STREQ(pi.name, "");
        EXPECT_STREQ(pi.account_key, "");
        EXPECT_EQ(pi.embedded, false);

        std::ofstream ofs ("project_init.xml", std::ofstream::out);
        ofs << exampleFile2;
        ofs.close();
        result = pi.init();
        EXPECT_EQ(result, 0);
        EXPECT_STREQ(pi.url, "https://secure.example.com/");
        EXPECT_STREQ(pi.name, "Secure Example Project");
        EXPECT_STREQ(pi.account_key, "zyxwvutsrqponmlkjihgfedcba0987654321");
        EXPECT_EQ(pi.embedded, true);

        std::remove("project_init.xml"); // delete file
    }

    TEST_F(test_project_init, write) {
        PROJECT_INIT pi;
        pi.init();
        int result = pi.write();
        EXPECT_EQ(result, 0);
        std::ifstream t("project_init.xml");
        std::string genfile((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
        EXPECT_EQ(emptyFile, genfile);

        strncpy(pi.url, "http://www.example.com", sizeof (pi.url));
        strncpy(pi.name, "Example Project", sizeof (pi.name));
        strncpy(pi.account_key, "1234567890abcdefghijklmnopqrstuvwxyz", sizeof (pi.account_key));
        result = pi.write();
        EXPECT_EQ(result, 0);
        std::ifstream t2("project_init.xml");
        std::string genfile2((std::istreambuf_iterator<char>(t2)), std::istreambuf_iterator<char>());
        EXPECT_EQ(exampleFile1, genfile2);
        std::remove("project_init.xml"); // delete file
    }

    TEST_F(test_project_init, remove) {
        PROJECT_INIT pi;
        pi.init();
        pi.write();
        int result = pi.remove();
        EXPECT_EQ(result, 0);
        std::ifstream t("project_init.xml");
        EXPECT_EQ(t.is_open(), false);
        result = pi.remove();
        EXPECT_EQ(result, 0);
    }




} // namespace

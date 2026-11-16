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
#include <filesystem>
#include "gtest/gtest.h"
#endif

#include "project_init.h"

using namespace std;

namespace test_lib {
    class test_project_init : public ::testing::Test {
    protected:
        ~test_project_init() override {
            if (filesystem::exists(xml_path)) {
                filesystem::remove(xml_path);
            }
        }

        const filesystem::path xml_path =
            filesystem::current_path() / "project_init.xml";
    };

    TEST_F(test_project_init, test_default_values_empty) {
        PROJECT_INIT pi;
        EXPECT_TRUE(pi.name.empty());
        EXPECT_TRUE(pi.url.empty());
        EXPECT_TRUE(pi.account_key.empty());
        EXPECT_FALSE(pi.embedded);
    }

    TEST_F(test_project_init, test_empty_values_after_clear) {
        PROJECT_INIT pi;
        pi.name = "name";
        pi.url = "https://www.test.com";
        pi.account_key = "qwerty";
        pi.embedded = true;

        pi.clear();

        EXPECT_TRUE(pi.name.empty());
        EXPECT_TRUE(pi.url.empty());
        EXPECT_TRUE(pi.account_key.empty());
        EXPECT_FALSE(pi.embedded);
    }

    TEST_F(test_project_init, test_write_read_file) {
        PROJECT_INIT pi;
        pi.name = "name";
        pi.url = "https://www.test.com";
        pi.account_key = "qwerty";
        pi.embedded = true;

        pi.write();

        ASSERT_TRUE(filesystem::exists(xml_path));

        PROJECT_INIT pi1;
        pi1.init();

        EXPECT_EQ("name", pi1.name);
        // checks also that the url was canonicalized
        EXPECT_EQ("https://www.test.com/", pi1.url);
        EXPECT_EQ("qwerty", pi1.account_key);
        EXPECT_TRUE(pi1.embedded);
    }

    TEST_F(test_project_init, test_init_no_file_expect_default_values) {
        PROJECT_INIT pi;
        pi.init();

        EXPECT_TRUE(pi.name.empty());
        EXPECT_TRUE(pi.url.empty());
        EXPECT_TRUE(pi.account_key.empty());
        EXPECT_FALSE(pi.embedded);
    }

    TEST_F(test_project_init, test_remove_existing_file) {
        PROJECT_INIT pi;
        auto res = pi.write();

        ASSERT_TRUE(filesystem::exists(xml_path));
        ASSERT_EQ(0, res);

        res = pi.remove();

        ASSERT_FALSE(filesystem::exists(xml_path));
        ASSERT_EQ(0, res);
    }

    TEST_F(test_project_init, test_remove_nonexisting_file) {
        PROJECT_INIT pi;
        ASSERT_FALSE(filesystem::exists(xml_path));
        const auto res = pi.remove();
        ASSERT_FALSE(filesystem::exists(xml_path));
        ASSERT_EQ(0, res);
    }
}

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

#include "gtest/gtest.h"
#include "hostinfo.h"

#include <cstdio>
#include <string>

namespace test_hostinfo_linux {
    class test_hostinfo_linux : public ::testing::Test {};

    TEST_F(test_hostinfo_linux, parse_linux_os_info_os_release_ubuntu) {
        std::string fixture_path(__FILE__);
        fixture_path = fixture_path.substr(0, fixture_path.find_last_of('/')) + "/../testdata/os-release.ubuntu";

        FILE* os_release = fopen(fixture_path.c_str(), "r");
        ASSERT_NE(nullptr, os_release);

        char os_name[256] = "", os_version[256] = "";
        EXPECT_TRUE(HOST_INFO::parse_linux_os_info(os_release, osrelease, os_name, sizeof(os_name), os_version, sizeof(os_version)));
        fclose(os_release);

        EXPECT_STREQ("Ubuntu", os_name);
        EXPECT_STREQ("Ubuntu 26.04 LTS", os_version);
    }

    TEST_F(test_hostinfo_linux, parse_linux_os_info_os_release_openwrt) {
        std::string fixture_path(__FILE__);
        fixture_path = fixture_path.substr(0, fixture_path.find_last_of('/')) + "/../testdata/os-release.openwrt";

        FILE* os_release = fopen(fixture_path.c_str(), "r");
        ASSERT_NE(nullptr, os_release);

        char os_name[256] = "", os_version[256] = "";
        EXPECT_TRUE(HOST_INFO::parse_linux_os_info(os_release, osrelease, os_name, sizeof(os_name), os_version, sizeof(os_version)));
        fclose(os_release);

        EXPECT_STREQ("OpenWrt", os_name);
        EXPECT_STREQ("OpenWrt 25.12.4", os_version);
    }
}

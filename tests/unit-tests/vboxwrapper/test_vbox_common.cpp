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

#include "app_ipc.h"
#include "parse.h"
#include "floppyio.h"

#include "vbox_common.h"

// Global variables needed by vbox_common.cpp
std::string slot_dir_path;
std::string project_dir_path;
APP_INIT_DATA aid;

namespace test_vbox_common {
    class test_vbox_common : public ::testing::Test {};

    TEST_F(test_vbox_common, test_is_virtualbox_version_newer) {
        VBOX_BASE vbox_base;
        vbox_base.virtualbox_version_raw = "1.2.3";
        EXPECT_TRUE(vbox_base.is_virtualbox_version_newer(0, 9, 9));
        EXPECT_TRUE(vbox_base.is_virtualbox_version_newer(1, 0, 0));
        EXPECT_TRUE(vbox_base.is_virtualbox_version_newer(1, 1, 9));
        EXPECT_TRUE(vbox_base.is_virtualbox_version_newer(1, 2, 2));
        EXPECT_FALSE(vbox_base.is_virtualbox_version_newer(1, 2, 3));
        EXPECT_FALSE(vbox_base.is_virtualbox_version_newer(1, 2, 4));
        EXPECT_FALSE(vbox_base.is_virtualbox_version_newer(2, 0, 0));
    }
}

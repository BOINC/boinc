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

#include "boinccas_helper.h"
#include "registry_helper.h"

namespace test_boinccas_CAValidateSetupType {
    class test_boinccas_CAValidateSetupType :
        public test_boinccas_TestBase {
    protected:
        test_boinccas_CAValidateSetupType() :
            test_boinccas_TestBase("ValidateSetupType") {
        }
    };

#ifdef BOINCCAS_TEST
    TEST_F(test_boinccas_CAValidateSetupType,
        No_Properties_Set_Expect_Default) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_EQ(0u, executeAction());

        auto [errorcode, value] = getMsiProperty("INSTALLDIR");
        ASSERT_EQ(0u, errorcode);
        EXPECT_EQ("C:\\Program Files\\BOINC\\", value);

        std::tie(errorcode, value) = getMsiProperty("DATADIR");
        ASSERT_EQ(0u, errorcode);
        EXPECT_EQ("C:\\ProgramData\\BOINC\\", value);
    }

    TEST_F(test_boinccas_CAValidateSetupType,
        INSTALLDIR_Set) {
        auto dir = std::filesystem::current_path() / "test_install_dir";

        insertMsiProperties({
            {"INSTALLDIR", dir.string()}
            });

        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_EQ(0u, executeAction());

        auto [errorcode, value] = getMsiProperty("INSTALLDIR");
        ASSERT_EQ(0u, errorcode);
        EXPECT_EQ(dir.string(), value);

        std::tie(errorcode, value) = getMsiProperty("DATADIR");
        ASSERT_EQ(0u, errorcode);
        EXPECT_EQ("C:\\ProgramData\\BOINC\\", value);
    }

    TEST_F(test_boinccas_CAValidateSetupType,
        DATADIR_Set) {
        auto dir = std::filesystem::current_path() / "test_data_dir";

        insertMsiProperties({
            {"DATADIR", dir.string()}
            });

        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_EQ(0u, executeAction());

        auto [errorcode, value] = getMsiProperty("INSTALLDIR");
        ASSERT_EQ(0u, errorcode);
        EXPECT_EQ("C:\\Program Files\\BOINC\\", value);

        std::tie(errorcode, value) = getMsiProperty("DATADIR");
        ASSERT_EQ(0u, errorcode);
        EXPECT_EQ(dir.string(), value);
    }

    TEST_F(test_boinccas_CAValidateSetupType,
        INSTALLDIR_And_DATADIR_Set) {
        auto installdir = std::filesystem::current_path() / "test_install_dir";
        auto datadir = std::filesystem::current_path() / "test_data_dir";

        insertMsiProperties({
            {"INSTALLDIR", installdir.string()},
            {"DATADIR", datadir.string()}
            });

        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_EQ(0u, executeAction());

        auto [errorcode, value] = getMsiProperty("INSTALLDIR");
        ASSERT_EQ(0u, errorcode);
        EXPECT_EQ(installdir.string(), value);

        std::tie(errorcode, value) = getMsiProperty("DATADIR");
        ASSERT_EQ(0u, errorcode);
        EXPECT_EQ(datadir.string(), value);
    }
#endif
}

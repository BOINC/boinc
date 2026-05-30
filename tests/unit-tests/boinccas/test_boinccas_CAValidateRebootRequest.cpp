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

namespace test_boinccas {
    class test_boinccas_CAValidateRebootRequest :
        public test_boinccas_TestBase {
    protected:
        test_boinccas_CAValidateRebootRequest() :
            test_boinccas_TestBase("ValidateRebootRequest") {
        }

        void TearDown() override {
            if (!testDir.empty() && std::filesystem::exists(testDir)) {
                std::filesystem::remove_all(testDir);
            }
            cleanRegistryValue(HKEY_LOCAL_MACHINE,
                "SYSTEM\\CurrentControlSet\\Control\\Session Manager",
                "PendingFileRenameOperations");
        }

        std::filesystem::path testDir;
    };

#ifdef BOINCCAS_TEST
    TEST_F(test_boinccas_CAValidateRebootRequest,
        NoProperties_Expect_Fail) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CAValidateRebootRequest,
        NonExistent_InstallDirectory_Expect_Fail) {
        const auto dir =
            std::filesystem::current_path() / "non_existent_directory";
        insertMsiProperties({
            {"INSTALLDIR", dir.string()}
            });

        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CAValidateRebootRequest,
        File_Exists_and_Removal_Pending) {
        cleanRegistryValue(HKEY_LOCAL_MACHINE,
            "SYSTEM\\CurrentControlSet\\Control\\Session Manager",
            "PendingFileRenameOperations");
        ASSERT_TRUE(getRegistryValue<std::string>(HKEY_LOCAL_MACHINE,
            "SYSTEM\\CurrentControlSet\\Control\\Session Manager",
            "PendingFileRenameOperations").empty());

        testDir =
            std::filesystem::current_path() / "test_directory";
        insertMsiProperties({
            {"INSTALLDIR", testDir.string()}
            });
        std::filesystem::create_directory(testDir);

        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_EQ(0u, executeAction());

        const auto rebootPendingFile = testDir / "RebootPending.txt";
        ASSERT_TRUE(std::filesystem::exists(rebootPendingFile));

        const auto files = getRegistryValue<std::string>(HKEY_LOCAL_MACHINE,
            "SYSTEM\\CurrentControlSet\\Control\\Session Manager",
            "PendingFileRenameOperations");
        EXPECT_FALSE(
            files.find(rebootPendingFile.string()) == std::string::npos);
    }
#endif
}

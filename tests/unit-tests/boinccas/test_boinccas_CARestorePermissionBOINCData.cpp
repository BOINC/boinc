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

#include <AclAPI.h>

namespace test_boinccas_CARestorePermissionBOINCData {
    class test_boinccas_CARestorePermissionBOINCData :
        public test_boinccas_TestBase {
    protected:
        test_boinccas_CARestorePermissionBOINCData() :
            test_boinccas_TestBase("RestorePermissionBOINCData") {
        }

        void TearDown() override {
            if (!testDir.empty() && std::filesystem::exists(testDir)) {
                std::filesystem::remove_all(testDir);
            }
        }

        void createDummyFile(const std::filesystem::path& filePath) {
            std::ofstream ofs(filePath);
            ofs << "dummy content";
            ofs.close();
        }

        int checkPermissions(const std::filesystem::path& path) {
            PACL pACL = nullptr;
            auto result = GetNamedSecurityInfo(path.string().c_str(),
                SE_FILE_OBJECT, DACL_SECURITY_INFORMATION |
                PROTECTED_DACL_SECURITY_INFORMATION, nullptr, nullptr, &pACL,
                nullptr, nullptr);
            if (result != ERROR_SUCCESS || pACL == nullptr) {
                return 1;
            }

            ULONG countOfExplicitEntries = 0;
            PEXPLICIT_ACCESS explicitEntries = nullptr;
            result = GetExplicitEntriesFromAcl(pACL, &countOfExplicitEntries,
                &explicitEntries);
            if (result != ERROR_SUCCESS || explicitEntries == nullptr) {
                return 2;
            }
            wil::unique_any<
                PEXPLICIT_ACCESS, decltype(&::LocalFree), ::LocalFree> ea(
                    explicitEntries);

            DWORD sizeSystem = SECURITY_MAX_SID_SIZE;
            std::array<BYTE, SECURITY_MAX_SID_SIZE> bufferSystem;
            if (!CreateWellKnownSid(WinLocalSystemSid, nullptr, &bufferSystem,
                &sizeSystem)) {
                return 3;
            }
            DWORD sizeAdmin = SECURITY_MAX_SID_SIZE;
            std::array<BYTE, SECURITY_MAX_SID_SIZE> bufferAdmin;
            if (!CreateWellKnownSid(WinBuiltinAdministratorsSid, nullptr,
                &bufferAdmin, &sizeAdmin)) {
                return 4;
            }

            auto foundSystem = false;
            auto foundAdmin = false;
            for (ULONG i = 0; i < countOfExplicitEntries; ++i) {
                const auto& entry = explicitEntries[i];
                auto found = false;
                if (EqualSid(entry.Trustee.ptstrName,
                    reinterpret_cast<PSID>(bufferSystem.data()))) {
                    foundSystem = true;
                    found = true;
                }
                if (EqualSid(entry.Trustee.ptstrName,
                    reinterpret_cast<PSID>(bufferAdmin.data()))) {
                    foundAdmin = true;
                    found = true;
                }
                if (found == false) {
                    continue;
                }

                if ((entry.grfAccessPermissions & FILE_ALL_ACCESS) !=
                    FILE_ALL_ACCESS &&
                    (entry.grfAccessPermissions & GENERIC_ALL) !=
                    GENERIC_ALL) {
                    return 5;
                }
                if (entry.grfAccessMode != GRANT_ACCESS) {
                    return 6;
                }
            }
            if (foundSystem == false) {
                return 7;
            }
            if (foundAdmin == false) {
                return 8;
            }

            return 0;
        }

        std::filesystem::path testDir;
    };

#ifdef BOINCCAS_TEST
    TEST_F(test_boinccas_CARestorePermissionBOINCData,
        Empty_DATADIR_Property) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CARestorePermissionBOINCData,
        Empty_DATADIR_Directory) {
        const auto dir = std::filesystem::current_path() /= "test_data";
        insertMsiProperties({
        {"DATADIR", dir.string()}
            });
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CARestorePermissionBOINCData,
        Test_Permissions_Set) {
        testDir = std::filesystem::current_path() /= "test_data";
        insertMsiProperties({
        {"DATADIR", testDir.string()}
            });

        const std::array folders = {
            testDir / "d1",
            testDir / "d2",
            testDir / "d1" / "d3",
            testDir / "d1" / "d4"
        };
        const std::array files = {
            testDir / "f1",
            testDir / "f2",
            testDir / "d1" / "f3",
            testDir / "d1" / "f4",
            testDir / "d2" / "f5",
            testDir / "d1" / "d3" / "f6",
            testDir / "d1" / "d4" / "f7"
        };

        std::filesystem::create_directories(testDir);
        for (const auto& folder : folders) {
            std::filesystem::create_directories(folder);
        }
        for (const auto& file : files) {
            createDummyFile(file);
        }

        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        ASSERT_EQ(0u, executeAction());

        EXPECT_EQ(0, checkPermissions(testDir));
        for (const auto& folder : folders) {
            EXPECT_EQ(0, checkPermissions(folder));
        }
        for (const auto& file : files) {
            EXPECT_EQ(0, checkPermissions(file));
        }
    }
#endif
}

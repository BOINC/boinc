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
#include "user_group_helper.h"

namespace test_boinccas_CASetPermissionBOINC {
    constexpr auto adminsGroupName = "test_admins";
    constexpr auto usersGroupName = "test_users";

    class test_boinccas_CASetPermissionBOINC :
        public test_boinccas_TestBase {
    protected:
        test_boinccas_CASetPermissionBOINC() :
            test_boinccas_TestBase("SetPermissionBOINC") {
        }

        void TearDown() override {
            if (!testDir.empty() && std::filesystem::exists(testDir)) {
                std::filesystem::remove_all(testDir);
            }
            if (localGroupExists(adminsGroupName)) {
                deleteLocalGroup(adminsGroupName);
            }
            if (localGroupExists(usersGroupName)) {
                deleteLocalGroup(usersGroupName);
            }
        }

        void createDummyFile(const std::filesystem::path& filePath) {
            std::ofstream ofs(filePath);
            ofs << "dummy content";
            ofs.close();
        }

        int checkPermissions(const std::filesystem::path& path,
            bool expectNoAllUsers) {
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

            auto admins_sid = getUserSid(adminsGroupName);
            if (admins_sid == nullptr) {
                return 3;
            }
            auto users_sid = getUserSid(usersGroupName);
            if (users_sid == nullptr) {
                return 4;
            }
            DWORD sizeAllUsers = SECURITY_MAX_SID_SIZE;
            std::array<BYTE, SECURITY_MAX_SID_SIZE> bufferAllUsers;
            if (!CreateWellKnownSid(WinBuiltinUsersSid, nullptr,
                &bufferAllUsers, &sizeAllUsers)) {
                return 5;
            }
            auto foundAdmin = false;
            auto foundUsers = false;
            auto foundAllUsers = false;

            for (ULONG i = 0; i < countOfExplicitEntries; ++i) {
                const auto& entry = explicitEntries[i];
                if (EqualSid(entry.Trustee.ptstrName, admins_sid.get())) {
                    foundAdmin = true;
                    if ((entry.grfAccessPermissions & FILE_ALL_ACCESS) !=
                        FILE_ALL_ACCESS &&
                        (entry.grfAccessPermissions & GENERIC_ALL) !=
                        GENERIC_ALL) {
                        return 6;
                    }
                    if (entry.grfAccessMode != GRANT_ACCESS) {
                        return 7;
                    }
                }
                if (EqualSid(entry.Trustee.ptstrName, users_sid.get())) {
                    foundUsers = true;
                    if ((entry.grfAccessPermissions & FILE_GENERIC_READ) !=
                        FILE_GENERIC_READ &&
                        (entry.grfAccessPermissions & GENERIC_READ) !=
                        GENERIC_READ) {
                        return 6;
                    }
                    if ((entry.grfAccessPermissions & FILE_GENERIC_EXECUTE) !=
                        FILE_GENERIC_EXECUTE &&
                        (entry.grfAccessPermissions & GENERIC_EXECUTE) !=
                        GENERIC_EXECUTE) {
                        return 7;
                    }
                    if (entry.grfAccessMode != GRANT_ACCESS) {
                        return 8;
                    }
                }
                if (EqualSid(entry.Trustee.ptstrName,
                    reinterpret_cast<PSID>(bufferAllUsers.data()))) {
                    foundAllUsers = true;
                    if ((entry.grfAccessPermissions & FILE_GENERIC_READ) !=
                        FILE_GENERIC_READ &&
                        (entry.grfAccessPermissions & GENERIC_READ) !=
                        GENERIC_READ) {
                        return 9;
                    }
                    if ((entry.grfAccessPermissions & FILE_GENERIC_EXECUTE) !=
                        FILE_GENERIC_EXECUTE &&
                        (entry.grfAccessPermissions & GENERIC_EXECUTE) !=
                        GENERIC_EXECUTE) {
                        return 10;
                    }
                    if (entry.grfAccessMode != GRANT_ACCESS) {
                        return 11;
                    }
                }
            }
            if (foundAdmin == false) {
                return 12;
            }
            if (foundUsers == false) {
                return 13;
            }
            if (!expectNoAllUsers && foundAllUsers == false) {
                return 14;
            }

            return 0;
        }

        std::filesystem::path testDir;
    };

#ifdef BOINCCAS_TEST
    TEST_F(test_boinccas_CASetPermissionBOINC,
        Empty_INSTALLDIR_Property_Expect_Fail) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3", "1");
        auto [errorcode, value] =
            getMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        setMsiProperty("BOINC_ADMINS_GROUPNAME", adminsGroupName);
        std::tie(errorcode, value) =
            getMsiProperty("BOINC_ADMINS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(adminsGroupName, value);

        ASSERT_TRUE(createLocalGroup(adminsGroupName));

        setMsiProperty("BOINC_USERS_GROUPNAME", usersGroupName);
        std::tie(errorcode, value) =
            getMsiProperty("BOINC_USERS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(usersGroupName, value);

        ASSERT_TRUE(createLocalGroup(usersGroupName));

        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CASetPermissionBOINC,
        INSTALLDIR_Directory_Doesnt_Exist_Expect_Fail) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3", "1");
        auto [errorcode, value] =
            getMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        setMsiProperty("BOINC_ADMINS_GROUPNAME", adminsGroupName);
        std::tie(errorcode, value) =
            getMsiProperty("BOINC_ADMINS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(adminsGroupName, value);

        ASSERT_TRUE(createLocalGroup(adminsGroupName));

        setMsiProperty("BOINC_USERS_GROUPNAME", usersGroupName);
        std::tie(errorcode, value) =
            getMsiProperty("BOINC_USERS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(usersGroupName, value);

        ASSERT_TRUE(createLocalGroup(usersGroupName));

        testDir = std::filesystem::current_path() /= "test_data";

        setMsiProperty("INSTALLDIR", testDir.string());
        std::tie(errorcode, value) =
            getMsiProperty("INSTALLDIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testDir.string(), value);

        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CASetPermissionBOINC,
        ProtectedMode_Not_Set_Expect_Success) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3", "0");
        auto [errorcode, value] =
            getMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("0", value);

        testDir = std::filesystem::current_path() /= "test_data";
        std::filesystem::create_directories(testDir);

        setMsiProperty("INSTALLDIR", testDir.string());
        std::tie(errorcode, value) =
            getMsiProperty("INSTALLDIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testDir.string(), value);

        EXPECT_EQ(0u, executeAction());
    }

    TEST_F(test_boinccas_CASetPermissionBOINC,
        ProtectedMode_Set_Without_Groups_Expect_Fail) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3", "1");
        auto [errorcode, value] =
            getMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        testDir = std::filesystem::current_path() /= "test_data";
        std::filesystem::create_directories(testDir);

        setMsiProperty("INSTALLDIR", testDir.string());
        std::tie(errorcode, value) =
            getMsiProperty("INSTALLDIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testDir.string(), value);

        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CASetPermissionBOINC,
        ProtectedMode_Set_Without_Admins_Group_Expect_Fail) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3", "1");
        auto [errorcode, value] =
            getMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        setMsiProperty("BOINC_USERS_GROUPNAME", usersGroupName);
        std::tie(errorcode, value) =
            getMsiProperty("BOINC_USERS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(usersGroupName, value);

        testDir = std::filesystem::current_path() /= "test_data";
        std::filesystem::create_directories(testDir);

        setMsiProperty("INSTALLDIR", testDir.string());
        std::tie(errorcode, value) =
            getMsiProperty("INSTALLDIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testDir.string(), value);

        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CASetPermissionBOINC,
        ProtectedMode_Set_Without_Users_Group_Expect_Fail) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3", "1");
        auto [errorcode, value] =
            getMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        setMsiProperty("BOINC_ADMINS_GROUPNAME", adminsGroupName);
        std::tie(errorcode, value) =
            getMsiProperty("BOINC_ADMINS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(adminsGroupName, value);

        testDir = std::filesystem::current_path() /= "test_data";
        std::filesystem::create_directories(testDir);

        setMsiProperty("INSTALLDIR", testDir.string());
        std::tie(errorcode, value) =
            getMsiProperty("INSTALLDIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testDir.string(), value);

        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CASetPermissionBOINC,
        ProtectedMode_Set_Admins_Group_Doesnt_Exist_Expect_Fail) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3", "1");
        auto [errorcode, value] =
            getMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        setMsiProperty("BOINC_ADMINS_GROUPNAME", adminsGroupName);
        std::tie(errorcode, value) =
            getMsiProperty("BOINC_ADMINS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(adminsGroupName, value);

        setMsiProperty("BOINC_USERS_GROUPNAME", usersGroupName);
        std::tie(errorcode, value) =
            getMsiProperty("BOINC_USERS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(usersGroupName, value);

        ASSERT_TRUE(createLocalGroup(usersGroupName));

        testDir = std::filesystem::current_path() /= "test_data";
        std::filesystem::create_directories(testDir);

        setMsiProperty("INSTALLDIR", testDir.string());
        std::tie(errorcode, value) =
            getMsiProperty("INSTALLDIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testDir.string(), value);

        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CASetPermissionBOINC,
        ProtectedMode_Set_Users_Group_Doesnt_Exist_Expect_Fail) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3", "1");
        auto [errorcode, value] =
            getMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        setMsiProperty("BOINC_ADMINS_GROUPNAME", adminsGroupName);
        std::tie(errorcode, value) =
            getMsiProperty("BOINC_ADMINS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(adminsGroupName, value);

        ASSERT_TRUE(createLocalGroup(adminsGroupName));

        setMsiProperty("BOINC_USERS_GROUPNAME", usersGroupName);
        std::tie(errorcode, value) =
            getMsiProperty("BOINC_USERS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(usersGroupName, value);

        testDir = std::filesystem::current_path() /= "test_data";
        std::filesystem::create_directories(testDir);

        setMsiProperty("INSTALLDIR", testDir.string());
        std::tie(errorcode, value) =
            getMsiProperty("INSTALLDIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testDir.string(), value);

        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CASetPermissionBOINC,
        ProtectedMode_Set_Admins_And_Users_Group_Expect_Success) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3", "1");
        auto [errorcode, value] =
            getMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        setMsiProperty("BOINC_ADMINS_GROUPNAME", adminsGroupName);
        std::tie(errorcode, value) =
            getMsiProperty("BOINC_ADMINS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(adminsGroupName, value);

        ASSERT_TRUE(createLocalGroup(adminsGroupName));

        setMsiProperty("BOINC_USERS_GROUPNAME", usersGroupName);
        std::tie(errorcode, value) =
            getMsiProperty("BOINC_USERS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(usersGroupName, value);

        ASSERT_TRUE(createLocalGroup(usersGroupName));

        testDir = std::filesystem::current_path() /= "test_data";
        std::filesystem::create_directories(testDir);

        setMsiProperty("INSTALLDIR", testDir.string());
        std::tie(errorcode, value) =
            getMsiProperty("INSTALLDIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testDir.string(), value);

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

        ASSERT_EQ(0u, executeAction());

        EXPECT_EQ(0, checkPermissions(testDir, true));
        for (const auto& folder : folders) {
            EXPECT_EQ(0, checkPermissions(folder, true));
        }
        for (const auto& file : files) {
            EXPECT_EQ(0, checkPermissions(file, true));
        }
    }

    TEST_F(test_boinccas_CASetPermissionBOINC,
        ProtectedMode_Set_AllUsers_Set_To_0_Expect_Success) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3", "1");
        auto [errorcode, value] =
            getMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        setMsiProperty("BOINC_ADMINS_GROUPNAME", adminsGroupName);
        std::tie(errorcode, value) =
            getMsiProperty("BOINC_ADMINS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(adminsGroupName, value);

        ASSERT_TRUE(createLocalGroup(adminsGroupName));

        setMsiProperty("BOINC_USERS_GROUPNAME", usersGroupName);
        std::tie(errorcode, value) =
            getMsiProperty("BOINC_USERS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(usersGroupName, value);

        ASSERT_TRUE(createLocalGroup(usersGroupName));

        setMsiProperty("ENABLEUSEBYALLUSERS", "0");
        std::tie(errorcode, value) =
            getMsiProperty("ENABLEUSEBYALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("0", value);

        testDir = std::filesystem::current_path() /= "test_data";
        std::filesystem::create_directories(testDir);

        setMsiProperty("INSTALLDIR", testDir.string());
        std::tie(errorcode, value) =
            getMsiProperty("INSTALLDIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testDir.string(), value);

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

        ASSERT_EQ(0u, executeAction());

        EXPECT_EQ(0, checkPermissions(testDir, true));
        for (const auto& folder : folders) {
            EXPECT_EQ(0, checkPermissions(folder, true));
        }
        for (const auto& file : files) {
            EXPECT_EQ(0, checkPermissions(file, true));
        }
    }

    TEST_F(test_boinccas_CASetPermissionBOINC,
        ProtectedMode_Set_AllUsers_Set_Expect_Success) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3", "1");
        auto [errorcode, value] =
            getMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        setMsiProperty("BOINC_ADMINS_GROUPNAME", adminsGroupName);
        std::tie(errorcode, value) =
            getMsiProperty("BOINC_ADMINS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(adminsGroupName, value);

        ASSERT_TRUE(createLocalGroup(adminsGroupName));

        setMsiProperty("BOINC_USERS_GROUPNAME", usersGroupName);
        std::tie(errorcode, value) =
            getMsiProperty("BOINC_USERS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(usersGroupName, value);

        ASSERT_TRUE(createLocalGroup(usersGroupName));

        setMsiProperty("ENABLEUSEBYALLUSERS", "1");
        std::tie(errorcode, value) =
            getMsiProperty("ENABLEUSEBYALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        testDir = std::filesystem::current_path() /= "test_data";
        std::filesystem::create_directories(testDir);

        setMsiProperty("INSTALLDIR", testDir.string());
        std::tie(errorcode, value) =
            getMsiProperty("INSTALLDIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testDir.string(), value);

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

        ASSERT_EQ(0u, executeAction());

        EXPECT_EQ(0, checkPermissions(testDir, false));
        for (const auto& folder : folders) {
            EXPECT_EQ(0, checkPermissions(folder, false));
        }
        for (const auto& file : files) {
            EXPECT_EQ(0, checkPermissions(file, false));
        }
    }
#endif
}

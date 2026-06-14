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

namespace test_boinccas {
    constexpr auto adminsGroupName = "test_admins";
    constexpr auto usersGroupName = "test_users";
    constexpr auto projectsGroupName = "test_projects";

    struct AccountsInfo {
        bool system = false;
        bool admins = false;
        bool currentUser = false;
        bool boinc_admins = false;
        bool boinc_users = false;
        bool boinc_projects = false;
        bool users = false;
        const bool operator==(const AccountsInfo& other) const noexcept {
            return
                system == other.system &&
                admins == other.admins &&
                currentUser == other.currentUser &&
                boinc_admins == other.boinc_admins &&
                boinc_users == other.boinc_users &&
                boinc_projects == other.boinc_projects &&
                users == other.users;
        }
    };

    class test_boinccas_CASetPermissionBOINCData :
        public test_boinccas_TestBase {
    protected:
        test_boinccas_CASetPermissionBOINCData() :
            test_boinccas_TestBase("SetPermissionBOINCData") {
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
            if (localGroupExists(projectsGroupName)) {
                deleteLocalGroup(projectsGroupName);
            }
        }

        void createDummyFile(const std::filesystem::path& filePath) {
            std::ofstream ofs(filePath);
            ofs << "dummy content";
            ofs.close();
        }

        int checkPermissions(const std::filesystem::path& path,
            const AccountsInfo& accountsInfo, bool allUsersRestricted) {
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
            if (admins_sid == nullptr && accountsInfo.boinc_admins) {
                return 3;
            }
            auto users_sid = getUserSid(usersGroupName);
            if (users_sid == nullptr && accountsInfo.boinc_users) {
                return 4;
            }
            auto projects_sid = getUserSid(projectsGroupName);
            if (projects_sid == nullptr && accountsInfo.boinc_projects) {
                return 5;
            }
            auto currentUserSid = getCurrentUserSid();
            if (currentUserSid == nullptr && accountsInfo.currentUser) {
                return 6;
            }

            DWORD sizeAllUsers = SECURITY_MAX_SID_SIZE;
            std::array<BYTE, SECURITY_MAX_SID_SIZE> bufferAllUsers;
            if (!CreateWellKnownSid(WinBuiltinUsersSid, nullptr,
                &bufferAllUsers, &sizeAllUsers)) {
                return 7;
            }
            DWORD sizeSystem = SECURITY_MAX_SID_SIZE;
            std::array<BYTE, SECURITY_MAX_SID_SIZE> bufferSystem;
            if (!CreateWellKnownSid(WinLocalSystemSid, nullptr,
                &bufferSystem, &sizeSystem)) {
                return 7;
            }
            DWORD sizeAdmins = SECURITY_MAX_SID_SIZE;
            std::array<BYTE, SECURITY_MAX_SID_SIZE> bufferAdmins;
            if (!CreateWellKnownSid(WinBuiltinAdministratorsSid, nullptr,
                &bufferAdmins, &sizeAdmins)) {
                return 9;
            }

            AccountsInfo foundAccountsInfo;

            for (ULONG i = 0; i < countOfExplicitEntries; ++i) {
                const auto& entry = explicitEntries[i];
                if (admins_sid &&
                    EqualSid(entry.Trustee.ptstrName, admins_sid.get())) {
                    foundAccountsInfo.boinc_admins = true;
                    if ((entry.grfAccessPermissions & FILE_ALL_ACCESS) !=
                        FILE_ALL_ACCESS &&
                        (entry.grfAccessPermissions & GENERIC_ALL) !=
                        GENERIC_ALL) {
                        return 10;
                    }
                    if (entry.grfAccessMode != GRANT_ACCESS) {
                        return 11;
                    }
                }
                if (users_sid &&
                    EqualSid(entry.Trustee.ptstrName, users_sid.get())) {
                    foundAccountsInfo.boinc_users = true;
                    if (((entry.grfAccessPermissions & FILE_EXECUTE) !=
                        FILE_EXECUTE) &&
                        !((entry.grfAccessPermissions & GENERIC_EXECUTE) &&
                            (entry.grfAccessPermissions & GENERIC_READ))) {
                        return 12;
                    }
                    if (entry.grfAccessMode != GRANT_ACCESS) {
                        return 13;
                    }
                }
                if (projects_sid &&
                    EqualSid(entry.Trustee.ptstrName, projects_sid.get())) {
                    foundAccountsInfo.boinc_projects = true;
                    if ((entry.grfAccessPermissions & FILE_TRAVERSE)
                        != FILE_TRAVERSE) {
                        return 14;
                    }
                    if (entry.grfAccessMode != GRANT_ACCESS) {
                        return 15;
                    }
                }
                if (currentUserSid &&
                    EqualSid(entry.Trustee.ptstrName, currentUserSid.get())) {
                    foundAccountsInfo.currentUser = true;
                    if ((entry.grfAccessPermissions & FILE_ALL_ACCESS) !=
                        FILE_ALL_ACCESS &&
                        (entry.grfAccessPermissions & GENERIC_ALL) !=
                        GENERIC_ALL) {
                        return 16;
                    }
                    if (entry.grfAccessMode != GRANT_ACCESS) {
                        return 17;
                    }
                }
                if (EqualSid(entry.Trustee.ptstrName,
                    reinterpret_cast<PSID>(bufferAllUsers.data()))) {
                    foundAccountsInfo.users = true;
                    if (allUsersRestricted) {
                        if (((entry.grfAccessPermissions & FILE_EXECUTE) !=
                            FILE_EXECUTE) &&
                            !((entry.grfAccessPermissions & GENERIC_EXECUTE) &&
                                (entry.grfAccessPermissions & GENERIC_READ))) {
                            return 18;
                        }
                    }
                    else {
                        if ((entry.grfAccessPermissions & FILE_ALL_ACCESS) !=
                            FILE_ALL_ACCESS &&
                            (entry.grfAccessPermissions & GENERIC_ALL) !=
                            GENERIC_ALL) {
                            return 19;
                        }
                    }
                    if (entry.grfAccessMode != GRANT_ACCESS) {
                        return 20;
                    }
                }
                if (EqualSid(entry.Trustee.ptstrName,
                    reinterpret_cast<PSID>(bufferSystem.data()))) {
                    foundAccountsInfo.system = true;
                    if ((entry.grfAccessPermissions & FILE_ALL_ACCESS) !=
                        FILE_ALL_ACCESS &&
                        (entry.grfAccessPermissions & GENERIC_ALL) !=
                        GENERIC_ALL) {
                        return 21;
                    }
                    if (entry.grfAccessMode != GRANT_ACCESS) {
                        return 22;
                    }
                }
                if (EqualSid(entry.Trustee.ptstrName,
                    reinterpret_cast<PSID>(bufferAdmins.data()))) {
                    foundAccountsInfo.admins = true;
                    if ((entry.grfAccessPermissions & FILE_ALL_ACCESS) !=
                        FILE_ALL_ACCESS &&
                        (entry.grfAccessPermissions & GENERIC_ALL) !=
                        GENERIC_ALL) {
                        return 23;
                    }
                    if (entry.grfAccessMode != GRANT_ACCESS) {
                        return 24;
                    }
                }
            }

            EXPECT_EQ(accountsInfo.system, foundAccountsInfo.system);
            EXPECT_EQ(accountsInfo.admins, foundAccountsInfo.admins);
            EXPECT_EQ(accountsInfo.currentUser,
                foundAccountsInfo.currentUser);
            EXPECT_EQ(accountsInfo.boinc_admins,
                foundAccountsInfo.boinc_admins);
            EXPECT_EQ(accountsInfo.boinc_users,
                foundAccountsInfo.boinc_users);
            EXPECT_EQ(accountsInfo.boinc_projects,
                foundAccountsInfo.boinc_projects);
            EXPECT_EQ(accountsInfo.users, foundAccountsInfo.users);

            return accountsInfo == foundAccountsInfo ? 0 : -1;
        }

        std::filesystem::path testDir;
    };

#ifdef BOINCCAS_TEST
    TEST_F(test_boinccas_CASetPermissionBOINCData,
        ProtectedMode_Empty_DATADIR_Property_Expect_Fail) {
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

        setMsiProperty("BOINC_PROJECTS_GROUPNAME", projectsGroupName);
        std::tie(errorcode, value) =
            getMsiProperty("BOINC_PROJECTS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(projectsGroupName, value);

        ASSERT_TRUE(createLocalGroup(projectsGroupName));

        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CASetPermissionBOINCData,
        NormalMode_Empty_DATADIR_Property_Expect_Fail) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3", "0");
        auto [errorcode, value] =
            getMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("0", value);

        const auto userSid = getCurrentUserSidString();
        setMsiProperty("UserSID", userSid);
        std::tie(errorcode, value) =
            getMsiProperty("UserSID");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(userSid, value);

        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CASetPermissionBOINCData,
        ProtectedMode_DATADIR_Doesnt_Exist_Expect_Fail) {
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

        setMsiProperty("BOINC_PROJECTS_GROUPNAME", projectsGroupName);
        std::tie(errorcode, value) =
            getMsiProperty("BOINC_PROJECTS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(projectsGroupName, value);

        ASSERT_TRUE(createLocalGroup(projectsGroupName));

        testDir = std::filesystem::current_path() /= "test_data";

        setMsiProperty("DATADIR", testDir.string());
        std::tie(errorcode, value) =
            getMsiProperty("DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testDir.string(), value);

        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CASetPermissionBOINCData,
        NormalMode_DATADIR_Doesnt_Exist_Expect_Fail) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3", "0");
        auto [errorcode, value] =
            getMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("0", value);

        const auto userSid = getCurrentUserSidString();
        setMsiProperty("UserSID", userSid);
        std::tie(errorcode, value) =
            getMsiProperty("UserSID");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(userSid, value);

        testDir = std::filesystem::current_path() /= "test_data";

        setMsiProperty("DATADIR", testDir.string());
        std::tie(errorcode, value) =
            getMsiProperty("DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testDir.string(), value);

        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CASetPermissionBOINCData,
        ProtectedMode_Without_Admins_Group_Expect_Fail) {
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

        ASSERT_TRUE(createLocalGroup(usersGroupName));

        setMsiProperty("BOINC_PROJECTS_GROUPNAME", projectsGroupName);
        std::tie(errorcode, value) =
            getMsiProperty("BOINC_PROJECTS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(projectsGroupName, value);

        ASSERT_TRUE(createLocalGroup(projectsGroupName));

        testDir = std::filesystem::current_path() /= "test_data";
        std::filesystem::create_directories(testDir);

        setMsiProperty("DATADIR", testDir.string());
        std::tie(errorcode, value) =
            getMsiProperty("DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testDir.string(), value);

        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CASetPermissionBOINCData,
        ProtectedMode_Without_Users_Group_Expect_Fail) {
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

        setMsiProperty("BOINC_PROJECTS_GROUPNAME", projectsGroupName);
        std::tie(errorcode, value) =
            getMsiProperty("BOINC_PROJECTS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(projectsGroupName, value);

        ASSERT_TRUE(createLocalGroup(projectsGroupName));

        testDir = std::filesystem::current_path() /= "test_data";
        std::filesystem::create_directories(testDir);

        setMsiProperty("DATADIR", testDir.string());
        std::tie(errorcode, value) =
            getMsiProperty("DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testDir.string(), value);

        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CASetPermissionBOINCData,
        ProtectedMode_Without_Projects_Group_Expect_Fail) {
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

        setMsiProperty("DATADIR", testDir.string());
        std::tie(errorcode, value) =
            getMsiProperty("DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testDir.string(), value);

        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CASetPermissionBOINCData,
        ProtectedMode_Admins_Group_Doesnt_Exist_Expect_Fail) {
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

        setMsiProperty("BOINC_PROJECTS_GROUPNAME", projectsGroupName);
        std::tie(errorcode, value) =
            getMsiProperty("BOINC_PROJECTS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(projectsGroupName, value);

        ASSERT_TRUE(createLocalGroup(projectsGroupName));

        testDir = std::filesystem::current_path() /= "test_data";
        std::filesystem::create_directories(testDir);

        setMsiProperty("DATADIR", testDir.string());
        std::tie(errorcode, value) =
            getMsiProperty("DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testDir.string(), value);

        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CASetPermissionBOINCData,
        ProtectedMode_Users_Group_Doesnt_Exist_Expect_Fail) {
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

        setMsiProperty("BOINC_PROJECTS_GROUPNAME", projectsGroupName);
        std::tie(errorcode, value) =
            getMsiProperty("BOINC_PROJECTS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(projectsGroupName, value);

        ASSERT_TRUE(createLocalGroup(projectsGroupName));

        testDir = std::filesystem::current_path() /= "test_data";
        std::filesystem::create_directories(testDir);

        setMsiProperty("DATADIR", testDir.string());
        std::tie(errorcode, value) =
            getMsiProperty("DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testDir.string(), value);

        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CASetPermissionBOINCData,
        ProtectedMode_Projects_Group_Doesnt_Exist_Expect_Fail) {
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

        setMsiProperty("BOINC_PROJECTS_GROUPNAME", projectsGroupName);
        std::tie(errorcode, value) =
            getMsiProperty("BOINC_PROJECTS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(projectsGroupName, value);

        testDir = std::filesystem::current_path() /= "test_data";
        std::filesystem::create_directories(testDir);

        setMsiProperty("DATADIR", testDir.string());
        std::tie(errorcode, value) =
            getMsiProperty("DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testDir.string(), value);

        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CASetPermissionBOINCData,
        ProtectedMode_AllUsers_Not_Set_Expect_Success) {
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

        setMsiProperty("BOINC_PROJECTS_GROUPNAME", projectsGroupName);
        std::tie(errorcode, value) =
            getMsiProperty("BOINC_PROJECTS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(projectsGroupName, value);

        ASSERT_TRUE(createLocalGroup(projectsGroupName));

        testDir = std::filesystem::current_path() /= "test_data";
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

        setMsiProperty("DATADIR", testDir.string());
        std::tie(errorcode, value) =
            getMsiProperty("DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testDir.string(), value);

        EXPECT_EQ(0u, executeAction());

        AccountsInfo accountsInfo;
        accountsInfo.system = true;
        accountsInfo.admins = true;
        accountsInfo.boinc_admins = true;
        accountsInfo.boinc_users = true;
        accountsInfo.boinc_projects = true;

        EXPECT_EQ(0, checkPermissions(testDir, accountsInfo, true));
        for (const auto& folder : folders) {
            EXPECT_EQ(0, checkPermissions(folder, accountsInfo, true));
        }
        for (const auto& file : files) {
            EXPECT_EQ(0, checkPermissions(file, accountsInfo, true));
        }
    }

    TEST_F(test_boinccas_CASetPermissionBOINCData,
        ProtectedMode_AllUsers_Set_Expect_Success) {
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

        setMsiProperty("BOINC_PROJECTS_GROUPNAME", projectsGroupName);
        std::tie(errorcode, value) =
            getMsiProperty("BOINC_PROJECTS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(projectsGroupName, value);

        ASSERT_TRUE(createLocalGroup(projectsGroupName));

        setMsiProperty("ENABLEUSEBYALLUSERS", "1");
        std::tie(errorcode, value) =
            getMsiProperty("ENABLEUSEBYALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        testDir = std::filesystem::current_path() /= "test_data";
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

        setMsiProperty("DATADIR", testDir.string());
        std::tie(errorcode, value) =
            getMsiProperty("DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testDir.string(), value);

        EXPECT_EQ(0u, executeAction());

        AccountsInfo accountsInfo;
        accountsInfo.system = true;
        accountsInfo.admins = true;
        accountsInfo.users = true;
        accountsInfo.boinc_admins = true;
        accountsInfo.boinc_users = true;
        accountsInfo.boinc_projects = true;

        EXPECT_EQ(0, checkPermissions(testDir, accountsInfo, true));
        for (const auto& folder : folders) {
            EXPECT_EQ(0, checkPermissions(folder, accountsInfo, true));
        }
        for (const auto& file : files) {
            EXPECT_EQ(0, checkPermissions(file, accountsInfo, true));
        }
    }

    TEST_F(test_boinccas_CASetPermissionBOINCData,
        ProtectedMode_AllUsers_Disabled_Expect_Success) {
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

        setMsiProperty("BOINC_PROJECTS_GROUPNAME", projectsGroupName);
        std::tie(errorcode, value) =
            getMsiProperty("BOINC_PROJECTS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(projectsGroupName, value);

        ASSERT_TRUE(createLocalGroup(projectsGroupName));

        setMsiProperty("ENABLEUSEBYALLUSERS", "0");
        std::tie(errorcode, value) =
            getMsiProperty("ENABLEUSEBYALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("0", value);

        testDir = std::filesystem::current_path() /= "test_data";
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

        setMsiProperty("DATADIR", testDir.string());
        std::tie(errorcode, value) =
            getMsiProperty("DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testDir.string(), value);

        EXPECT_EQ(0u, executeAction());

        AccountsInfo accountsInfo;
        accountsInfo.system = true;
        accountsInfo.admins = true;
        accountsInfo.boinc_admins = true;
        accountsInfo.boinc_users = true;
        accountsInfo.boinc_projects = true;

        EXPECT_EQ(0, checkPermissions(testDir, accountsInfo, true));
        for (const auto& folder : folders) {
            EXPECT_EQ(0, checkPermissions(folder, accountsInfo, true));
        }
        for (const auto& file : files) {
            EXPECT_EQ(0, checkPermissions(file, accountsInfo, true));
        }
    }

    TEST_F(test_boinccas_CASetPermissionBOINCData,
        NormalMode_AllUsers_Not_Set_Expect_Success) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3", "0");
        auto [errorcode, value] =
            getMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("0", value);

        const auto userSid = getCurrentUserSidString();
        setMsiProperty("UserSID", userSid);
        std::tie(errorcode, value) =
            getMsiProperty("UserSID");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(userSid, value);

        testDir = std::filesystem::current_path() /= "test_data";
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

        setMsiProperty("DATADIR", testDir.string());
        std::tie(errorcode, value) =
            getMsiProperty("DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testDir.string(), value);

        EXPECT_EQ(0u, executeAction());

        AccountsInfo accountsInfo;
        accountsInfo.system = true;
        accountsInfo.admins = true;
        accountsInfo.currentUser = true;

        EXPECT_EQ(0, checkPermissions(testDir, accountsInfo, false));
        for (const auto& folder : folders) {
            EXPECT_EQ(0, checkPermissions(folder, accountsInfo, false));
        }
        for (const auto& file : files) {
            EXPECT_EQ(0, checkPermissions(file, accountsInfo, false));
        }
    }

    TEST_F(test_boinccas_CASetPermissionBOINCData,
        NormalMode_AllUsers_Set_Expect_Success) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3", "0");
        auto [errorcode, value] =
            getMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("0", value);

        const auto userSid = getCurrentUserSidString();
        setMsiProperty("UserSID", userSid);
        std::tie(errorcode, value) =
            getMsiProperty("UserSID");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(userSid, value);

        setMsiProperty("ENABLEUSEBYALLUSERS", "1");
        std::tie(errorcode, value) =
            getMsiProperty("ENABLEUSEBYALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        testDir = std::filesystem::current_path() /= "test_data";
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

        setMsiProperty("DATADIR", testDir.string());
        std::tie(errorcode, value) =
            getMsiProperty("DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testDir.string(), value);

        EXPECT_EQ(0u, executeAction());

        AccountsInfo accountsInfo;
        accountsInfo.system = true;
        accountsInfo.admins = true;
        accountsInfo.users = true;
        accountsInfo.currentUser = true;

        EXPECT_EQ(0, checkPermissions(testDir, accountsInfo, false));
        for (const auto& folder : folders) {
            EXPECT_EQ(0, checkPermissions(folder, accountsInfo, false));
        }
        for (const auto& file : files) {
            EXPECT_EQ(0, checkPermissions(file, accountsInfo, false));
        }
    }

    TEST_F(test_boinccas_CASetPermissionBOINCData,
        NormalMode_AllUsers_Disabled_Expect_Success) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3", "0");
        auto [errorcode, value] =
            getMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("0", value);

        const auto userSid = getCurrentUserSidString();
        setMsiProperty("UserSID", userSid);
        std::tie(errorcode, value) =
            getMsiProperty("UserSID");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(userSid, value);

        setMsiProperty("ENABLEUSEBYALLUSERS", "0");
        std::tie(errorcode, value) =
            getMsiProperty("ENABLEUSEBYALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("0", value);

        testDir = std::filesystem::current_path() /= "test_data";
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

        setMsiProperty("DATADIR", testDir.string());
        std::tie(errorcode, value) =
            getMsiProperty("DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testDir.string(), value);

        EXPECT_EQ(0u, executeAction());

        AccountsInfo accountsInfo;
        accountsInfo.system = true;
        accountsInfo.admins = true;
        accountsInfo.currentUser = true;

        EXPECT_EQ(0, checkPermissions(testDir, accountsInfo, false));
        for (const auto& folder : folders) {
            EXPECT_EQ(0, checkPermissions(folder, accountsInfo, false));
        }
        for (const auto& file : files) {
            EXPECT_EQ(0, checkPermissions(file, accountsInfo, false));
        }
    }

    TEST_F(test_boinccas_CASetPermissionBOINCData,
        NormalMode_Protected_Not_Set_AllUsers_Not_Set_Expect_Success) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        const auto userSid = getCurrentUserSidString();
        setMsiProperty("UserSID", userSid);
        auto [errorcode, value] =
            getMsiProperty("UserSID");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(userSid, value);

        testDir = std::filesystem::current_path() /= "test_data";
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

        setMsiProperty("DATADIR", testDir.string());
        std::tie(errorcode, value) =
            getMsiProperty("DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testDir.string(), value);

        EXPECT_EQ(0u, executeAction());

        AccountsInfo accountsInfo;
        accountsInfo.system = true;
        accountsInfo.admins = true;
        accountsInfo.currentUser = true;

        EXPECT_EQ(0, checkPermissions(testDir, accountsInfo, false));
        for (const auto& folder : folders) {
            EXPECT_EQ(0, checkPermissions(folder, accountsInfo, false));
        }
        for (const auto& file : files) {
            EXPECT_EQ(0, checkPermissions(file, accountsInfo, false));
        }
    }

    TEST_F(test_boinccas_CASetPermissionBOINCData,
        NormalMode_Protected_Not_Set_AllUsers_Set_Expect_Success) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        const auto userSid = getCurrentUserSidString();
        setMsiProperty("UserSID", userSid);
        auto [errorcode, value] =
            getMsiProperty("UserSID");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(userSid, value);

        setMsiProperty("ENABLEUSEBYALLUSERS", "1");
        std::tie(errorcode, value) =
            getMsiProperty("ENABLEUSEBYALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        testDir = std::filesystem::current_path() /= "test_data";
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

        setMsiProperty("DATADIR", testDir.string());
        std::tie(errorcode, value) =
            getMsiProperty("DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testDir.string(), value);

        EXPECT_EQ(0u, executeAction());

        AccountsInfo accountsInfo;
        accountsInfo.system = true;
        accountsInfo.admins = true;
        accountsInfo.users = true;
        accountsInfo.currentUser = true;

        EXPECT_EQ(0, checkPermissions(testDir, accountsInfo, false));
        for (const auto& folder : folders) {
            EXPECT_EQ(0, checkPermissions(folder, accountsInfo, false));
        }
        for (const auto& file : files) {
            EXPECT_EQ(0, checkPermissions(file, accountsInfo, false));
        }
    }

    TEST_F(test_boinccas_CASetPermissionBOINCData,
        NormalMode_Protected_Not_Set_AllUsers_Disabled_Expect_Success) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        const auto userSid = getCurrentUserSidString();
        setMsiProperty("UserSID", userSid);
        auto [errorcode, value] =
            getMsiProperty("UserSID");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(userSid, value);

        setMsiProperty("ENABLEUSEBYALLUSERS", "0");
        std::tie(errorcode, value) =
            getMsiProperty("ENABLEUSEBYALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("0", value);

        testDir = std::filesystem::current_path() /= "test_data";
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

        setMsiProperty("DATADIR", testDir.string());
        std::tie(errorcode, value) =
            getMsiProperty("DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testDir.string(), value);

        EXPECT_EQ(0u, executeAction());

        AccountsInfo accountsInfo;
        accountsInfo.system = true;
        accountsInfo.admins = true;
        accountsInfo.currentUser = true;

        EXPECT_EQ(0, checkPermissions(testDir, accountsInfo, false));
        for (const auto& folder : folders) {
            EXPECT_EQ(0, checkPermissions(folder, accountsInfo, false));
        }
        for (const auto& file : files) {
            EXPECT_EQ(0, checkPermissions(file, accountsInfo, false));
        }
    }

    TEST_F(test_boinccas_CASetPermissionBOINCData,
        ProtectedMode_Eveything_Set_Expect_Success) {
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

        setMsiProperty("BOINC_PROJECTS_GROUPNAME", projectsGroupName);
        std::tie(errorcode, value) =
            getMsiProperty("BOINC_PROJECTS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(projectsGroupName, value);

        ASSERT_TRUE(createLocalGroup(projectsGroupName));

        const auto userSid = getCurrentUserSidString();
        setMsiProperty("UserSID", userSid);
        std::tie(errorcode, value) =
            getMsiProperty("UserSID");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(userSid, value);

        setMsiProperty("ENABLEUSEBYALLUSERS", "1");
        std::tie(errorcode, value) =
            getMsiProperty("ENABLEUSEBYALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        testDir = std::filesystem::current_path() /= "test_data";
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

        setMsiProperty("DATADIR", testDir.string());
        std::tie(errorcode, value) =
            getMsiProperty("DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testDir.string(), value);

        EXPECT_EQ(0u, executeAction());

        AccountsInfo accountsInfo;
        accountsInfo.system = true;
        accountsInfo.admins = true;
        accountsInfo.users = true;
        accountsInfo.boinc_admins = true;
        accountsInfo.boinc_users = true;
        accountsInfo.boinc_projects = true;

        EXPECT_EQ(0, checkPermissions(testDir, accountsInfo, true));
        for (const auto& folder : folders) {
            EXPECT_EQ(0, checkPermissions(folder, accountsInfo, true));
        }
        for (const auto& file : files) {
            EXPECT_EQ(0, checkPermissions(file, accountsInfo, true));
        }
    }

    TEST_F(test_boinccas_CASetPermissionBOINCData,
        NormalMode_Eveything_Set_Expect_Success) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3", "0");
        auto [errorcode, value] =
            getMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("0", value);

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

        setMsiProperty("BOINC_PROJECTS_GROUPNAME", projectsGroupName);
        std::tie(errorcode, value) =
            getMsiProperty("BOINC_PROJECTS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(projectsGroupName, value);

        ASSERT_TRUE(createLocalGroup(projectsGroupName));

        const auto userSid = getCurrentUserSidString();
        setMsiProperty("UserSID", userSid);
        std::tie(errorcode, value) =
            getMsiProperty("UserSID");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(userSid, value);

        setMsiProperty("ENABLEUSEBYALLUSERS", "1");
        std::tie(errorcode, value) =
            getMsiProperty("ENABLEUSEBYALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        testDir = std::filesystem::current_path() /= "test_data";
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

        setMsiProperty("DATADIR", testDir.string());
        std::tie(errorcode, value) =
            getMsiProperty("DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(testDir.string(), value);

        EXPECT_EQ(0u, executeAction());

        AccountsInfo accountsInfo;
        accountsInfo.system = true;
        accountsInfo.admins = true;
        accountsInfo.users = true;
        accountsInfo.currentUser = true;

        EXPECT_EQ(0, checkPermissions(testDir, accountsInfo, false));
        for (const auto& folder : folders) {
            EXPECT_EQ(0, checkPermissions(folder, accountsInfo, false));
        }
        for (const auto& file : files) {
            EXPECT_EQ(0, checkPermissions(file, accountsInfo, false));
        }
    }
#endif
}

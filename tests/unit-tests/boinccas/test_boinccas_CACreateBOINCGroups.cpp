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

#include <Lm.h>

namespace test_boinccas_CACreateBOINCGroups {
    constexpr auto masterAccountName = "boinc_master";
    constexpr auto projectAccountName = "boinc_project";
    constexpr auto masterAccountPassword = "qwerty123456!@#$%^";
    constexpr auto projectAccountPassword = "ytrewq654321^%$#@!";
    constexpr auto testUserAccountName = "boinc_test_user";
    constexpr auto testUserAccountPassword = "ewqytr321654#@!^%$";
    constexpr auto testPCName = "testpc";
    constexpr auto adminsGroupName = "boinc_admins";
    constexpr auto usersGroupName = "boinc_users";
    constexpr auto projectsGroupName = "boinc_projects";

    class test_boinccas_CACreateBOINCGroups : public test_boinccas_TestBase {
    protected:
        test_boinccas_CACreateBOINCGroups() :
            test_boinccas_TestBase("CreateBOINCGroups") {
        }

        void SetUp() override {
            if (userExists(masterAccountName)) {
                userDelete(masterAccountName);
            }
            if (userExists(projectAccountName)) {
                userDelete(projectAccountName);
            }
            if (userExists(testUserAccountName)) {
                userDelete(testUserAccountName);
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

            ASSERT_FALSE(userExists(masterAccountName));
            ASSERT_FALSE(userExists(projectAccountName));
            ASSERT_TRUE(userCreate(masterAccountName,
                masterAccountPassword));
            ASSERT_TRUE(userCreate(projectAccountName,
                projectAccountPassword));
            ASSERT_TRUE(userCreate(testUserAccountName,
                testUserAccountPassword));
            ASSERT_TRUE(userExists(masterAccountName));
            ASSERT_TRUE(userExists(projectAccountName));
            ASSERT_TRUE(userExists(testUserAccountName));

            ASSERT_FALSE(localGroupExists(adminsGroupName));
            ASSERT_FALSE(localGroupExists(usersGroupName));
            ASSERT_FALSE(localGroupExists(projectsGroupName));
        }

        void TearDown() override {
            if (userExists(masterAccountName)) {
                userDelete(masterAccountName);
            }
            if (userExists(projectAccountName)) {
                userDelete(projectAccountName);
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
    };

#ifdef BOINCCAS_TEST
    TEST_F(test_boinccas_CACreateBOINCGroups,
        CreateGroups_ProtectionIsNotSet) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        const auto usersGroup = getLocalizedUsersGroupName();
        ASSERT_FALSE(usersGroup.empty());
        const auto currentSid = getCurrentUserSidString();
        ASSERT_FALSE(currentSid.empty());

        setMsiProperty("UserSID", currentSid);
        auto [errorcode, value] = getMsiProperty("UserSID");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(currentSid, value);

        EXPECT_EQ(0u, executeAction());

        std::tie(errorcode, value) = getMsiProperty("BOINC_ADMINS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(adminsGroupName, value);

        std::tie(errorcode, value) = getMsiProperty("BOINC_USERS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(usersGroupName, value);

        std::tie(errorcode, value) =
            getMsiProperty("BOINC_PROJECTS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(projectsGroupName, value);

        EXPECT_TRUE(localGroupExists(adminsGroupName));
        EXPECT_TRUE(localGroupExists(usersGroupName));
        EXPECT_TRUE(localGroupExists(projectsGroupName));

        EXPECT_TRUE(isAccountMemberOfLocalGroup({}, adminsGroupName));
        EXPECT_FALSE(isAccountMemberOfLocalGroup({}, projectsGroupName));
        EXPECT_FALSE(isAccountMemberOfLocalGroup({}, usersGroupName));

        EXPECT_FALSE(isAccountMemberOfLocalGroup(masterAccountName,
            adminsGroupName));
        EXPECT_FALSE(isAccountMemberOfLocalGroup(masterAccountName,
            projectsGroupName));
        EXPECT_FALSE(isAccountMemberOfLocalGroup(masterAccountName,
            usersGroupName));
        EXPECT_FALSE(isAccountMemberOfLocalGroup(projectAccountName,
            projectsGroupName));
        EXPECT_FALSE(isAccountMemberOfLocalGroup(projectAccountName,
            usersGroupName));
        EXPECT_FALSE(isAccountMemberOfLocalGroup(projectAccountName,
            usersGroup));
        EXPECT_FALSE(isAccountMemberOfLocalGroup(masterAccountName,
            usersGroup));

        std::tie(errorcode, value) = getMsiProperty("RETURN_REBOOTREQUESTED");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("1", value);

        EXPECT_TRUE(MsiGetMode(getMsiHandle(), MSIRUNMODE_REBOOTATEND));
        // cancel reboot
        MsiSetMode(getMsiHandle(), MSIRUNMODE_REBOOTATEND, FALSE);
    }

    TEST_F(test_boinccas_CACreateBOINCGroups, CreateGroups_ProtectedDisabled) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        const auto usersGroup = getLocalizedUsersGroupName();
        ASSERT_FALSE(usersGroup.empty());
        const auto currentSid = getCurrentUserSidString();
        ASSERT_FALSE(currentSid.empty());

        setMsiProperty("UserSID", currentSid);
        auto [errorcode, value] = getMsiProperty("UserSID");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(currentSid, value);

        setMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3", "0");
        std::tie(errorcode, value) =
            getMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("0", value);

        EXPECT_EQ(0u, executeAction());

        std::tie(errorcode, value) = getMsiProperty("BOINC_ADMINS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(adminsGroupName, value);

        std::tie(errorcode, value) = getMsiProperty("BOINC_USERS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(usersGroupName, value);

        std::tie(errorcode, value) =
            getMsiProperty("BOINC_PROJECTS_GROUPNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(projectsGroupName, value);

        EXPECT_TRUE(localGroupExists(adminsGroupName));
        EXPECT_TRUE(localGroupExists(usersGroupName));
        EXPECT_TRUE(localGroupExists(projectsGroupName));

        EXPECT_TRUE(isAccountMemberOfLocalGroup({}, adminsGroupName));
        EXPECT_FALSE(isAccountMemberOfLocalGroup({}, projectsGroupName));
        EXPECT_FALSE(isAccountMemberOfLocalGroup({}, usersGroupName));

        EXPECT_FALSE(isAccountMemberOfLocalGroup(masterAccountName,
            adminsGroupName));
        EXPECT_FALSE(isAccountMemberOfLocalGroup(masterAccountName,
            projectsGroupName));
        EXPECT_FALSE(isAccountMemberOfLocalGroup(masterAccountName,
            usersGroupName));
        EXPECT_FALSE(isAccountMemberOfLocalGroup(projectAccountName,
            projectsGroupName));
        EXPECT_FALSE(isAccountMemberOfLocalGroup(projectAccountName,
            usersGroupName));
        EXPECT_FALSE(isAccountMemberOfLocalGroup(projectAccountName,
            usersGroup));
        EXPECT_FALSE(isAccountMemberOfLocalGroup(masterAccountName,
            usersGroup));

        std::tie(errorcode, value) = getMsiProperty("RETURN_REBOOTREQUESTED");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("1", value);

        EXPECT_TRUE(MsiGetMode(getMsiHandle(), MSIRUNMODE_REBOOTATEND));
        // cancel reboot
        MsiSetMode(getMsiHandle(), MSIRUNMODE_REBOOTATEND, FALSE);
    }

    TEST_F(test_boinccas_CACreateBOINCGroups,
        CreateGroups_ProtectedEnabled_NoMasterAccount) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        const auto usersGroup = getLocalizedUsersGroupName();
        ASSERT_FALSE(usersGroup.empty());
        const auto currentSid = getCurrentUserSidString();
        ASSERT_FALSE(currentSid.empty());

        setMsiProperty("UserSID", currentSid);
        auto [errorcode, value] = getMsiProperty("UserSID");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(currentSid, value);

        setMsiProperty("BOINC_PROJECT_USERNAME", projectAccountName);
        std::tie(errorcode, value) = getMsiProperty("BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(projectAccountName, value);

        setMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3", "1");
        std::tie(errorcode, value) =
            getMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        EXPECT_NE(0u, executeAction());

        // cancel reboot
        MsiSetMode(getMsiHandle(), MSIRUNMODE_REBOOTATEND, FALSE);
    }

    TEST_F(test_boinccas_CACreateBOINCGroups,
        CreateGroups_ProtectedEnabled_NoProjectAccount) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        const auto usersGroup = getLocalizedUsersGroupName();
        ASSERT_FALSE(usersGroup.empty());
        const auto currentSid = getCurrentUserSidString();
        ASSERT_FALSE(currentSid.empty());

        setMsiProperty("UserSID", currentSid);
        auto [errorcode, value] = getMsiProperty("UserSID");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(currentSid, value);

        setMsiProperty("BOINC_MASTER_USERNAME", masterAccountName);
        std::tie(errorcode, value) = getMsiProperty("BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(masterAccountName, value);

        setMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3", "1");
        std::tie(errorcode, value) =
            getMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        EXPECT_NE(0u, executeAction());

        // cancel reboot
        MsiSetMode(getMsiHandle(), MSIRUNMODE_REBOOTATEND, FALSE);
    }

    TEST_F(test_boinccas_CACreateBOINCGroups,
        CreateGroups_ProtectedEnabled_NoAccounts) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        const auto usersGroup = getLocalizedUsersGroupName();
        ASSERT_FALSE(usersGroup.empty());
        const auto currentSid = getCurrentUserSidString();
        ASSERT_FALSE(currentSid.empty());

        setMsiProperty("UserSID", currentSid);
        auto [errorcode, value] =
            getMsiProperty("UserSID");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(currentSid, value);

        setMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3", "1");
        std::tie(errorcode, value) =
            getMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        EXPECT_NE(0u, executeAction());

        // cancel reboot
        MsiSetMode(getMsiHandle(), MSIRUNMODE_REBOOTATEND, FALSE);
    }

    TEST_F(test_boinccas_CACreateBOINCGroups,
        CreateGroups_ProtectedEnabled_AddMembers) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        const auto usersGroup = getLocalizedUsersGroupName();
        ASSERT_FALSE(usersGroup.empty());
        const auto currentSid = getCurrentUserSidString();
        ASSERT_FALSE(currentSid.empty());

        setMsiProperty("UserSID", currentSid);
        auto [errorcode, value] = getMsiProperty("UserSID");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(currentSid, value);

        setMsiProperty("BOINC_MASTER_USERNAME", masterAccountName);
        std::tie(errorcode, value) = getMsiProperty("BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(masterAccountName, value);

        setMsiProperty("BOINC_PROJECT_USERNAME", projectAccountName);
        std::tie(errorcode, value) = getMsiProperty("BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ(projectAccountName, value);

        setMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3", "1");
        std::tie(errorcode, value) =
            getMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        ASSERT_TRUE(addUserToTheBuiltinAdministratorsGroup(
            getUserSid(testUserAccountName)));

        EXPECT_EQ(0u, executeAction());

        EXPECT_TRUE(localGroupExists(adminsGroupName));
        EXPECT_TRUE(localGroupExists(usersGroupName));
        EXPECT_TRUE(localGroupExists(projectsGroupName));

        // On the CI runner current user is always an administrator
        EXPECT_TRUE(isAccountMemberOfLocalGroup({}, adminsGroupName));
        EXPECT_FALSE(isAccountMemberOfLocalGroup({}, projectsGroupName));
        EXPECT_FALSE(isAccountMemberOfLocalGroup({}, usersGroupName));

        EXPECT_TRUE(isAccountMemberOfLocalGroup(masterAccountName,
            adminsGroupName));
        EXPECT_FALSE(isAccountMemberOfLocalGroup(masterAccountName,
            projectsGroupName));
        EXPECT_FALSE(isAccountMemberOfLocalGroup(masterAccountName,
            usersGroupName));
        EXPECT_TRUE(isAccountMemberOfLocalGroup(projectAccountName,
            projectsGroupName));
        EXPECT_FALSE(isAccountMemberOfLocalGroup(projectAccountName,
            usersGroupName));
        EXPECT_TRUE(isAccountMemberOfLocalGroup(projectAccountName,
            usersGroup));
        EXPECT_TRUE(isAccountMemberOfLocalGroup(masterAccountName,
            usersGroup));
        EXPECT_TRUE(isAccountMemberOfLocalGroup(testUserAccountName,
            getLocalizedAdministratorsGroupName()));

        std::tie(errorcode, value) = getMsiProperty("RETURN_REBOOTREQUESTED");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("1", value);

        EXPECT_TRUE(MsiGetMode(getMsiHandle(), MSIRUNMODE_REBOOTATEND));
        // cancel reboot
        MsiSetMode(getMsiHandle(), MSIRUNMODE_REBOOTATEND, FALSE);
    }
#endif
}

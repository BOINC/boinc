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

namespace test_boinccas_CAGrantBOINCProjectRights {
    constexpr auto userName = "boinc_project";
    constexpr auto userPassword = "qwerty123456!@#$%^";

    class test_boinccas_CAGrantBOINCProjectRights :
        public test_boinccas_TestBase {
    protected:
        test_boinccas_CAGrantBOINCProjectRights() :
            test_boinccas_TestBase("GrantBOINCProjectRights") {
        }

        void TearDown() override {
            if (userExists(userName)) {
                userDelete(userName);
            }
        }
    };

    constexpr std::array expectedSetRights = {
        "SeServiceLogonRight",
        "SeDenyNetworkLogonRight",
        "SeDenyInteractiveLogonRight",
        "SeDenyRemoteInteractiveLogonRight"
    };

    constexpr std::array expectedRemovedRights = {
        "SeDenyBatchLogonRight",
        "SeDenyServiceLogonRight",
        "SeNetworkLogonRight",
        "SeRemoteInteractiveLogonRight",
        "SeBatchLogonRight",
        "SeInteractiveLogonRight",
        "SeTcbPrivilege",
        "SeMachineAccountPrivilege",
        "SeIncreaseQuotaPrivilege",
        "SeBackupPrivilege",
        "SeChangeNotifyPrivilege",
        "SeSystemTimePrivilege",
        "SeCreateTokenPrivilege",
        "SeCreatePagefilePrivilege",
        "SeCreateGlobalPrivilege",
        "SeDebugPrivilege",
        "SeEnableDelegationPrivilege",
        "SeRemoteShutdownPrivilege",
        "SeAuditPrivilege",
        "SeImpersonatePrivilege",
        "SeIncreaseBasePriorityPrivilege",
        "SeLoadDriverPrivilege",
        "SeLockMemoryPrivilege",
        "SeSecurityPrivilege",
        "SeSystemEnvironmentPrivilege",
        "SeManageVolumePrivilege",
        "SeProfileSingleProcessPrivilege",
        "SeSystemProfilePrivilege",
        "SeUndockPrivilege",
        "SeAssignPrimaryTokenPrivilege",
        "SeRestorePrivilege",
        "SeShutdownPrivilege",
        "SeSynchAgentPrivilege",
        "SeTakeOwnershipPrivilege"
    };

#ifdef BOINCCAS_TEST
    TEST_F(test_boinccas_CAGrantBOINCProjectRights, NoUser_Expect_Fail) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_FALSE(userExists(userName));
        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CAGrantBOINCProjectRights,
        UserExists_No_Property_Set_Expect_Fail) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_TRUE(userCreate(userName, userPassword));
        ASSERT_TRUE(userExists(userName));
        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CAGrantBOINCProjectRights,
        UserExists_Expect_Correct_Rights_Set) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_TRUE(userCreate(userName, userPassword));
        ASSERT_TRUE(userExists(userName));
        setMsiProperty("BOINC_PROJECT_USERNAME", userName);
        EXPECT_EQ(0u, executeAction());

        const auto rights = getAccountRights(userName);
        for (const auto& expectedRight : expectedSetRights) {
            EXPECT_NE(std::find(rights.cbegin(), rights.cend(),
                expectedRight), rights.cend())
                << "Expected right not found: " << expectedRight;
        }
        for (const auto& expectedRemovedRight : expectedRemovedRights) {
            EXPECT_EQ(std::find(rights.cbegin(), rights.cend(),
                expectedRemovedRight), rights.cend())
                << "Unexpected right found: " << expectedRemovedRight;
        }
    }

    TEST_F(test_boinccas_CAGrantBOINCProjectRights,
        UserExistsWithOtherRights_Expect_CorrectRightsSet) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_TRUE(userCreate(userName, userPassword));
        ASSERT_TRUE(userExists(userName));
        const auto opResult = setAccountRights(userName, expectedRemovedRights);
        ASSERT_TRUE(opResult);
        setMsiProperty("BOINC_PROJECT_USERNAME", userName);
        EXPECT_EQ(0u, executeAction());

        const auto rights = getAccountRights(userName);
        for (const auto& expectedRight : expectedSetRights) {
            EXPECT_NE(std::find(rights.cbegin(), rights.cend(),
                expectedRight), rights.cend())
                << "Expected right not found: " << expectedRight;
        }
        for (const auto& expectedRemovedRight : expectedRemovedRights) {
            EXPECT_EQ(std::find(rights.cbegin(), rights.cend(),
                expectedRemovedRight), rights.cend())
                << "Unexpected right found: " << expectedRemovedRight;
        }
    }
#endif
}

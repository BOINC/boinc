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

namespace test_boinccas_CAGrantBOINCAdminsRights {
    class test_boinccas_CAGrantBOINCAdminsRights :
        public test_boinccas_TestBase {
    protected:
        test_boinccas_CAGrantBOINCAdminsRights() :
            test_boinccas_TestBase("GrantBOINCAdminsRights") {
        }

        void TearDown() override {
            if (localGroupExists("boinc_admins")) {
                deleteLocalGroup("boinc_admins");
            }
        }
    };

    constexpr std::array expectedSetRights = {
        "SeIncreaseQuotaPrivilege",
        "SeChangeNotifyPrivilege",
        "SeCreateGlobalPrivilege",
        "SeAssignPrimaryTokenPrivilege"
    };

    constexpr std::array expectedRemovedRights = {
        "SeNetworkLogonRight",
        "SeRemoteInteractiveLogonRight",
        "SeBatchLogonRight",
        "SeInteractiveLogonRight",
        "SeServiceLogonRight",
        "SeDenyNetworkLogonRight",
        "SeDenyInteractiveLogonRight",
        "SeDenyBatchLogonRight",
        "SeDenyServiceLogonRight",
        "SeDenyRemoteInteractiveLogonRight",
        "SeTcbPrivilege",
        "SeMachineAccountPrivilege",
        "SeBackupPrivilege",
        "SeSystemTimePrivilege",
        "SeCreateTokenPrivilege",
        "SeCreatePagefilePrivilege",
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
        "SeRestorePrivilege",
        "SeShutdownPrivilege",
        "SeSynchAgentPrivilege",
        "SeTakeOwnershipPrivilege"
    };

#ifdef BOINCCAS_TEST
    TEST_F(test_boinccas_CAGrantBOINCAdminsRights, NoGroup_Expect_Fail) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_FALSE(localGroupExists("boinc_admins"));
        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CAGrantBOINCAdminsRights,
        GroupExists_Expect_Correct_Rights_Set) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_TRUE(createLocalGroup("boinc_admins"));
        ASSERT_TRUE(localGroupExists("boinc_admins"));
        EXPECT_EQ(0u, executeAction());

        const auto rights = getAccountRights("boinc_admins");
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

    TEST_F(test_boinccas_CAGrantBOINCAdminsRights,
        GroupExistsWithOtherRights_Expect_CorrectRightsSet) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_TRUE(createLocalGroup("boinc_admins"));
        ASSERT_TRUE(localGroupExists("boinc_admins"));
        auto rightSet = false;
        for (auto i = 0; i < 10; ++i) {
            const auto [opResult, failed] = setAccountRights("boinc_admins", expectedRemovedRights);
            if (opResult) {
                rightSet = true;
                break;
            }
            if (!failed.empty()) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        ASSERT_TRUE(rightSet);
        EXPECT_EQ(0u, executeAction());
        const auto rights = getAccountRights("boinc_admins");
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

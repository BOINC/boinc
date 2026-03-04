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
#include "user_group_helper.h"

namespace test_boinccas_CAGrantBOINCAdminsVirtualBoxRights {
    constexpr auto vboxKey = "APPID\\{819B4D85-9CEE-493C-B6FC-64FFE759B3C9}";
    constexpr std::array permissions =
    { "AccessPermission", "LaunchPermission" };
    constexpr auto groupName = "boinc_admins";

    class test_boinccas_CAGrantBOINCAdminsVirtualBoxRights :
        public test_boinccas_TestBase {
    protected:
        test_boinccas_CAGrantBOINCAdminsVirtualBoxRights() :
            test_boinccas_TestBase("GrantBOINCAdminsVirtualBoxRights") {
        }

        void TearDown() override {
            const auto boincAdminsSidString =
                getSidStringFromSid(getUserSid(groupName));
            const auto systemSidString =
                getSidStringFromSid(getUserSid("SYSTEM"));
            const auto interactiveSidString =
                getSidStringFromSid(getUserSid("INTERACTIVE"));

            for (const auto permission : permissions) {
                auto accessSD = getRegistryValue<std::vector<BYTE>>(
                    HKEY_CLASSES_ROOT, vboxKey, permission);
                if (accessSD.empty()) {
                    continue;
                }
                auto present = FALSE;
                auto defaultDACL = FALSE;
                PACL dacl = nullptr;
                if (!GetSecurityDescriptorDacl(
                    reinterpret_cast<PSECURITY_DESCRIPTOR>(accessSD.data()),
                    &present, &dacl, &defaultDACL)) {
                    continue;
                }
                if (!present || dacl == nullptr) {
                    continue;
                }
                ACL_SIZE_INFORMATION aclSizeInfo;
                if (!GetAclInformation(dacl, &aclSizeInfo,
                    sizeof(aclSizeInfo), AclSizeInformation)) {
                    continue;
                }
                if (aclSizeInfo.AceCount == 0u) {
                    continue;
                }
                for (decltype(aclSizeInfo.AceCount) i = 0;
                    i < aclSizeInfo.AceCount; ++i) {
                    LPVOID ace = nullptr;
                    if (!GetAce(dacl, i, &ace)) {
                        continue;
                    }
                    auto aceHeader = reinterpret_cast<ACE_HEADER*>(ace);
                    if (aceHeader->AceType == ACCESS_ALLOWED_ACE_TYPE) {
                        auto allowedAce =
                            reinterpret_cast<ACCESS_ALLOWED_ACE*>(ace);
                        wil::unique_sid sid(static_cast<PSID>(
                            LocalAlloc(LMEM_FIXED, SECURITY_MAX_SID_SIZE)));
                        CopySid(SECURITY_MAX_SID_SIZE, sid.get(),
                            reinterpret_cast<PSID>(&allowedAce->SidStart));
                        auto sidString = getSidStringFromSid(std::move(sid));
                        if (!boincAdminsSidString.empty() &&
                            sidString == boincAdminsSidString) {
                            DeleteAce(dacl, i);
                            continue;
                        }
                        if (!systemSidString.empty() &&
                            sidString == systemSidString) {
                            DeleteAce(dacl, i);
                            continue;
                        }
                        if (!interactiveSidString.empty() &&
                            sidString == interactiveSidString) {
                            DeleteAce(dacl, i);
                            continue;
                        }
                    }
                }
            }

            if (localGroupExists(groupName)) {
                deleteLocalGroup(groupName);
            }
            cleanRegistryKey(HKEY_CLASSES_ROOT, vboxKey);
        }

        void validatePermissions() {
            const auto boincAdminsSidString =
                getSidStringFromSid(getUserSid(groupName));
            ASSERT_FALSE(boincAdminsSidString.empty());
            const auto systemSidString =
                getSidStringFromSid(getUserSid("SYSTEM"));
            ASSERT_FALSE(systemSidString.empty());
            const auto interactiveSidString =
                getSidStringFromSid(getUserSid("INTERACTIVE"));
            ASSERT_FALSE(interactiveSidString.empty());

            for (const auto permission : permissions) {
                auto accessSD = getRegistryValue<std::vector<BYTE>>(
                    HKEY_CLASSES_ROOT, vboxKey, permission);
                ASSERT_FALSE(accessSD.empty());

                auto present = FALSE;
                auto defaultDACL = FALSE;
                PACL dacl = nullptr;
                ASSERT_TRUE(GetSecurityDescriptorDacl(
                    reinterpret_cast<PSECURITY_DESCRIPTOR>(accessSD.data()),
                    &present, &dacl, &defaultDACL));
                ASSERT_TRUE(present);
                ASSERT_NE(dacl, nullptr);

                ACL_SIZE_INFORMATION aclSizeInfo;
                ASSERT_TRUE(GetAclInformation(dacl, &aclSizeInfo,
                    sizeof(aclSizeInfo), AclSizeInformation));
                ASSERT_GT(aclSizeInfo.AceCount, 0u);

                auto foundSystem = false;
                auto foundInteractive = false;
                auto foundBOINCAdmins = false;
                for (decltype(aclSizeInfo.AceCount) i = 0;
                    i < aclSizeInfo.AceCount; ++i) {
                    LPVOID ace = nullptr;
                    ASSERT_TRUE(GetAce(dacl, i, &ace));
                    auto aceHeader = reinterpret_cast<ACE_HEADER*>(ace);
                    if (aceHeader->AceType == ACCESS_ALLOWED_ACE_TYPE) {
                        auto allowedAce =
                            reinterpret_cast<ACCESS_ALLOWED_ACE*>(ace);
                        wil::unique_sid sid(static_cast<PSID>(
                            LocalAlloc(LMEM_FIXED, SECURITY_MAX_SID_SIZE)));
                        CopySid(SECURITY_MAX_SID_SIZE, sid.get(),
                            reinterpret_cast<PSID>(&allowedAce->SidStart));
                        auto sidString = getSidStringFromSid(std::move(sid));
                        if (!foundBOINCAdmins &&
                            sidString == boincAdminsSidString) {
                            foundBOINCAdmins = true;
                            ASSERT_TRUE(
                                (allowedAce->Mask & COM_RIGHTS_EXECUTE) != 0);
                            continue;
                        }
                        if (!foundSystem && sidString == systemSidString) {
                            foundSystem = true;
                            ASSERT_TRUE(
                                (allowedAce->Mask & COM_RIGHTS_EXECUTE) != 0);
                            continue;
                        }
                        if (!foundInteractive &&
                            sidString == interactiveSidString) {
                            foundInteractive = true;
                            ASSERT_TRUE(
                                (allowedAce->Mask & COM_RIGHTS_EXECUTE) != 0);
                            continue;
                        }
                    }
                }
                ASSERT_TRUE(foundSystem);
                ASSERT_TRUE(foundInteractive);
                ASSERT_TRUE(foundBOINCAdmins);
            }
        }
    };

#ifdef BOINCCAS_TEST
    TEST_F(test_boinccas_CAGrantBOINCAdminsVirtualBoxRights,
        NoBoincAdminsGroup_ExpectFail) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_FALSE(localGroupExists(groupName));
        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CAGrantBOINCAdminsVirtualBoxRights,
        BoincAdminsGroupExists_SingleRunSuccess) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_TRUE(createLocalGroup(groupName));
        ASSERT_TRUE(localGroupExists(groupName));

        ASSERT_EQ(0u, executeAction());
        validatePermissions();
    }

    TEST_F(test_boinccas_CAGrantBOINCAdminsVirtualBoxRights,
        BoincAdminsGroupExists_PermissionsOwerwrittenSuccess) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_TRUE(createLocalGroup(groupName));
        ASSERT_TRUE(localGroupExists(groupName));

        ASSERT_EQ(0u, executeAction());
        validatePermissions();

        ASSERT_EQ(0u, executeAction());
        validatePermissions();
    }
#endif
}

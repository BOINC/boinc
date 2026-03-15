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

#include <shlobj.h>

namespace test_boinccas_CARestoreSetupState {
    class test_boinccas_CARestoreSetupState : public test_boinccas_TestBase {
    protected:
        test_boinccas_CARestoreSetupState() :
            test_boinccas_TestBase("RestoreSetupState") {
        }

        void TearDown() override {
            cleanRegistryKey();
        }

        std::string getDefaultDataPath() {
            PWSTR path = nullptr;
            const auto hr = SHGetKnownFolderPath(FOLDERID_ProgramData, 0,
                nullptr, &path);
            if (path == nullptr) {
                return {};
            }
            wil::unique_cotaskmem_string pathDeleter(path);
            if (FAILED(hr)) {
                return {};
            }
            std::filesystem::path dataPath(path);
            dataPath /= "BOINC";
            return dataPath.string() + "\\";
        }
    };

#ifdef BOINCCAS_TEST
    TEST_F(test_boinccas_CARestoreSetupState,
        NoPreviousData_Expect_DATADIR_Set) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_TRUE(getRegistryValue("SETUPSTATESTORED").empty());
        EXPECT_TRUE(getRegistryValue("INSTALLDIR").empty());
        EXPECT_TRUE(getRegistryValue("DATADIR").empty());
        EXPECT_TRUE(getRegistryValue("LAUNCHPROGRAM").empty());
        EXPECT_TRUE(getRegistryValue("BOINC_MASTER_USERNAME").empty());
        EXPECT_TRUE(getRegistryValue("BOINC_PROJECT_USERNAME").empty());
        EXPECT_TRUE(getRegistryValue("ENABLELAUNCHATLOGON").empty());
        EXPECT_TRUE(getRegistryValue("ENABLESCREENSAVER").empty());
        EXPECT_TRUE(
            getRegistryValue("ENABLEPROTECTEDAPPLICATIONEXECUTION3").empty());
        EXPECT_TRUE(getRegistryValue("ENABLEUSEBYALLUSERS").empty());
        EXPECT_TRUE(getRegistryValue("UpgradingTo").empty());

        auto [errorcode, value] = getMsiProperty("OVERRIDE_INSTALLDIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("OVERRIDE_DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("OVERRIDE_LAUNCHPROGRAM");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_ENABLELAUNCHATLOGON");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_ENABLESCREENSAVER");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_ENABLEUSEBYALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());

        ASSERT_EQ(0u, executeAction());

        std::tie(errorcode, value) = getMsiProperty("DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(getDefaultDataPath(), value);

        std::tie(errorcode, value) = getMsiProperty("INSTALLDIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("LAUNCHPROGRAM");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("ENABLELAUNCHATLOGON");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("ENABLESCREENSAVER");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("ENABLEUSEBYALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("ALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("ProductVersion");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("IS_MAJOR_UPGRADE");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
    }

    TEST_F(test_boinccas_CARestoreSetupState,
        SetupStateRestored_Set_To_Invalid_Value) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        ASSERT_TRUE(setRegistryValue("SETUPSTATESTORED", "Invalid"));
        ASSERT_EQ("Invalid", getRegistryValue("SETUPSTATESTORED"));

        EXPECT_TRUE(getRegistryValue("INSTALLDIR").empty());
        EXPECT_TRUE(getRegistryValue("DATADIR").empty());
        EXPECT_TRUE(getRegistryValue("LAUNCHPROGRAM").empty());
        EXPECT_TRUE(getRegistryValue("BOINC_MASTER_USERNAME").empty());
        EXPECT_TRUE(getRegistryValue("BOINC_PROJECT_USERNAME").empty());
        EXPECT_TRUE(getRegistryValue("ENABLELAUNCHATLOGON").empty());
        EXPECT_TRUE(getRegistryValue("ENABLESCREENSAVER").empty());
        EXPECT_TRUE(getRegistryValue(
            "ENABLEPROTECTEDAPPLICATIONEXECUTION3").empty());
        EXPECT_TRUE(getRegistryValue("ENABLEUSEBYALLUSERS").empty());
        EXPECT_TRUE(getRegistryValue("UpgradingTo").empty());

        auto [errorcode, value] = getMsiProperty("OVERRIDE_INSTALLDIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("OVERRIDE_DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("OVERRIDE_LAUNCHPROGRAM");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_ENABLELAUNCHATLOGON");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_ENABLESCREENSAVER");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_ENABLEUSEBYALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());

        ASSERT_EQ(0u, executeAction());

        std::tie(errorcode, value) = getMsiProperty("DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(getDefaultDataPath(), value);

        std::tie(errorcode, value) = getMsiProperty("INSTALLDIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("LAUNCHPROGRAM");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("ENABLELAUNCHATLOGON");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("ENABLESCREENSAVER");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("ENABLEUSEBYALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("ALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("ProductVersion");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("IS_MAJOR_UPGRADE");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
    }

    TEST_F(test_boinccas_CARestoreSetupState,
        SetupStateRestored_Set_No_Prev_Data_No_Override_Data) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        ASSERT_TRUE(setRegistryValue("SETUPSTATESTORED", "TRUE"));
        ASSERT_EQ("TRUE", getRegistryValue("SETUPSTATESTORED"));

        EXPECT_TRUE(getRegistryValue("INSTALLDIR").empty());
        EXPECT_TRUE(getRegistryValue("DATADIR").empty());
        EXPECT_TRUE(getRegistryValue("LAUNCHPROGRAM").empty());
        EXPECT_TRUE(getRegistryValue("BOINC_MASTER_USERNAME").empty());
        EXPECT_TRUE(getRegistryValue("BOINC_PROJECT_USERNAME").empty());
        EXPECT_TRUE(getRegistryValue("ENABLELAUNCHATLOGON").empty());
        EXPECT_TRUE(getRegistryValue("ENABLESCREENSAVER").empty());
        EXPECT_TRUE(getRegistryValue(
            "ENABLEPROTECTEDAPPLICATIONEXECUTION3").empty());
        EXPECT_TRUE(getRegistryValue("ENABLEUSEBYALLUSERS").empty());
        EXPECT_TRUE(getRegistryValue("UpgradingTo").empty());

        auto [errorcode, value] = getMsiProperty("OVERRIDE_INSTALLDIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("OVERRIDE_DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("OVERRIDE_LAUNCHPROGRAM");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_ENABLELAUNCHATLOGON");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_ENABLESCREENSAVER");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_ENABLEUSEBYALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());

        ASSERT_EQ(0u, executeAction());

        std::tie(errorcode, value) = getMsiProperty("DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ(getDefaultDataPath(), value);

        std::tie(errorcode, value) = getMsiProperty("INSTALLDIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("LAUNCHPROGRAM");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("1", value);
        std::tie(errorcode, value) = getMsiProperty("BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("ENABLELAUNCHATLOGON");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("ENABLESCREENSAVER");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("ENABLEUSEBYALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("ALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("ProductVersion");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("IS_MAJOR_UPGRADE");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
    }

    TEST_F(test_boinccas_CARestoreSetupState,
        SetupStateRestored_Set_Prev_Data_Set_No_Override_Data) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        ASSERT_TRUE(setRegistryValue("SETUPSTATESTORED", "TRUE"));
        ASSERT_EQ("TRUE", getRegistryValue("SETUPSTATESTORED"));
        ASSERT_TRUE(setRegistryValue("INSTALLDIR", "install_test"));
        ASSERT_EQ("install_test", getRegistryValue("INSTALLDIR"));
        ASSERT_TRUE(setRegistryValue("DATADIR", "data_test"));
        ASSERT_EQ("data_test", getRegistryValue("DATADIR"));
        ASSERT_TRUE(setRegistryValue("LAUNCHPROGRAM", "1"));
        ASSERT_EQ("1", getRegistryValue("LAUNCHPROGRAM"));
        ASSERT_TRUE(setRegistryValue("BOINC_MASTER_USERNAME", "test_master"));
        ASSERT_EQ("test_master", getRegistryValue("BOINC_MASTER_USERNAME"));
        ASSERT_TRUE(setRegistryValue("BOINC_PROJECT_USERNAME", "test_project"));
        ASSERT_EQ("test_project", getRegistryValue("BOINC_PROJECT_USERNAME"));
        ASSERT_TRUE(setRegistryValue("ENABLELAUNCHATLOGON", "1"));
        ASSERT_EQ("1", getRegistryValue("ENABLELAUNCHATLOGON"));
        ASSERT_TRUE(setRegistryValue("ENABLESCREENSAVER", "1"));
        ASSERT_EQ("1", getRegistryValue("ENABLESCREENSAVER"));
        ASSERT_TRUE(setRegistryValue("ENABLEPROTECTEDAPPLICATIONEXECUTION3", "1"));
        ASSERT_EQ("1", getRegistryValue("ENABLEPROTECTEDAPPLICATIONEXECUTION3"));
        ASSERT_TRUE(setRegistryValue("ENABLEUSEBYALLUSERS", "1"));
        ASSERT_EQ("1", getRegistryValue("ENABLEUSEBYALLUSERS"));

        EXPECT_TRUE(getRegistryValue("UpgradingTo").empty());

        auto [errorcode, value] = getMsiProperty("OVERRIDE_INSTALLDIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("OVERRIDE_DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("OVERRIDE_LAUNCHPROGRAM");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_ENABLELAUNCHATLOGON");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_ENABLESCREENSAVER");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_ENABLEUSEBYALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());

        ASSERT_EQ(0u, executeAction());

        std::tie(errorcode, value) = getMsiProperty("DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("data_test", value);

        std::tie(errorcode, value) = getMsiProperty("INSTALLDIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("install_test", value);
        std::tie(errorcode, value) = getMsiProperty("LAUNCHPROGRAM");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("1", value);
        std::tie(errorcode, value) = getMsiProperty("BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("test_master", value);
        std::tie(errorcode, value) = getMsiProperty("BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("test_project", value);
        std::tie(errorcode, value) = getMsiProperty("ENABLELAUNCHATLOGON");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("1", value);
        std::tie(errorcode, value) = getMsiProperty("ENABLESCREENSAVER");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("1", value);
        std::tie(errorcode, value) =
            getMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("1", value);
        std::tie(errorcode, value) = getMsiProperty("ENABLEUSEBYALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("1", value);
        std::tie(errorcode, value) = getMsiProperty("ALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("1", value);
        std::tie(errorcode, value) = getMsiProperty("ProductVersion");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("IS_MAJOR_UPGRADE");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
    }

    TEST_F(test_boinccas_CARestoreSetupState,
        Corrected_LAUNCHPROGRAM) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        ASSERT_TRUE(setRegistryValue("SETUPSTATESTORED", "TRUE"));
        ASSERT_EQ("TRUE", getRegistryValue("SETUPSTATESTORED"));
        ASSERT_TRUE(setRegistryValue("INSTALLDIR", "install_test"));
        ASSERT_EQ("install_test", getRegistryValue("INSTALLDIR"));
        ASSERT_TRUE(setRegistryValue("DATADIR", "data_test"));
        ASSERT_EQ("data_test", getRegistryValue("DATADIR"));
        ASSERT_TRUE(setRegistryValue("LAUNCHPROGRAM", "2"));
        ASSERT_EQ("2", getRegistryValue("LAUNCHPROGRAM"));
        ASSERT_TRUE(setRegistryValue("BOINC_MASTER_USERNAME", "test_master"));
        ASSERT_EQ("test_master", getRegistryValue("BOINC_MASTER_USERNAME"));
        ASSERT_TRUE(setRegistryValue("BOINC_PROJECT_USERNAME", "test_project"));
        ASSERT_EQ("test_project", getRegistryValue("BOINC_PROJECT_USERNAME"));
        ASSERT_TRUE(setRegistryValue("ENABLELAUNCHATLOGON", "1"));
        ASSERT_EQ("1", getRegistryValue("ENABLELAUNCHATLOGON"));
        ASSERT_TRUE(setRegistryValue("ENABLESCREENSAVER", "1"));
        ASSERT_EQ("1", getRegistryValue("ENABLESCREENSAVER"));
        ASSERT_TRUE(setRegistryValue("ENABLEPROTECTEDAPPLICATIONEXECUTION3", "1"));
        ASSERT_EQ("1", getRegistryValue("ENABLEPROTECTEDAPPLICATIONEXECUTION3"));
        ASSERT_TRUE(setRegistryValue("ENABLEUSEBYALLUSERS", "1"));
        ASSERT_EQ("1", getRegistryValue("ENABLEUSEBYALLUSERS"));

        EXPECT_TRUE(getRegistryValue("UpgradingTo").empty());

        auto [errorcode, value] = getMsiProperty("OVERRIDE_INSTALLDIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("OVERRIDE_DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("OVERRIDE_LAUNCHPROGRAM");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_ENABLELAUNCHATLOGON");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_ENABLESCREENSAVER");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_ENABLEUSEBYALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());

        ASSERT_EQ(0u, executeAction());

        std::tie(errorcode, value) = getMsiProperty("DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("data_test", value);

        std::tie(errorcode, value) = getMsiProperty("INSTALLDIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("install_test", value);
        std::tie(errorcode, value) = getMsiProperty("LAUNCHPROGRAM");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("test_master", value);
        std::tie(errorcode, value) = getMsiProperty("BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("test_project", value);
        std::tie(errorcode, value) = getMsiProperty("ENABLELAUNCHATLOGON");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("1", value);
        std::tie(errorcode, value) = getMsiProperty("ENABLESCREENSAVER");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("1", value);
        std::tie(errorcode, value) =
            getMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("1", value);
        std::tie(errorcode, value) = getMsiProperty("ENABLEUSEBYALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("1", value);
        std::tie(errorcode, value) = getMsiProperty("ALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("1", value);
        std::tie(errorcode, value) = getMsiProperty("ProductVersion");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("IS_MAJOR_UPGRADE");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
    }

    TEST_F(test_boinccas_CARestoreSetupState,
        SetupStateRestored_Set_No_Prev_Data_Override_Data_Set) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        ASSERT_TRUE(setRegistryValue("SETUPSTATESTORED", "TRUE"));
        ASSERT_EQ("TRUE", getRegistryValue("SETUPSTATESTORED"));

        EXPECT_TRUE(getRegistryValue("INSTALLDIR").empty());
        EXPECT_TRUE(getRegistryValue("DATADIR").empty());
        EXPECT_TRUE(getRegistryValue("LAUNCHPROGRAM").empty());
        EXPECT_TRUE(getRegistryValue("BOINC_MASTER_USERNAME").empty());
        EXPECT_TRUE(getRegistryValue("BOINC_PROJECT_USERNAME").empty());
        EXPECT_TRUE(getRegistryValue("ENABLELAUNCHATLOGON").empty());
        EXPECT_TRUE(getRegistryValue("ENABLESCREENSAVER").empty());
        EXPECT_TRUE(getRegistryValue(
            "ENABLEPROTECTEDAPPLICATIONEXECUTION3").empty());
        EXPECT_TRUE(getRegistryValue("ENABLEUSEBYALLUSERS").empty());
        EXPECT_TRUE(getRegistryValue("UpgradingTo").empty());

        setMsiProperty("OVERRIDE_INSTALLDIR", "install_test_override");
        auto [errorcode, value] = getMsiProperty("OVERRIDE_INSTALLDIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("install_test_override", value);

        setMsiProperty("OVERRIDE_DATADIR", "data_test_override");
        std::tie(errorcode, value) = getMsiProperty("OVERRIDE_DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("data_test_override", value);

        setMsiProperty("OVERRIDE_LAUNCHPROGRAM", "0");
        std::tie(errorcode, value) = getMsiProperty("OVERRIDE_LAUNCHPROGRAM");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("0", value);

        setMsiProperty("OVERRIDE_BOINC_MASTER_USERNAME",
            "test_master_override");
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("test_master_override", value);

        setMsiProperty("OVERRIDE_BOINC_PROJECT_USERNAME",
            "test_project_override");
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("test_project_override", value);

        setMsiProperty("OVERRIDE_ENABLELAUNCHATLOGON", "0");
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_ENABLELAUNCHATLOGON");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("0", value);

        setMsiProperty("OVERRIDE_ENABLESCREENSAVER", "0");
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_ENABLESCREENSAVER");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("0", value);

        setMsiProperty("OVERRIDE_ENABLEPROTECTEDAPPLICATIONEXECUTION3", "0");
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("0", value);

        setMsiProperty("OVERRIDE_ENABLEUSEBYALLUSERS", "0");
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_ENABLEUSEBYALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("0", value);

        ASSERT_EQ(0u, executeAction());

        std::tie(errorcode, value) = getMsiProperty("DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("data_test_override", value);

        std::tie(errorcode, value) = getMsiProperty("INSTALLDIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("install_test_override", value);
        std::tie(errorcode, value) = getMsiProperty("LAUNCHPROGRAM");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("0", value);
        std::tie(errorcode, value) = getMsiProperty("BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("test_master_override", value);
        std::tie(errorcode, value) = getMsiProperty("BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("test_project_override", value);
        std::tie(errorcode, value) = getMsiProperty("ENABLELAUNCHATLOGON");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("0", value);
        std::tie(errorcode, value) = getMsiProperty("ENABLESCREENSAVER");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("0", value);
        std::tie(errorcode, value) =
            getMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("0", value);
        std::tie(errorcode, value) = getMsiProperty("ENABLEUSEBYALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("0", value);
        std::tie(errorcode, value) = getMsiProperty("ALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("0", value);
        std::tie(errorcode, value) = getMsiProperty("ProductVersion");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("IS_MAJOR_UPGRADE");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
    }

    TEST_F(test_boinccas_CARestoreSetupState,
        SetupStateRestored_Set_Prev_Data_Set_Override_Data_Set) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        ASSERT_TRUE(setRegistryValue("SETUPSTATESTORED", "TRUE"));
        ASSERT_EQ("TRUE", getRegistryValue("SETUPSTATESTORED"));
        ASSERT_TRUE(setRegistryValue("INSTALLDIR", "install_test"));
        ASSERT_EQ("install_test", getRegistryValue("INSTALLDIR"));
        ASSERT_TRUE(setRegistryValue("DATADIR", "data_test"));
        ASSERT_EQ("data_test", getRegistryValue("DATADIR"));
        ASSERT_TRUE(setRegistryValue("LAUNCHPROGRAM", "1"));
        ASSERT_EQ("1", getRegistryValue("LAUNCHPROGRAM"));
        ASSERT_TRUE(setRegistryValue("BOINC_MASTER_USERNAME", "test_master"));
        ASSERT_EQ("test_master", getRegistryValue("BOINC_MASTER_USERNAME"));
        ASSERT_TRUE(setRegistryValue("BOINC_PROJECT_USERNAME", "test_project"));
        ASSERT_EQ("test_project", getRegistryValue("BOINC_PROJECT_USERNAME"));
        ASSERT_TRUE(setRegistryValue("ENABLELAUNCHATLOGON", "1"));
        ASSERT_EQ("1", getRegistryValue("ENABLELAUNCHATLOGON"));
        ASSERT_TRUE(setRegistryValue("ENABLESCREENSAVER", "1"));
        ASSERT_EQ("1", getRegistryValue("ENABLESCREENSAVER"));
        ASSERT_TRUE(setRegistryValue("ENABLEPROTECTEDAPPLICATIONEXECUTION3", "1"));
        ASSERT_EQ("1", getRegistryValue("ENABLEPROTECTEDAPPLICATIONEXECUTION3"));
        ASSERT_TRUE(setRegistryValue("ENABLEUSEBYALLUSERS", "1"));
        ASSERT_EQ("1", getRegistryValue("ENABLEUSEBYALLUSERS"));

        EXPECT_TRUE(getRegistryValue("UpgradingTo").empty());

        setMsiProperty("OVERRIDE_INSTALLDIR", "install_test_override");
        auto [errorcode, value] = getMsiProperty("OVERRIDE_INSTALLDIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("install_test_override", value);

        setMsiProperty("OVERRIDE_DATADIR", "data_test_override");
        std::tie(errorcode, value) = getMsiProperty("OVERRIDE_DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("data_test_override", value);

        setMsiProperty("OVERRIDE_LAUNCHPROGRAM", "0");
        std::tie(errorcode, value) = getMsiProperty("OVERRIDE_LAUNCHPROGRAM");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("0", value);

        setMsiProperty("OVERRIDE_BOINC_MASTER_USERNAME",
            "test_master_override");
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("test_master_override", value);

        setMsiProperty("OVERRIDE_BOINC_PROJECT_USERNAME",
            "test_project_override");
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("test_project_override", value);

        setMsiProperty("OVERRIDE_ENABLELAUNCHATLOGON", "0");
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_ENABLELAUNCHATLOGON");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("0", value);

        setMsiProperty("OVERRIDE_ENABLESCREENSAVER", "0");
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_ENABLESCREENSAVER");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("0", value);

        setMsiProperty("OVERRIDE_ENABLEPROTECTEDAPPLICATIONEXECUTION3", "0");
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("0", value);

        setMsiProperty("OVERRIDE_ENABLEUSEBYALLUSERS", "0");
        std::tie(errorcode, value) =
            getMsiProperty("OVERRIDE_ENABLEUSEBYALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("0", value);

        ASSERT_EQ(0u, executeAction());

        std::tie(errorcode, value) = getMsiProperty("DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("data_test_override", value);

        std::tie(errorcode, value) = getMsiProperty("INSTALLDIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("install_test_override", value);
        std::tie(errorcode, value) = getMsiProperty("LAUNCHPROGRAM");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("0", value);
        std::tie(errorcode, value) = getMsiProperty("BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("test_master_override", value);
        std::tie(errorcode, value) = getMsiProperty("BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("test_project_override", value);
        std::tie(errorcode, value) = getMsiProperty("ENABLELAUNCHATLOGON");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("0", value);
        std::tie(errorcode, value) = getMsiProperty("ENABLESCREENSAVER");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("0", value);
        std::tie(errorcode, value) =
            getMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("0", value);
        std::tie(errorcode, value) = getMsiProperty("ENABLEUSEBYALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("0", value);
        std::tie(errorcode, value) = getMsiProperty("ALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("0", value);
        std::tie(errorcode, value) = getMsiProperty("ProductVersion");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
        std::tie(errorcode, value) = getMsiProperty("IS_MAJOR_UPGRADE");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
    }

    TEST_F(test_boinccas_CARestoreSetupState,
        No_ProductVersion_No_UpgradingTo_No_IS_MAJOR_UPGRADE) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_TRUE(getRegistryValue("UpgradingTo").empty());
        auto [errorcode, value] = getMsiProperty("ProductVersion");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());

        ASSERT_EQ(0u, executeAction());

        std::tie(errorcode, value) = getMsiProperty("IS_MAJOR_UPGRADE");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
    }

    TEST_F(test_boinccas_CARestoreSetupState,
        ProductVersion_Set_No_UpgradingTo_No_IS_MAJOR_UPGRADE) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_TRUE(getRegistryValue("UpgradingTo").empty());

        setMsiProperty("ProductVersion", "1.2.3");
        auto [errorcode, value] = getMsiProperty("ProductVersion");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("1.2.3", value);

        ASSERT_EQ(0u, executeAction());

        std::tie(errorcode, value) = getMsiProperty("IS_MAJOR_UPGRADE");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
    }

    TEST_F(test_boinccas_CARestoreSetupState,
        No_ProductVersion_UpgradingTo_Set_IS_MAJOR_UPGRADE) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_TRUE(setRegistryValue("UpgradingTo", "1.2.3"));
        EXPECT_EQ("1.2.3", getRegistryValue("UpgradingTo"));
        auto [errorcode, value] = getMsiProperty("ProductVersion");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());

        ASSERT_EQ(0u, executeAction());

        std::tie(errorcode, value) = getMsiProperty("IS_MAJOR_UPGRADE");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("Yes", value);
    }

    TEST_F(test_boinccas_CARestoreSetupState,
        ProductVersion_Less_Than_UpgradingTo_IS_MAJOR_UPGRADE) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_TRUE(setRegistryValue("UpgradingTo", "1.2.3"));
        EXPECT_EQ("1.2.3", getRegistryValue("UpgradingTo"));

        setMsiProperty("ProductVersion", "1.2.2");
        auto [errorcode, value] = getMsiProperty("ProductVersion");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("1.2.2", value);

        ASSERT_EQ(0u, executeAction());

        std::tie(errorcode, value) = getMsiProperty("IS_MAJOR_UPGRADE");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("Yes", value);
    }

    TEST_F(test_boinccas_CARestoreSetupState,
        ProductVersion_Equal_UpgradingTo_No_IS_MAJOR_UPGRADE) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_TRUE(setRegistryValue("UpgradingTo", "1.2.3"));
        EXPECT_EQ("1.2.3", getRegistryValue("UpgradingTo"));

        setMsiProperty("ProductVersion", "1.2.3");
        auto [errorcode, value] = getMsiProperty("ProductVersion");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("1.2.3", value);

        ASSERT_EQ(0u, executeAction());

        std::tie(errorcode, value) = getMsiProperty("IS_MAJOR_UPGRADE");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
    }

    TEST_F(test_boinccas_CARestoreSetupState,
        ProductVersion_Greater_Than_UpgradingTo_No_IS_MAJOR_UPGRADE) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_TRUE(setRegistryValue("UpgradingTo", "1.2.3"));
        EXPECT_EQ("1.2.3", getRegistryValue("UpgradingTo"));

        setMsiProperty("ProductVersion", "1.2.4");
        auto [errorcode, value] = getMsiProperty("ProductVersion");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("1.2.4", value);

        ASSERT_EQ(0u, executeAction());

        std::tie(errorcode, value) = getMsiProperty("IS_MAJOR_UPGRADE");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
    }
#endif
}

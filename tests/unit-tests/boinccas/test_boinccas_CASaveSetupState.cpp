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

namespace test_boinccas_CASaveSetupState {
    class test_boinccas_CASaveSetupState : public test_boinccas_TestBase {
    protected:
        test_boinccas_CASaveSetupState() :
            test_boinccas_TestBase("SaveSetupState") {
        }

        void TearDown() override {
            cleanRegistryKey();
        }
    };

#ifdef BOINCCAS_TEST
    TEST_F(test_boinccas_CASaveSetupState,
        NoPropertiesData_Expect_Default_Values) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_EQ(0u, executeAction());

        EXPECT_TRUE(getRegistryValue("INSTALLDIR").empty());
        EXPECT_TRUE(getRegistryValue("DATADIR").empty());
        EXPECT_EQ("0", getRegistryValue("LAUNCHPROGRAM"));
        EXPECT_TRUE(getRegistryValue("BOINC_MASTER_USERNAME").empty());
        EXPECT_TRUE(getRegistryValue("BOINC_PROJECT_USERNAME").empty());
        EXPECT_EQ("0", getRegistryValue("ENABLELAUNCHATLOGON"));
        EXPECT_EQ("0", getRegistryValue("ENABLESCREENSAVER"));
        EXPECT_EQ("0",
            getRegistryValue("ENABLEPROTECTEDAPPLICATIONEXECUTION3"));
        EXPECT_EQ("0", getRegistryValue("ENABLEUSEBYALLUSERS"));
        EXPECT_EQ("TRUE", getRegistryValue("SETUPSTATESTORED"));
    }

    TEST_F(test_boinccas_CASaveSetupState,
        PropertiesData_Set_Expect_Valid_Values) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("INSTALLDIR", "test");
        auto [errorcode, value] = getMsiProperty("INSTALLDIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("test", value);

        setMsiProperty("DATADIR", "test_data");
        std::tie(errorcode, value) = getMsiProperty("DATADIR");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("test_data", value);

        setMsiProperty("LAUNCHPROGRAM", "1");
        std::tie(errorcode, value) = getMsiProperty("LAUNCHPROGRAM");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        setMsiProperty("BOINC_MASTER_USERNAME", "master_test");
        std::tie(errorcode, value) = getMsiProperty("BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("master_test", value);

        setMsiProperty("BOINC_PROJECT_USERNAME", "project_test");
        std::tie(errorcode, value) = getMsiProperty("BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("project_test", value);

        setMsiProperty("ENABLELAUNCHATLOGON", "1");
        std::tie(errorcode, value) = getMsiProperty("ENABLELAUNCHATLOGON");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        setMsiProperty("ENABLESCREENSAVER", "1");
        std::tie(errorcode, value) = getMsiProperty("ENABLESCREENSAVER");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        setMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3", "1");
        std::tie(errorcode, value) =
            getMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        setMsiProperty("ENABLEUSEBYALLUSERS", "1");
        std::tie(errorcode, value) = getMsiProperty("ENABLEUSEBYALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        EXPECT_EQ(0u, executeAction());

        EXPECT_EQ("test", getRegistryValue("INSTALLDIR"));
        EXPECT_EQ("test_data", getRegistryValue("DATADIR"));
        EXPECT_EQ("1", getRegistryValue("LAUNCHPROGRAM"));
        EXPECT_EQ("master_test", getRegistryValue("BOINC_MASTER_USERNAME"));
        EXPECT_EQ("project_test", getRegistryValue("BOINC_PROJECT_USERNAME"));
        EXPECT_EQ("1", getRegistryValue("ENABLELAUNCHATLOGON"));
        EXPECT_EQ("1", getRegistryValue("ENABLESCREENSAVER"));
        EXPECT_EQ("1",
            getRegistryValue("ENABLEPROTECTEDAPPLICATIONEXECUTION3"));
        EXPECT_EQ("1", getRegistryValue("ENABLEUSEBYALLUSERS"));
        EXPECT_EQ("TRUE", getRegistryValue("SETUPSTATESTORED"));
    }

    TEST_F(test_boinccas_CASaveSetupState,
        InvalidData_Expect_Default_Values) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("LAUNCHPROGRAM", "2");
        auto [errorcode, value] = getMsiProperty("LAUNCHPROGRAM");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("2", value);

        setMsiProperty("ENABLELAUNCHATLOGON", "3");
        std::tie(errorcode, value) = getMsiProperty("ENABLELAUNCHATLOGON");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("3", value);

        setMsiProperty("ENABLESCREENSAVER", "4");
        std::tie(errorcode, value) = getMsiProperty("ENABLESCREENSAVER");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("4", value);

        setMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3", "5");
        std::tie(errorcode, value) =
            getMsiProperty("ENABLEPROTECTEDAPPLICATIONEXECUTION3");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("5", value);

        setMsiProperty("ENABLEUSEBYALLUSERS", "6");
        std::tie(errorcode, value) = getMsiProperty("ENABLEUSEBYALLUSERS");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("6", value);

        EXPECT_EQ(0u, executeAction());

        EXPECT_TRUE(getRegistryValue("INSTALLDIR").empty());
        EXPECT_TRUE(getRegistryValue("DATADIR").empty());
        EXPECT_EQ("0", getRegistryValue("LAUNCHPROGRAM"));
        EXPECT_TRUE(getRegistryValue("BOINC_MASTER_USERNAME").empty());
        EXPECT_TRUE(getRegistryValue("BOINC_PROJECT_USERNAME").empty());
        EXPECT_EQ("0", getRegistryValue("ENABLELAUNCHATLOGON"));
        EXPECT_EQ("0", getRegistryValue("ENABLESCREENSAVER"));
        EXPECT_EQ("0",
            getRegistryValue("ENABLEPROTECTEDAPPLICATIONEXECUTION3"));
        EXPECT_EQ("0", getRegistryValue("ENABLEUSEBYALLUSERS"));
        EXPECT_EQ("TRUE", getRegistryValue("SETUPSTATESTORED"));
    }
#endif
}

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
    class test_boinccas_CASaveExecutionState : public test_boinccas_TestBase {
    protected:
        test_boinccas_CASaveExecutionState() :
            test_boinccas_TestBase("SaveExecutionState") {
        }

        void TearDown() override {
            cleanRegistryKey();
        }
    };

#ifdef BOINCCAS_TEST
    TEST_F(test_boinccas_CASaveExecutionState,
        NoPropertiesData_Expect_Empty_Values) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_EQ(0u, executeAction());

        EXPECT_TRUE(getRegistryValue("LAUNCHPROGRAM").empty());
        EXPECT_TRUE(getRegistryValue("LAUNCHWSLIMAGEINSTALLER").empty());
        EXPECT_TRUE(getRegistryValue("RETURN_REBOOTREQUESTED").empty());
        EXPECT_TRUE(getRegistryValue("RETURN_VALIDATEINSTALL").empty());
        EXPECT_TRUE(getRegistryValue("RETURN_BOINC_MASTER_USERNAME").empty());
        EXPECT_TRUE(getRegistryValue("RETURN_BOINC_PROJECT_USERNAME").empty());
    }

    TEST_F(test_boinccas_CASaveExecutionState,
        PropertiesData_Set_Expect_Valid_Values) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("LAUNCHPROGRAM", "1");
        auto [errorcode, value] = getMsiProperty("LAUNCHPROGRAM");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        setMsiProperty("LAUNCHWSLIMAGEINSTALLER", "1");
        std::tie(errorcode, value) = getMsiProperty("LAUNCHWSLIMAGEINSTALLER");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        setMsiProperty("RETURN_REBOOTREQUESTED", "0");
        std::tie(errorcode, value) = getMsiProperty("RETURN_REBOOTREQUESTED");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("0", value);

        setMsiProperty("RETURN_VALIDATEINSTALL", "1");
        std::tie(errorcode, value) = getMsiProperty("RETURN_VALIDATEINSTALL");
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

        EXPECT_EQ(0u, executeAction());

        EXPECT_EQ("1", getRegistryValue("LAUNCHPROGRAM"));
        EXPECT_EQ("1", getRegistryValue("LAUNCHWSLIMAGEINSTALLER"));
        EXPECT_EQ("0", getRegistryValue("RETURN_REBOOTREQUESTED"));
        EXPECT_EQ("1", getRegistryValue("RETURN_VALIDATEINSTALL"));
        EXPECT_EQ("master_test",
            getRegistryValue("RETURN_BOINC_MASTER_USERNAME"));
        EXPECT_EQ("project_test",
            getRegistryValue("RETURN_BOINC_PROJECT_USERNAME"));
    }

    TEST_F(test_boinccas_CASaveExecutionState,
        REBOOTREQUESTED_Expect_LAUNCHPROGRAM_Empty) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("LAUNCHPROGRAM", "1");
        auto [errorcode, value] = getMsiProperty("LAUNCHPROGRAM");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        setMsiProperty("LAUNCHWSLIMAGEINSTALLER", "1");
        std::tie(errorcode, value) = getMsiProperty("LAUNCHWSLIMAGEINSTALLER");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        setMsiProperty("RETURN_REBOOTREQUESTED", "1");
        std::tie(errorcode, value) = getMsiProperty("RETURN_REBOOTREQUESTED");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        setMsiProperty("RETURN_VALIDATEINSTALL", "1");
        std::tie(errorcode, value) = getMsiProperty("RETURN_VALIDATEINSTALL");
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

        EXPECT_EQ(0u, executeAction());

        EXPECT_TRUE(getRegistryValue("LAUNCHPROGRAM").empty());
        EXPECT_EQ("1", getRegistryValue("LAUNCHWSLIMAGEINSTALLER"));
        EXPECT_EQ("1", getRegistryValue("RETURN_REBOOTREQUESTED"));
        EXPECT_EQ("1", getRegistryValue("RETURN_VALIDATEINSTALL"));
        EXPECT_EQ("master_test",
            getRegistryValue("RETURN_BOINC_MASTER_USERNAME"));
        EXPECT_EQ("project_test",
            getRegistryValue("RETURN_BOINC_PROJECT_USERNAME"));
    }

    TEST_F(test_boinccas_CASaveExecutionState,
        VALIDATEINSTALL_Equal_0_Expect_LAUNCHPROGRAM_Empty) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        setMsiProperty("LAUNCHPROGRAM", "1");
        auto [errorcode, value] = getMsiProperty("LAUNCHPROGRAM");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        setMsiProperty("LAUNCHWSLIMAGEINSTALLER", "1");
        std::tie(errorcode, value) = getMsiProperty("LAUNCHWSLIMAGEINSTALLER");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);

        setMsiProperty("RETURN_REBOOTREQUESTED", "0");
        std::tie(errorcode, value) = getMsiProperty("RETURN_REBOOTREQUESTED");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("0", value);

        setMsiProperty("RETURN_VALIDATEINSTALL", "0");
        std::tie(errorcode, value) = getMsiProperty("RETURN_VALIDATEINSTALL");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("0", value);

        setMsiProperty("BOINC_MASTER_USERNAME", "master_test");
        std::tie(errorcode, value) = getMsiProperty("BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("master_test", value);

        setMsiProperty("BOINC_PROJECT_USERNAME", "project_test");
        std::tie(errorcode, value) = getMsiProperty("BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("project_test", value);

        EXPECT_EQ(0u, executeAction());

        EXPECT_TRUE(getRegistryValue("LAUNCHPROGRAM").empty());
        EXPECT_EQ("1", getRegistryValue("LAUNCHWSLIMAGEINSTALLER"));
        EXPECT_EQ("0", getRegistryValue("RETURN_REBOOTREQUESTED"));
        EXPECT_EQ("0", getRegistryValue("RETURN_VALIDATEINSTALL"));
        EXPECT_EQ("master_test",
            getRegistryValue("RETURN_BOINC_MASTER_USERNAME"));
        EXPECT_EQ("project_test",
            getRegistryValue("RETURN_BOINC_PROJECT_USERNAME"));
    }
#endif
}

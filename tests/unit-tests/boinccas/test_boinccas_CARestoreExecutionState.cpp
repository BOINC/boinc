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

namespace test_boinccas_CARestoreExecutionState {
    class test_boinccas_CARestoreExecutionState : public test_boinccas_TestBase {
    protected:
        test_boinccas_CARestoreExecutionState() :
            test_boinccas_TestBase("RestoreExecutionState") {
        }

        void TearDown() override {
            cleanRegistryKey();
        }
    };

#ifdef BOINCCAS_TEST
    TEST_F(test_boinccas_CARestoreExecutionState,
        NoPreviousData_Expect_Empty_Values) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_EQ(0u, executeAction());

        auto [errorcode, value] = getMsiProperty("LAUNCHPROGRAM");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());

        std::tie(errorcode, value) = getMsiProperty("RETURN_REBOOTREQUESTED");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());

        std::tie(errorcode, value) = getMsiProperty("RETURN_VALIDATEINSTALL");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());

        std::tie(errorcode, value) = getMsiProperty("BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());

        std::tie(errorcode, value) = getMsiProperty("BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
    }

    TEST_F(test_boinccas_CARestoreExecutionState,
        MissingPreviousData_Expect_Empty_Values) {
        ASSERT_TRUE(setRegistryValue("Test", "test"));

        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_EQ(0u, executeAction());

        auto [errorcode, value] = getMsiProperty("LAUNCHPROGRAM");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());

        std::tie(errorcode, value) = getMsiProperty("RETURN_REBOOTREQUESTED");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());

        std::tie(errorcode, value) = getMsiProperty("RETURN_VALIDATEINSTALL");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());

        std::tie(errorcode, value) = getMsiProperty("BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());

        std::tie(errorcode, value) = getMsiProperty("BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_TRUE(value.empty());
    }

    TEST_F(test_boinccas_CARestoreExecutionState,
        PreviousDataSet_Expect_Correct_Values) {
        ASSERT_TRUE(setRegistryValue("LAUNCHPROGRAM", "2"));
        ASSERT_TRUE(setRegistryValue("RETURN_REBOOTREQUESTED", "4"));
        ASSERT_TRUE(setRegistryValue("RETURN_VALIDATEINSTALL", "6"));
        ASSERT_TRUE(setRegistryValue(
            "RETURN_BOINC_MASTER_USERNAME", "test_master"));
        ASSERT_TRUE(setRegistryValue(
            "RETURN_BOINC_PROJECT_USERNAME", "test_project"));

        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_EQ(0u, executeAction());

        auto [errorcode, value] = getMsiProperty("LAUNCHPROGRAM");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("2", value);

        std::tie(errorcode, value) = getMsiProperty("RETURN_REBOOTREQUESTED");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("4", value);

        std::tie(errorcode, value) = getMsiProperty("RETURN_VALIDATEINSTALL");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("6", value);

        std::tie(errorcode, value) = getMsiProperty("BOINC_MASTER_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("test_master", value);

        std::tie(errorcode, value) = getMsiProperty("BOINC_PROJECT_USERNAME");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        EXPECT_EQ("test_project", value);
    }
#endif
}

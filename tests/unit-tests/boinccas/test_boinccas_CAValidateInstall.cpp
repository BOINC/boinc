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

namespace test_boinccas_CAValidateInstall {
    class test_boinccas_CAValidateInstall :
        public test_boinccas_TestBase {
    protected:
        test_boinccas_CAValidateInstall() :
            test_boinccas_TestBase("ValidateInstall") {
        }

        void TearDown() override {
            if (!testDir.empty() && std::filesystem::exists(testDir)) {
                std::filesystem::remove_all(testDir);
            }
        }

        std::filesystem::path testDir;
    };
#define BOINCCAS_TEST
#ifdef BOINCCAS_TEST
    TEST_F(test_boinccas_CAValidateInstall,
        NoProperties_Expect_Fail) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CAValidateInstall,
        NonExistent_InstallDirectory_Expect_Fail) {
        const auto dir =
            std::filesystem::current_path() / "non_existent_directory";
        insertMsiProperties({
            {"INSTALLDIR", dir.string()}
            });

        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CAValidateInstall,
        No_ProductVersion_Set_Expect_Fail) {
        testDir =
            std::filesystem::current_path() / "test_directory";
        insertMsiProperties({
            {"INSTALLDIR", testDir.string()}
            });
        std::filesystem::create_directory(testDir);

        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CAValidateInstall,
        No_BOINC_Component_Set_Expect_Fail) {
        testDir =
            std::filesystem::current_path() / "test_directory";
        insertMsiProperties({
            {"INSTALLDIR", testDir.string()},
            {"ProductVersion", "1.2.3.4"}
            });
        insertMsiComponents({
            {"_BOINCManager", "_BOINCManager_boincmgr.exe"},
            {"_BOINCCMD", "_BOINCCMD_boinccmd.exe"},
            {"_BOINCTray", "_BOINCTray_boinctray.exe"},
            });
        std::filesystem::create_directory(testDir);
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinc.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boincmgr.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinccmd.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinctray.exe");

        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CAValidateInstall,
        No_BOINCManager_Component_Set_Expect_Fail) {
        testDir =
            std::filesystem::current_path() / "test_directory";
        insertMsiProperties({
            {"INSTALLDIR", testDir.string()},
            {"ProductVersion", "1.2.3.4"}
            });
        insertMsiComponents({
            {"_BOINC", "_BOINC_boinc.exe"},
            {"_BOINCCMD", "_BOINCCMD_boinccmd.exe"},
            {"_BOINCTray", "_BOINCTray_boinctray.exe"},
            });
        std::filesystem::create_directory(testDir);
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinc.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boincmgr.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinccmd.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinctray.exe");

        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CAValidateInstall,
        No_BOINCCMD_Component_Set_Expect_Fail) {
        testDir =
            std::filesystem::current_path() / "test_directory";
        insertMsiProperties({
            {"INSTALLDIR", testDir.string()},
            {"ProductVersion", "1.2.3.4"}
            });
        insertMsiComponents({
            {"_BOINCManager", "_BOINCManager_boincmgr.exe"},
            {"_BOINC", "_BOINC_boinc.exe"},
            {"_BOINCTray", "_BOINCTray_boinctray.exe"},
            });
        std::filesystem::create_directory(testDir);
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinc.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boincmgr.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinccmd.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinctray.exe");

        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CAValidateInstall,
        No_BOINCTray_Component_Set_Expect_Fail) {
        testDir =
            std::filesystem::current_path() / "test_directory";
        insertMsiProperties({
            {"INSTALLDIR", testDir.string()},
            {"ProductVersion", "1.2.3.4"}
            });
        insertMsiComponents({
            {"_BOINCManager", "_BOINCManager_boincmgr.exe"},
            {"_BOINCCMD", "_BOINCCMD_boinccmd.exe"},
            {"_BOINC", "_BOINC_boinc.exe"},
            });
        std::filesystem::create_directory(testDir);
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinc.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boincmgr.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinccmd.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinctray.exe");

        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CAValidateInstall,
        No_BOINC_File_Expect_Fail) {
        testDir =
            std::filesystem::current_path() / "test_directory";
        insertMsiProperties({
            {"INSTALLDIR", testDir.string()},
            {"ProductVersion", "1.2.3.4"}
            });
        insertMsiComponents({
            {"_BOINC", "_BOINC_boinc.exe"},
            {"_BOINCManager", "_BOINCManager_boincmgr.exe"},
            {"_BOINCCMD", "_BOINCCMD_boinccmd.exe"},
            {"_BOINCTray", "_BOINCTray_boinctray.exe"},
            });
        std::filesystem::create_directory(testDir);
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boincmgr.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinccmd.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinctray.exe");

        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_NE(0u, executeAction());

        auto [errorcode, value] = getMsiProperty("RETURN_VALIDATEINSTALL");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("0", value);
    }

    TEST_F(test_boinccas_CAValidateInstall,
        No_BOINCManager_File_Expect_Fail) {
        testDir =
            std::filesystem::current_path() / "test_directory";
        insertMsiProperties({
            {"INSTALLDIR", testDir.string()},
            {"ProductVersion", "1.2.3.4"}
            });
        insertMsiComponents({
            {"_BOINC", "_BOINC_boinc.exe"},
            {"_BOINCManager", "_BOINCManager_boincmgr.exe"},
            {"_BOINCCMD", "_BOINCCMD_boinccmd.exe"},
            {"_BOINCTray", "_BOINCTray_boinctray.exe"},
            });
        std::filesystem::create_directory(testDir);
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinc.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinccmd.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinctray.exe");

        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_NE(0u, executeAction());

        auto [errorcode, value] = getMsiProperty("RETURN_VALIDATEINSTALL");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("0", value);
    }

    TEST_F(test_boinccas_CAValidateInstall,
        No_BOINCCMD_File_Expect_Fail) {
        testDir =
            std::filesystem::current_path() / "test_directory";
        insertMsiProperties({
            {"INSTALLDIR", testDir.string()},
            {"ProductVersion", "1.2.3.4"}
            });
        insertMsiComponents({
            {"_BOINC", "_BOINC_boinc.exe"},
            {"_BOINCManager", "_BOINCManager_boincmgr.exe"},
            {"_BOINCCMD", "_BOINCCMD_boinccmd.exe"},
            {"_BOINCTray", "_BOINCTray_boinctray.exe"},
            });
        std::filesystem::create_directory(testDir);
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinc.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boincmgr.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinctray.exe");

        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_NE(0u, executeAction());

        auto [errorcode, value] = getMsiProperty("RETURN_VALIDATEINSTALL");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("0", value);
    }

    TEST_F(test_boinccas_CAValidateInstall,
        No_BOINCTray_File_Expect_Fail) {
        testDir =
            std::filesystem::current_path() / "test_directory";
        insertMsiProperties({
            {"INSTALLDIR", testDir.string()},
            {"ProductVersion", "1.2.3.4"}
            });
        insertMsiComponents({
            {"_BOINC", "_BOINC_boinc.exe"},
            {"_BOINCManager", "_BOINCManager_boincmgr.exe"},
            {"_BOINCCMD", "_BOINCCMD_boinccmd.exe"},
            {"_BOINCTray", "_BOINCTray_boinctray.exe"},
            });
        std::filesystem::create_directory(testDir);
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinc.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boincmgr.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinccmd.exe");

        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_NE(0u, executeAction());

        auto [errorcode, value] = getMsiProperty("RETURN_VALIDATEINSTALL");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("0", value);
    }

    TEST_F(test_boinccas_CAValidateInstall,
        BOINC_No_Version_Expect_Fail) {
        testDir =
            std::filesystem::current_path() / "test_directory";
        insertMsiProperties({
            {"INSTALLDIR", testDir.string()},
            {"ProductVersion", "1.2.3.4"}
            });
        insertMsiComponents({
            {"_BOINC", "_BOINC_boinc.exe"},
            {"_BOINCManager", "_BOINCManager_boincmgr.exe"},
            {"_BOINCCMD", "_BOINCCMD_boinccmd.exe"},
            {"_BOINCTray", "_BOINCTray_boinctray.exe"},
            });
        std::filesystem::create_directory(testDir);
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy.exe",
            testDir / "boinc.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boincmgr.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinccmd.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinctray.exe");

        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_NE(0u, executeAction());

        auto [errorcode, value] = getMsiProperty("RETURN_VALIDATEINSTALL");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("0", value);
    }

    TEST_F(test_boinccas_CAValidateInstall,
        BOINCManager_No_Version_Expect_Fail) {
        testDir =
            std::filesystem::current_path() / "test_directory";
        insertMsiProperties({
            {"INSTALLDIR", testDir.string()},
            {"ProductVersion", "1.2.3.4"}
            });
        insertMsiComponents({
            {"_BOINC", "_BOINC_boinc.exe"},
            {"_BOINCManager", "_BOINCManager_boincmgr.exe"},
            {"_BOINCCMD", "_BOINCCMD_boinccmd.exe"},
            {"_BOINCTray", "_BOINCTray_boinctray.exe"},
            });
        std::filesystem::create_directory(testDir);
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinc.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy.exe",
            testDir / "boincmgr.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinccmd.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinctray.exe");

        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_NE(0u, executeAction());

        auto [errorcode, value] = getMsiProperty("RETURN_VALIDATEINSTALL");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("0", value);
    }

    TEST_F(test_boinccas_CAValidateInstall,
        BOINCCmd_No_Version_Expect_Fail) {
        testDir =
            std::filesystem::current_path() / "test_directory";
        insertMsiProperties({
            {"INSTALLDIR", testDir.string()},
            {"ProductVersion", "1.2.3.4"}
            });
        insertMsiComponents({
            {"_BOINC", "_BOINC_boinc.exe"},
            {"_BOINCManager", "_BOINCManager_boincmgr.exe"},
            {"_BOINCCMD", "_BOINCCMD_boinccmd.exe"},
            {"_BOINCTray", "_BOINCTray_boinctray.exe"},
            });
        std::filesystem::create_directory(testDir);
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinc.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boincmgr.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy.exe",
            testDir / "boinccmd.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinctray.exe");

        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_NE(0u, executeAction());

        auto [errorcode, value] = getMsiProperty("RETURN_VALIDATEINSTALL");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("0", value);
    }

    TEST_F(test_boinccas_CAValidateInstall,
        BOINCTray_No_Version_Expect_Fail) {
        testDir =
            std::filesystem::current_path() / "test_directory";
        insertMsiProperties({
            {"INSTALLDIR", testDir.string()},
            {"ProductVersion", "1.2.3.4"}
            });
        insertMsiComponents({
            {"_BOINC", "_BOINC_boinc.exe"},
            {"_BOINCManager", "_BOINCManager_boincmgr.exe"},
            {"_BOINCCMD", "_BOINCCMD_boinccmd.exe"},
            {"_BOINCTray", "_BOINCTray_boinctray.exe"},
            });
        std::filesystem::create_directory(testDir);
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinc.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boincmgr.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinccmd.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy.exe",
            testDir / "boinctray.exe");

        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_NE(0u, executeAction());

        auto [errorcode, value] = getMsiProperty("RETURN_VALIDATEINSTALL");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("0", value);
    }

    TEST_F(test_boinccas_CAValidateInstall,
        Correct_Version_Expect_Success) {
        testDir =
            std::filesystem::current_path() / "test_directory";
        insertMsiProperties({
            {"INSTALLDIR", testDir.string()},
            {"ProductVersion", "1.2.3.4"}
            });
        insertMsiComponents({
            {"_BOINC", "_BOINC_boinc.exe"},
            {"_BOINCManager", "_BOINCManager_boincmgr.exe"},
            {"_BOINCCMD", "_BOINCCMD_boinccmd.exe"},
            {"_BOINCTray", "_BOINCTray_boinctray.exe"},
            });
        std::filesystem::create_directory(testDir);
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinc.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boincmgr.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinccmd.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinctray.exe");

        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_EQ(0u, executeAction());

        auto [errorcode, value] = getMsiProperty("RETURN_VALIDATEINSTALL");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("1", value);
    }

    TEST_F(test_boinccas_CAValidateInstall,
        Wrong_Version_Expect_Fail) {
        testDir =
            std::filesystem::current_path() / "test_directory";
        insertMsiProperties({
            {"INSTALLDIR", testDir.string()},
            {"ProductVersion", "4.3.2.1"}
            });
        insertMsiComponents({
            {"_BOINC", "_BOINC_boinc.exe"},
            {"_BOINCManager", "_BOINCManager_boincmgr.exe"},
            {"_BOINCCMD", "_BOINCCMD_boinccmd.exe"},
            {"_BOINCTray", "_BOINCTray_boinctray.exe"},
            });
        std::filesystem::create_directory(testDir);
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinc.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boincmgr.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinccmd.exe");
        std::filesystem::copy(
            std::filesystem::current_path() / "unittest_dummy_gui.exe",
            testDir / "boinctray.exe");

        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_NE(0u, executeAction());

        auto [errorcode, value] = getMsiProperty("RETURN_VALIDATEINSTALL");
        EXPECT_EQ(static_cast<unsigned int>(ERROR_SUCCESS), errorcode);
        ASSERT_EQ("0", value);
    }
#endif
}

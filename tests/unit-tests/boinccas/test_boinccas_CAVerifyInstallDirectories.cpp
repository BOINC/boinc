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
    class test_boinccas_CAVerifyInstallDirectories :
        public test_boinccas_TestBase {
    protected:
        test_boinccas_CAVerifyInstallDirectories() :
            test_boinccas_TestBase("VerifyInstallDirectories") {
        }
    };

#ifdef BOINCCAS_TEST
    TEST_F(test_boinccas_CAVerifyInstallDirectories,
        INSTALLDIR_And_DATADIR_Correctly_Set) {
        auto installdir = std::filesystem::current_path() / "test_install_dir";
        auto datadir = std::filesystem::current_path() / "test_data_dir";

        insertMsiProperties({
            {"INSTALLDIR", installdir.string()},
            {"DATADIR", datadir.string()}
            });

        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        EXPECT_EQ(0u, executeAction());

        auto [errorcode, returnValue] =
            getMsiProperty("RETURN_VERIFYINSTALLDIRECTORIES");
        ASSERT_EQ(0u, errorcode);
        EXPECT_EQ("1", returnValue);
    }

    TEST_F(test_boinccas_CAVerifyInstallDirectories,
        INSTALLDIR_Not_Set_Expect_Fail) {
        auto datadir = std::filesystem::current_path() / "test_data_dir";

        insertMsiProperties({
            {"DATADIR", datadir.string()}
            });

        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        EXPECT_NE(0u, executeAction());

        auto [errorcode, returnValue] =
            getMsiProperty("RETURN_VERIFYINSTALLDIRECTORIES");
        ASSERT_EQ(0u, errorcode);
        EXPECT_EQ("0", returnValue);
    }

    TEST_F(test_boinccas_CAVerifyInstallDirectories,
        DATADIR_Not_Set_Expect_Fail) {
        auto installdir = std::filesystem::current_path() / "test_install_dir";

        insertMsiProperties({
            {"INSTALLDIR", installdir.string()}
            });

        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        EXPECT_NE(0u, executeAction());

        auto [errorcode, returnValue] =
            getMsiProperty("RETURN_VERIFYINSTALLDIRECTORIES");
        ASSERT_EQ(0u, errorcode);
        EXPECT_EQ("0", returnValue);
    }

    TEST_F(test_boinccas_CAVerifyInstallDirectories,
        INSTALLDIR_And_DATADIR_Are_The_Same_Expect_Fail) {
        auto installdir = std::filesystem::current_path() / "test_install_dir";
        auto datadir = installdir;

        insertMsiProperties({
            {"INSTALLDIR", installdir.string()},
            {"DATADIR", datadir.string()}
            });

        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        EXPECT_NE(0u, executeAction());

        auto [errorcode, returnValue] =
            getMsiProperty("RETURN_VERIFYINSTALLDIRECTORIES");
        ASSERT_EQ(0u, errorcode);
        EXPECT_EQ("0", returnValue);
    }

    TEST_F(test_boinccas_CAVerifyInstallDirectories,
        INSTALLDIR_Is_Windows_Dir_Expect_Fail) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        auto [errorcode, installdir] = getMsiProperty("WindowsFolder");
        auto datadir = std::filesystem::current_path() / "test_data_dir";

        insertMsiProperties({
            {"INSTALLDIR", installdir},
            {"DATADIR", datadir.string()}
            });

        EXPECT_NE(0u, executeAction());

        auto [returncode, returnValue] =
            getMsiProperty("RETURN_VERIFYINSTALLDIRECTORIES");
        ASSERT_EQ(0u, returncode);
        EXPECT_EQ("0", returnValue);
    }

    TEST_F(test_boinccas_CAVerifyInstallDirectories,
        DATADIR_Is_Windows_Dir_Expect_Fail) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        auto [errorcode, datadir] = getMsiProperty("WindowsFolder");
        auto installdir = std::filesystem::current_path() / "test_install_dir";

        insertMsiProperties({
            {"INSTALLDIR", installdir.string()},
            {"DATADIR", datadir}
            });

        EXPECT_NE(0u, executeAction());

        auto [returncode, returnValue] =
            getMsiProperty("RETURN_VERIFYINSTALLDIRECTORIES");
        ASSERT_EQ(0u, returncode);
        EXPECT_EQ("0", returnValue);
    }

    TEST_F(test_boinccas_CAVerifyInstallDirectories,
        INSTALLDIR_Is_Windows_Volume_Expect_Fail) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        auto [errorcode, installdir] = getMsiProperty("WindowsVolume");
        auto datadir = std::filesystem::current_path() / "test_data_dir";

        insertMsiProperties({
            {"INSTALLDIR", installdir},
            {"DATADIR", datadir.string()}
            });

        EXPECT_NE(0u, executeAction());

        auto [returncode, returnValue] =
            getMsiProperty("RETURN_VERIFYINSTALLDIRECTORIES");
        ASSERT_EQ(0u, returncode);
        EXPECT_EQ("0", returnValue);
    }

    TEST_F(test_boinccas_CAVerifyInstallDirectories,
        DATADIR_Is_Windows_Volume_Expect_Fail) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        auto [errorcode, datadir] = getMsiProperty("WindowsVolume");
        auto installdir = std::filesystem::current_path() / "test_install_dir";

        insertMsiProperties({
            {"INSTALLDIR", installdir.string()},
            {"DATADIR", datadir}
            });

        EXPECT_NE(0u, executeAction());

        auto [returncode, returnValue] =
            getMsiProperty("RETURN_VERIFYINSTALLDIRECTORIES");
        ASSERT_EQ(0u, returncode);
        EXPECT_EQ("0", returnValue);
    }

    TEST_F(test_boinccas_CAVerifyInstallDirectories,
        INSTALLDIR_Is_System_Dir_Expect_Fail) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        auto [errorcode, installdir] = getMsiProperty("System64Folder");
        auto datadir = std::filesystem::current_path() / "test_data_dir";

        insertMsiProperties({
            {"INSTALLDIR", installdir},
            {"DATADIR", datadir.string()}
            });

        EXPECT_NE(0u, executeAction());

        auto [returncode, returnValue] =
            getMsiProperty("RETURN_VERIFYINSTALLDIRECTORIES");
        ASSERT_EQ(0u, returncode);
        EXPECT_EQ("0", returnValue);
    }

    TEST_F(test_boinccas_CAVerifyInstallDirectories,
        DATADIR_Is_System_Dir_Expect_Fail) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        auto [errorcode, datadir] = getMsiProperty("System64Folder");
        auto installdir = std::filesystem::current_path() / "test_install_dir";

        insertMsiProperties({
            {"INSTALLDIR", installdir.string()},
            {"DATADIR", datadir}
            });

        EXPECT_NE(0u, executeAction());

        auto [returncode, returnValue] =
            getMsiProperty("RETURN_VERIFYINSTALLDIRECTORIES");
        ASSERT_EQ(0u, returncode);
        EXPECT_EQ("0", returnValue);
    }

    TEST_F(test_boinccas_CAVerifyInstallDirectories,
        INSTALLDIR_Is_ProgramFiles_Dir_Expect_Fail) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        auto [errorcode, installdir] = getMsiProperty("ProgramFiles64Folder");
        auto datadir = std::filesystem::current_path() / "test_data_dir";

        insertMsiProperties({
            {"INSTALLDIR", installdir},
            {"DATADIR", datadir.string()}
            });

        EXPECT_NE(0u, executeAction());

        auto [returncode, returnValue] =
            getMsiProperty("RETURN_VERIFYINSTALLDIRECTORIES");
        ASSERT_EQ(0u, returncode);
        EXPECT_EQ("0", returnValue);
    }

    TEST_F(test_boinccas_CAVerifyInstallDirectories,
        DATADIR_Is_ProgramFiles_Dir_Expect_Fail) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        auto [errorcode, datadir] = getMsiProperty("ProgramFiles64Folder");
        auto installdir = std::filesystem::current_path() / "test_install_dir";

        insertMsiProperties({
            {"INSTALLDIR", installdir.string()},
            {"DATADIR", datadir}
            });

        EXPECT_NE(0u, executeAction());

        auto [returncode, returnValue] =
            getMsiProperty("RETURN_VERIFYINSTALLDIRECTORIES");
        ASSERT_EQ(0u, returncode);
        EXPECT_EQ("0", returnValue);
    }
#endif
}

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
#include <openssl/evp.h>
#include <thread>

namespace test_boinccas {
    class test_boinccas_CALaunchBOINCWSLInstaller :
        public test_boinccas_TestBase {
    protected:
        const std::string executableName =
            "boinc-buda-runner-wsl-installer.exe";

        test_boinccas_CALaunchBOINCWSLInstaller() :
            test_boinccas_TestBase("LaunchBOINCWSLInstaller") {}

        void TearDown() override {
            const auto processId = findProcessByName(executableName);
            if (processId != 0) {
                EXPECT_TRUE(killProcessById(processId));
            }
            for (auto i = 0u; i < 5u; ++i) {
                if (!isProcessRunning(executableName)) {
                    break;
                }
                Sleep(1000);
            }
            if (!testDir.empty() && std::filesystem::exists(testDir)) {
                std::filesystem::remove_all(testDir);
            }
        }

        DWORD findProcessByName(const std::string& processName) {
            const auto snapshot =
                CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (snapshot == INVALID_HANDLE_VALUE) {
                return 0;
            }
            wil::unique_handle snapshotHandle(snapshot);

            PROCESSENTRY32 processEntry;
            processEntry.dwSize = sizeof(PROCESSENTRY32);
            if (!Process32First(snapshot, &processEntry)) {
                return 0;
            }
            do {
                if (processName == processEntry.szExeFile) {
                    return processEntry.th32ProcessID;
                }
            } while (Process32Next(snapshot, &processEntry));
            return 0;
        }

        bool isProcessRunning(const std::string& processName) {
            return findProcessByName(processName) != 0;
        }

        bool killProcessById(DWORD processId) {
            const auto processHandle =
                OpenProcess(PROCESS_TERMINATE, FALSE, processId);
            if (!processHandle) {
                return false;
            }
            wil::unique_handle processHandleWrapper(processHandle);
            return TerminateProcess(processHandleWrapper.get(), 0) == TRUE;
        }

        std::string getFileHash(const std::filesystem::path& filePath) {
            std::ifstream file(filePath, std::ios::binary);
            if (!file) {
                return {};
            }

            std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> ctx(
                EVP_MD_CTX_new(), EVP_MD_CTX_free);
            if (!ctx) {
                return {};
            }

            if (EVP_DigestInit_ex(ctx.get(), EVP_sha256(), nullptr) != 1) {
                return {};
            }
            constexpr auto bufferSize = 8192u;
            std::array<char, bufferSize> buffer;
            while (file.read(buffer.data(), bufferSize)) {
                if (EVP_DigestUpdate(ctx.get(), buffer.data(),
                    file.gcount()) != 1) {
                    return {};
                }
            }
            if (EVP_DigestUpdate(ctx.get(), buffer.data(),
                file.gcount()) != 1) {
                return {};
            }
            std::array<unsigned char, EVP_MAX_MD_SIZE> hash;
            unsigned int hashSize = 0;
            if (EVP_DigestFinal_ex(ctx.get(), hash.data(), &hashSize) != 1) {
                return {};
            }
            std::ostringstream hashStringStream;
            for (size_t i = 0; i < hashSize; ++i) {
                hashStringStream << std::hex << std::setw(2)
                    << std::setfill('0') << std::nouppercase
                    << static_cast<int>(hash[i]);
            }
            return "sha256:" + hashStringStream.str();
        }

        std::filesystem::path testDir;
    };

#ifdef BOINCCAS_TEST
    TEST_F(test_boinccas_CALaunchBOINCWSLInstaller,
        Empty_INSTALLDIR_Property) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CALaunchBOINCWSLInstaller,
        NonExistent_INSTALLDIR_Directory) {
        const auto dir =
            std::filesystem::current_path() /= "non_existent_directory";
        insertMsiProperties({
            {"INSTALLDIR", dir.string()}
            });

        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CALaunchBOINCWSLInstaller,
        Empty_INSTALLDIR_Directory) {
        testDir = std::filesystem::current_path() /= "empty";
        std::filesystem::create_directory(testDir);
        insertMsiProperties({
            {"INSTALLDIR", testDir.string()}
            });

        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CALaunchBOINCWSLInstaller,
        NonEmpty_INSTALLDIR_Directory_No_Hash) {
        testDir = std::filesystem::current_path() /= "non_empty";
        std::filesystem::create_directory(testDir);

        ASSERT_TRUE(std::filesystem::copy_file("unittest_dummy_gui.exe",
            testDir / executableName));
        auto executableFound = false;
        for (auto i = 0u; i < 5u; ++i) {
            executableFound =
                std::filesystem::exists(testDir / executableName);
            if (executableFound) {
                break;
            }
            Sleep(1000);
        }
        ASSERT_TRUE(executableFound);

        insertMsiProperties({
            {"INSTALLDIR", testDir.string()}
            });
        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CALaunchBOINCWSLInstaller,
        NonEmpty_INSTALLDIR_Directory_Invalid_Hash) {
        testDir = std::filesystem::current_path() /= "non_empty";
        std::filesystem::create_directory(testDir);

        ASSERT_TRUE(std::filesystem::copy_file("unittest_dummy_gui.exe",
            testDir / executableName));
        auto executableFound = false;
        for (auto i = 0u; i < 5u; ++i) {
            executableFound =
                std::filesystem::exists(testDir / executableName);
            if (executableFound) {
                break;
            }
            Sleep(1000);
        }
        ASSERT_TRUE(executableFound);

        insertMsiProperties({
            {"INSTALLDIR", testDir.string()},
            {"boinc_buda_runner_wsl_installer_checksum",
            "invalid_checksum"}
            });
        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        EXPECT_NE(0u, executeAction());
    }

    TEST_F(test_boinccas_CALaunchBOINCWSLInstaller,
        NonEmpty_INSTALLDIR_Directory_ValidHash) {
        testDir = std::filesystem::current_path() /= "non_empty";
        std::filesystem::create_directory(testDir);

        ASSERT_TRUE(std::filesystem::copy_file("unittest_dummy_gui.exe",
            testDir / executableName));
        auto executableFound = false;
        for (auto i = 0u; i < 5u; ++i) {
            executableFound =
                std::filesystem::exists(testDir / executableName);
            if (executableFound) {
                break;
            }
            Sleep(1000);
        }
        ASSERT_TRUE(executableFound);

        insertMsiProperties({
            {"INSTALLDIR", testDir.string()},
            {"boinc_buda_runner_wsl_installer_checksum",
            getFileHash(testDir / executableName)}
            });
        const auto result = openMsi();
        ASSERT_EQ(0u, result);

        auto processFound = false;
        auto terminatorThread = std::thread([this, &processFound]() {
            for (auto i = 0u; i < 10u; ++i) {
                const auto processId = findProcessByName(executableName);
                if (processId != 0) {
                    processFound = true;
                    Sleep(100);
                    killProcessById(processId);
                    break;
                }
                Sleep(500);
            }
        });

        EXPECT_EQ(0u, executeAction());
        EXPECT_TRUE(processFound);

        if (terminatorThread.joinable()) {
            terminatorThread.join();
        }
    }

#endif
}

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

namespace test_boinccas_CAShutdownBOINC {
    constexpr std::array executables = {
        "boinc.exe",
        "boinctray.exe"
    };
    constexpr auto boincServiceName = "BOINC";
    class test_boinccas_CAShutdownBOINC : public test_boinccas_TestBase {
    protected:
        test_boinccas_CAShutdownBOINC() :
            test_boinccas_TestBase("ShutdownBOINC") {
        }

        void TearDown() override {
            for (const auto& executableName : executables) {
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

                StopServiceByName(boincServiceName);
                Sleep(1000);
                DeleteServiceByName(boincServiceName);
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

        bool StopServiceByName(const std::string& serviceName) {
            auto scm = OpenSCManager(nullptr, nullptr,
                SC_MANAGER_CONNECT);
            if (!scm) {
                return false;
            }
            wil::unique_schandle scmHandle(scm);

            auto service = OpenService(scm, serviceName.c_str(),
                SERVICE_STOP | SERVICE_QUERY_STATUS);
            if (!service) {
                return false;
            }
            wil::unique_schandle serviceHandle(service);

            SERVICE_STATUS_PROCESS status{};
            DWORD bytesNeeded;

            ControlService(service, SERVICE_CONTROL_STOP,
                reinterpret_cast<LPSERVICE_STATUS>(&status));

            auto retry = 0u;
            while (status.dwCurrentState != SERVICE_STOPPED) {
                Sleep(500);

                if (!QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO,
                    reinterpret_cast<LPBYTE>(&status), sizeof(status),
                    &bytesNeeded)) {
                    break;
                }

                if (status.dwCurrentState == SERVICE_STOPPED)
                    break;
                if (++retry > 10) {
                    return false;
                }
            }

            return true;
        }

        bool DeleteServiceByName(const std::string& serviceName) {
            auto scm = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT);
            if (!scm) return false;
            wil::unique_schandle scmHandle(scm);

            auto service = OpenService(scm, serviceName.c_str(), DELETE);
            if (!service) {
                return false;
            }
            wil::unique_schandle serviceHandle(service);

            return DeleteService(service);
        }

        std::filesystem::path testDir;
    };

#ifdef BOINCCAS_TEST
    TEST_F(test_boinccas_CAShutdownBOINC,
        NoRunningProcesses_Expect_Success) {
        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_EQ(0u, executeAction());
    }

    TEST_F(test_boinccas_CAShutdownBOINC,
        TestShutdown) {
        testDir = std::filesystem::current_path() /= "non_empty";
        std::filesystem::create_directory(testDir);

        ASSERT_TRUE(std::filesystem::copy_file("unittest_dummy.exe",
            testDir / "unittest_dummy_child.exe"));
        auto executableFound = false;
        for (auto i = 0u; i < 5u; ++i) {
            if (std::filesystem::exists(
                testDir / "unittest_dummy_child.exe")) {
                executableFound = true;
                break;
            }
            Sleep(1000);
        }
        ASSERT_TRUE(executableFound);

        for (const auto& executableName : executables) {
            ASSERT_TRUE(std::filesystem::copy_file("unittest_dummy.exe",
                testDir / executableName));
            auto executableFound = false;
            for (auto i = 0u; i < 5u; ++i) {
                if (std::filesystem::exists(testDir / executableName)) {
                    executableFound = true;
                    break;
                }
                Sleep(1000);
            }
            ASSERT_TRUE(executableFound);

            STARTUPINFO si = {};
            si.cb = sizeof(si);
            PROCESS_INFORMATION pi = {};
            ASSERT_TRUE(CreateProcess(
                (testDir / executableName).string().c_str(), nullptr, nullptr,
                nullptr, FALSE, 0, nullptr, nullptr, &si, &pi));
            wil::unique_handle processHandle(pi.hProcess);
            wil::unique_handle threadHandle(pi.hThread);
        }

        auto scm = OpenSCManager(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);
        if (!scm) {
            FAIL() << "Failed to open service control manager";
        }
        wil::unique_schandle scmHandle(scm);

        const auto serviceExecutable =
            std::filesystem::current_path() /= "unittest_dummy.exe";
        auto service = CreateService(scm, boincServiceName, boincServiceName,
            SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START,
            SERVICE_ERROR_NORMAL, serviceExecutable.string().c_str(), nullptr,
            nullptr, nullptr, nullptr, nullptr);
        if (!service) {
            FAIL() << "Failed to create service";
        }
        wil::unique_schandle serviceHandle(service);

        auto serviceStarted = false;
        for (auto i = 0u; i < 5u; ++i) {
            if (StartService(serviceHandle.get(), 0, nullptr)) {
                serviceStarted = true;
                break;
            }
            Sleep(1000);
        }
        ASSERT_TRUE(serviceStarted);

        SERVICE_STATUS_PROCESS status;
        DWORD bytesNeeded;

        serviceStarted = false;
        for (auto i = 0u; i < 5u; ++i) {
            QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO,
                reinterpret_cast<LPBYTE>(&status), sizeof(status),
                &bytesNeeded);
            if (status.dwCurrentState == SERVICE_RUNNING) {
                serviceStarted = true;
                break;
            }
            Sleep(1000);
        }
        ASSERT_TRUE(serviceStarted);

        const auto result = openMsi();
        ASSERT_EQ(0u, result);
        ASSERT_EQ(0u, executeAction());

        for (const auto& executableName : executables) {
            auto processFound = true;
            for (auto i = 0u; i < 5u; ++i) {
                if (!isProcessRunning(executableName)) {
                    processFound = false;
                    break;
                }
                Sleep(1000);
            }
            EXPECT_FALSE(processFound);
        }

        auto processFound = true;
        for (auto i = 0u; i < 5u; ++i) {
            if (!isProcessRunning("unittest_dummy_child.exe")) {
                processFound = false;
                break;
            }
            Sleep(1000);
        }
        EXPECT_FALSE(processFound);

        auto serviceStopped = false;
        for (auto i = 0u; i < 5u; ++i) {
            QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO,
                reinterpret_cast<LPBYTE>(&status), sizeof(status),
                &bytesNeeded);
            if (status.dwCurrentState == SERVICE_STOPPED) {
                serviceStopped = true;
                break;
            }
            Sleep(1000);
        }
        EXPECT_TRUE(serviceStopped);
    }
#endif
}

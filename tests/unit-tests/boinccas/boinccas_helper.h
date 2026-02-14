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

#pragma once

std::string getRegistryValue(const std::string& valueName);
bool setRegistryValue(const std::string& valueName,
    const std::string& valueData);
void cleanRegistryKey();

bool userExists(const std::string& username);
bool userCreate(const std::string& username, const std::string& password);
bool userDelete(const std::string& username);
wil::unique_sid getUserSid(const std::string& username);
std::string getCurrentUserSidString();
bool isAccountMemberOfLocalGroup(const std::string& accountName,
    const std::string& groupName);
std::string getLocalizedUsersGroupName();
std::string getLocalizedAdministratorsGroupName();
bool localGroupExists(const std::string& groupName);
bool createLocalGroup(const std::string& groupName);
bool deleteLocalGroup(const std::string& groupName);
bool addUserToTheBuiltinAdministratorsGroup(wil::unique_sid&& userSid);

class MsiHelper {
public:
    MsiHelper();
    ~MsiHelper();
    void insertProperties(
        const std::vector<std::pair<std::string, std::string>>& properties);
    std::tuple<unsigned int, std::string> getProperty(MSIHANDLE hMsiHandle,
        const std::string& propertyName);
    void setProperty(MSIHANDLE hMsiHandle, const std::string& propertyName,
        const std::string& propertyValue);

    std::string getMsiHandle() const {
        return "#" + std::to_string(hMsi);
    }

private:
    void init();
    void cleanup();
    void fillSummaryInformationTable();
    void createPropertiesTable();
    void createTable(const std::string_view& sql_create);
    MSIHANDLE hMsi = 0;
    INSTALLUILEVEL originalUiLevel;
};

class test_boinccas_Base {
    using boinccasFn = UINT(WINAPI*)(MSIHANDLE);
public:
    test_boinccas_Base() = delete;
    virtual ~test_boinccas_Base() = default;
protected:
    test_boinccas_Base(std::string_view functionName) {
        wil::unique_hmodule dll(LoadLibrary("boinccas.dll"));
        if (!dll) {
            throw std::runtime_error("Failed to load boinccas.dll");
        }
        auto func = reinterpret_cast<boinccasFn>(GetProcAddress(dll.get(),
            functionName.data()));
        if (!func) {
            throw std::runtime_error("Failed to load function: " +
                std::string(functionName));
        }
        hDll = std::move(dll);
        hFunc = func;
    }

    auto openMsi() {
        return MsiOpenPackage(msiHelper.getMsiHandle().c_str(), &hMsi);
    }

    auto executeAction() {
        return hFunc(hMsi);
    }

    void insertMsiProperties(
        const std::vector<std::pair<std::string, std::string>>& properties) {
        msiHelper.insertProperties(properties);
    }

    std::tuple<unsigned int, std::string> getMsiProperty(
        const std::string& propertyName) {
        return msiHelper.getProperty(hMsi, propertyName);
    }

    void setMsiProperty(const std::string& propertyName,
        const std::string& propertyValue) {
        msiHelper.setProperty(hMsi, propertyName, propertyValue);
    }

    MSIHANDLE getMsiHandle() {
        return hMsi;
    }
private:
    wil::unique_hmodule hDll = nullptr;
    boinccasFn hFunc = nullptr;
    MsiHelper msiHelper;
    PMSIHANDLE hMsi;
};

class test_boinccas_TestBase :
    public test_boinccas_Base, public ::testing::Test {
public:
    test_boinccas_TestBase() = delete;
    virtual ~test_boinccas_TestBase() override = default;
protected:
    test_boinccas_TestBase(std::string_view functionName) :
        test_boinccas_Base(functionName) {
    }
};

template <typename T>
class test_boinccas_TestBase_WithParam :
    public test_boinccas_Base, public ::testing::TestWithParam<T> {
public:
    test_boinccas_TestBase_WithParam() = delete;
    virtual ~test_boinccas_TestBase_WithParam() override = default;
protected:
    test_boinccas_TestBase_WithParam(std::string_view functionName) :
        test_boinccas_Base(functionName) {
    }
};

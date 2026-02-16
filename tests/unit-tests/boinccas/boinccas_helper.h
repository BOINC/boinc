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

#include <NTSecAPI.h>

#include "win_util.h"

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

// Need these lines copied from the wil library
// to avoid symbol conflicts with the LSA functions.
typedef wil::unique_any<LSA_HANDLE,
    decltype(&::LsaClose), ::LsaClose> unique_hlsa;
using lsa_freemem_deleter = wil::function_deleter<
    decltype(&::LsaFreeMemory), LsaFreeMemory>;
template <typename T>
using unique_lsamem_ptr = wistd::unique_ptr<
    wil::details::ensure_trivially_destructible_t<T>, lsa_freemem_deleter>;
// End of lines copied from wil.

LSA_HANDLE GetPolicyHandle();
LSA_UNICODE_STRING toLsaUnicodeString(const std::wstring& str);

std::vector<std::string> getAccountRights(const std::string& username);

template<typename, typename = void>
struct has_value_type : std::false_type {};

template<typename T>
struct has_value_type<T, std::void_t<typename T::value_type>> :
    std::true_type {};

template<
    typename C,
    typename T = typename C::value_type,
    typename = std::enable_if_t<
    has_value_type<C>::value&&
    std::is_convertible<T, std::string>::value
    >
>
bool setAccountRights(const std::string& username, const C& rights) {
    std::cout << ">>1<<" << std::endl;
    auto policyHandle = GetPolicyHandle();
    if (policyHandle == nullptr) {
        return false;
    }
    unique_hlsa pHandle(policyHandle);
    const auto sid = getUserSid(username);
    if (!sid.is_valid()) {
        return false;
    }
    const auto existingRights = getAccountRights(username);
    for (const auto& right : existingRights) {
        if (std::find(rights.cbegin(), rights.cend(), right)
            == rights.cend()) {
            auto rightString =
                toLsaUnicodeString(boinc_ascii_to_wide(right));
            unique_lsamem_ptr<LSA_UNICODE_STRING> pUserRights(&rightString);
            const auto result =
                LsaRemoveAccountRights(
                    policyHandle, sid.get(), FALSE, &rightString, 1);
            if (result != STATUS_SUCCESS && result != ERROR_NO_SUCH_PRIVILEGE) {
                return false;
            }
        }
    }
    std::cout << ">>2<<" << std::endl;
    for (const auto right : rights) {
        if (std::find(existingRights.cbegin(), existingRights.cend(), right) ==
            existingRights.cend()) {
            auto rightString =
                toLsaUnicodeString(boinc_ascii_to_wide(right));
            unique_lsamem_ptr<LSA_UNICODE_STRING> pUserRights(&rightString);
            const auto result =
                LsaAddAccountRights(policyHandle, sid.get(), &rightString, 1);
            if (result != STATUS_SUCCESS) {
                return false;
            }
        }
    }
    return true;
}

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
protected:
    ~test_boinccas_Base() = default;
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
protected:
    ~test_boinccas_TestBase() = default;
    test_boinccas_TestBase(std::string_view functionName) :
        test_boinccas_Base(functionName) {
    }
};

template <typename T>
class test_boinccas_TestBase_WithParam :
    public test_boinccas_Base, public ::testing::TestWithParam<T> {
public:
    test_boinccas_TestBase_WithParam() = delete;
protected:
    ~test_boinccas_TestBase_WithParam() = default;
    test_boinccas_TestBase_WithParam(std::string_view functionName) :
        test_boinccas_Base(functionName) {
    }
};

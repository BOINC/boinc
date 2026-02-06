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

#include <LMaccess.h>
#include <lmerr.h>
#include <LMAPIbuf.h>
#include <sddl.h>
#include <stdexcept>
#include <array>

#include "win_util.h"

MsiHelper::MsiHelper() {
    originalUiLevel = MsiSetInternalUI(INSTALLUILEVEL_NONE, nullptr);
    cleanup();
    init();
}

MsiHelper::~MsiHelper() {
    cleanup();
    MsiSetInternalUI(originalUiLevel, nullptr);
}

void MsiHelper::init() {
    auto result = MsiOpenDatabase(msiName, MSIDBOPEN_CREATE, &hMsi);
    if (result != ERROR_SUCCESS) {
        throw std::runtime_error("MsiOpenDatabase failed: " +
            std::to_string(result));
    }
    fillSummaryInformationTable();
    createPropertiesTable();
    insertProperties({
            {"ProductCode", "{3F18E95A-7D04-4807-839E-23A535627A86}"},
            {"Manufacturer", "Test Manufacturer"},
            {"ProductLanguage", "1033"},
            {"ProductName", "Test"}
        });
    result = MsiDatabaseCommit(hMsi);
    if (result != ERROR_SUCCESS) {
        throw std::runtime_error("MsiDatabaseCommit failed: " +
            std::to_string(result));
    }
}

void MsiHelper::cleanup() {
    if (hMsi != 0) {
        MsiCloseHandle(hMsi);
        hMsi = 0;
    }
    try {
        std::filesystem::remove(std::filesystem::current_path() / msiName);
    }
    catch (const std::exception& ex) {
        throw std::runtime_error("Failed to remove existing MSI file: " +
            std::string(ex.what()));
    }
}

void MsiHelper::createTable(const std::string_view& sql_create) {
    PMSIHANDLE hView;
    auto result = MsiDatabaseOpenView(hMsi, sql_create.data(), &hView);
    if (result != ERROR_SUCCESS) {
        throw std::runtime_error("Error creating view: " +
            std::to_string(result));
    }
    result = MsiViewExecute(hView, 0);
    if (result != ERROR_SUCCESS) {
        throw std::runtime_error("Error executing view: " +
            std::to_string(result));
    }
    result = MsiViewClose(hView);
    if (result != ERROR_SUCCESS) {
        throw std::runtime_error("Error closing view: " +
            std::to_string(result));
    }
    result = MsiDatabaseCommit(hMsi);
    if (result != ERROR_SUCCESS) {
        throw std::runtime_error("MsiDatabaseCommit failed: " +
            std::to_string(result));
    }
}

void MsiHelper::createPropertiesTable() {
    constexpr std::string_view sql_create = "CREATE TABLE `Property` "
        "(`Property` CHAR(72) NOT NULL, `Value` LONGCHAR NOT NULL LOCALIZABLE "
        "PRIMARY KEY `Property`)";
    createTable(sql_create);
}

void MsiHelper::insertProperties(
    const std::vector<std::pair<std::string, std::string>>& properties) {
    constexpr std::string_view sql_insert =
        "INSERT INTO `Property` (`Property`, `Value`) "
        "VALUES (?, ?)";
    PMSIHANDLE hView;
    auto result = MsiDatabaseOpenView(hMsi, sql_insert.data(), &hView);
    if (result != ERROR_SUCCESS) {
        throw std::runtime_error("Error creating view: " +
            std::to_string(result));
    }
    for (const auto& record : properties) {
        const auto hRecord = MsiCreateRecord(2);
        if (hRecord == 0) {
            throw std::runtime_error("Failed to create record");
        }
        result = MsiRecordSetString(hRecord, 1, record.first.c_str());
        if (result != ERROR_SUCCESS) {
            throw std::runtime_error("Failed to set record, errorcode: " +
                std::to_string(result));
        }
        result = MsiRecordSetString(hRecord, 2, record.second.c_str());
        if (result != ERROR_SUCCESS) {
            throw std::runtime_error("Failed to set record, errorcode: " +
                std::to_string(result));
        }

        result = MsiViewExecute(hView, hRecord);
        if (result != ERROR_SUCCESS) {
            throw std::runtime_error("Error inserting record: " +
                std::to_string(result));
        }
        result = MsiCloseHandle(hRecord);
        if (result != ERROR_SUCCESS) {
            throw std::runtime_error("Error closing record: " +
                std::to_string(result));
        }
    }

    result = MsiViewClose(hView);
    if (result != ERROR_SUCCESS) {
        throw std::runtime_error("Error closing view: " +
            std::to_string(result));
    }

    result = MsiDatabaseCommit(hMsi);
    if (result != ERROR_SUCCESS) {
        throw std::runtime_error("MsiDatabaseCommit failed: " +
            std::to_string(result));
    }
}

std::tuple<unsigned int, std::string> MsiHelper::getProperty(
    const std::string& propertyName) {
    return getProperty(hMsi, propertyName);
}

std::tuple<unsigned int, std::string> MsiHelper::getProperty(
    MSIHANDLE hMsiHandle, const std::string& propertyName) {
    DWORD size = 0;
    auto result =
        MsiGetProperty(hMsiHandle, propertyName.c_str(), "", &size);
    if (result == ERROR_MORE_DATA) {
        std::string value(size++, '\0');
        result =
            MsiGetProperty(hMsiHandle, propertyName.c_str(), value.data(),
                &size);
        if (result != ERROR_SUCCESS) {
            return { result, {} };
        }

        return { result, value };
    }

    if (result != ERROR_SUCCESS) {
        return { result, {} };
    }

    return { result, {} };
}

void MsiHelper::setProperty(const std::string& propertyName,
    const std::string& propertyValue) {
    setProperty(hMsi, propertyName, propertyValue);
}

void MsiHelper::setProperty(MSIHANDLE hMsiHandle,
    const std::string& propertyName, const std::string& propertyValue) {
    const auto result = MsiSetProperty(hMsiHandle, propertyName.c_str(),
        propertyValue.c_str());
    if (result != ERROR_SUCCESS) {
        throw std::runtime_error("MsiSetProperty failed: " +
            std::to_string(result));
    }
}

void MsiHelper::fillSummaryInformationTable() {
    PMSIHANDLE hSummaryInfo;
    constexpr auto updateCount = 4;

    auto result = MsiGetSummaryInformation(hMsi, nullptr, updateCount,
        &hSummaryInfo);
    if (result != ERROR_SUCCESS) {
        throw std::runtime_error("MsiGetSummaryInformation failed: " +
            std::to_string(result));
    }
    result = MsiSummaryInfoSetProperty(hSummaryInfo, 2, VT_LPSTR, 0, nullptr,
        "Test");
    if (result != ERROR_SUCCESS) {
        throw std::runtime_error("MsiSummaryInfoSetProperty 2 failed: " +
            std::to_string(result));
    }
    result = MsiSummaryInfoSetProperty(hSummaryInfo, 7, VT_LPSTR, 0, nullptr,
        "Intel;1033");
    if (result != ERROR_SUCCESS) {
        throw std::runtime_error("MsiSummaryInfoSetProperty 7 failed: " +
            std::to_string(result));
    }
    result = MsiSummaryInfoSetProperty(hSummaryInfo, 9, VT_LPSTR, 0, nullptr,
        "{2C4296B7-9E88-4CD8-9FC6-26CE7B053ED1}");
    if (result != ERROR_SUCCESS) {
        throw std::runtime_error("MsiSummaryInfoSetProperty 9 failed: " +
            std::to_string(result));
    }
    result = MsiSummaryInfoSetProperty(hSummaryInfo, 14, VT_I4, 200, nullptr,
        nullptr);
    if (result != ERROR_SUCCESS) {
        throw std::runtime_error("MsiSummaryInfoSetProperty 14 failed: " +
            std::to_string(result));
    }
    result = MsiSummaryInfoPersist(hSummaryInfo);
    if (result != ERROR_SUCCESS) {
        throw std::runtime_error("MsiSummaryInfoPersist failed:" +
            std::to_string(result));
    }
}

constexpr auto registryKey =
"SOFTWARE\\Space Sciences Laboratory, U.C. Berkeley\\BOINC Setup";

std::string getRegistryValue(const std::string& valueName) {
    HKEY hKey = nullptr;
    const auto openResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, registryKey, 0,
        KEY_READ, &hKey);
    if (openResult != ERROR_SUCCESS) {
        return {};
    }

    DWORD type = 0;
    DWORD size = 0;
    auto queryResult = RegQueryValueEx(hKey, valueName.c_str(), nullptr,
        &type, nullptr, &size);
    if (queryResult != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return {};
    }

    std::string value;
    value.resize(size);
    queryResult = RegQueryValueEx(hKey, valueName.c_str(), nullptr, &type,
        reinterpret_cast<LPBYTE>(value.data()), &size);
    RegCloseKey(hKey);

    if (queryResult != ERROR_SUCCESS) {
        return {};
    }

    // Only REG_SZ and REG_EXPAND_SZ are expected; trim at first NUL.
    const auto nulPos = value.find('\0');
    if (nulPos != std::string::npos) {
        value.resize(nulPos);
    }

    return value;
}

bool setRegistryValue(const std::string& valueName,
    const std::string& valueData) {
    HKEY hKey = nullptr;
    const auto createResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, registryKey,
        0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey,
        nullptr);
    if (createResult != ERROR_SUCCESS) {
        return false;
    }
    const auto setResult = RegSetValueEx(hKey, valueName.c_str(), 0,
        REG_SZ, reinterpret_cast<const BYTE*>(valueData.c_str()),
        static_cast<DWORD>(valueData.size() + 1));
    RegCloseKey(hKey);
    return setResult == ERROR_SUCCESS;
}

void cleanRegistryKey() {
    HKEY hKey = nullptr;
    const auto openResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, registryKey, 0,
        KEY_WRITE, &hKey);
    if (openResult != ERROR_SUCCESS) {
        return;
    }
    RegDeleteKey(HKEY_LOCAL_MACHINE, registryKey);
    RegCloseKey(hKey);
}

bool userExists(const std::string& username) {
    auto un = boinc_ascii_to_wide(username);
    LPUSER_INFO_0 pBuf = nullptr;
    const auto result = NetUserGetInfo(nullptr,
        un.c_str(),
        0, reinterpret_cast<LPBYTE*>(&pBuf));
    if (pBuf != nullptr) {
        NetApiBufferFree(pBuf);
    }

    return result == NERR_Success;
}

bool userCreate(const std::string& username, const std::string& password) {
    auto un = boinc_ascii_to_wide(username);
    auto up = boinc_ascii_to_wide(password);
    USER_INFO_1 ui;
    ui.usri1_name = un.data();
    ui.usri1_password = up.data();
    ui.usri1_comment = nullptr;
    ui.usri1_priv = USER_PRIV_USER;
    ui.usri1_home_dir = nullptr;
    ui.usri1_comment = nullptr;
    ui.usri1_flags = UF_SCRIPT;
    ui.usri1_script_path = nullptr;

    DWORD dwParameterError;
    return NetUserAdd(nullptr, 1,
        reinterpret_cast<LPBYTE>(&ui), &dwParameterError) == NERR_Success;
}

bool userDelete(const std::string& username) {
    auto un = boinc_ascii_to_wide(username);
    return NetUserDel(nullptr, un.c_str()) == NERR_Success;
}

wil::unique_sid getCurrentUserSid() {
    wil::unique_handle token = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
        return nullptr;
    }
    DWORD len = 0;
    GetTokenInformation(token.get(), TokenUser, nullptr, 0, &len);
    std::vector<BYTE> buf(len);
    if (!GetTokenInformation(token.get(), TokenUser, buf.data(), len, &len)) {
        return nullptr;
    }
    auto tu = reinterpret_cast<TOKEN_USER*>(buf.data());
    const auto sidLen = GetLengthSid(tu->User.Sid);
    wil::unique_sid sid(static_cast<PSID>(LocalAlloc(LMEM_FIXED, sidLen)));
    CopySid(sidLen, sid.get(), tu->User.Sid);
    return std::move(sid);
}

wil::unique_sid getUserSid(const std::string& username) {
    DWORD sidSize = 0;
    SID_NAME_USE use;
    DWORD domainSize = 0;
    LookupAccountName(nullptr, username.c_str(), nullptr, &sidSize,
        nullptr, &domainSize, &use);
    if (sidSize == 0 || domainSize == 0) {
        return nullptr;
    }
    std::string domainName(domainSize, '\0');
    std::vector<BYTE> sidBuf(sidSize);
    domainSize = 256;
    if (!LookupAccountName(nullptr, username.c_str(), sidBuf.data(),
        &sidSize, domainName.data(), &domainSize, &use)) {
        return nullptr;
    }
    wil::unique_sid sid(static_cast<PSID>(LocalAlloc(LMEM_FIXED, sidSize)));
    CopySid(sidSize, sid.get(), reinterpret_cast<PSID>(sidBuf.data()));
    return std::move(sid);
}

std::string getCurrentUserSidString() {
    LPSTR sidStr = nullptr;
    auto sid = getCurrentUserSid();
    if (!ConvertSidToStringSid(sid.get(), &sidStr)) {
        if (sidStr) {
            LocalFree(sidStr);
        }
        return {};
    }
    std::string result = sidStr ? sidStr : "";
    LocalFree(sidStr);
    return result;
}

bool isAccountMemberOfLocalGroup(const std::string& accountName,
    const std::string& groupName) {
    wil::unique_sid acctSid = nullptr;
    if (accountName.empty()) {
        acctSid = getCurrentUserSid();
        if (!acctSid) {
            return false;
        }
    }
    else {
        acctSid = getUserSid(accountName);
        if (!acctSid) {
            return false;
        }
    }
    LOCALGROUP_MEMBERS_INFO_0* members = nullptr;
    DWORD entries = 0, total = 0;
    const auto gn = boinc_ascii_to_wide(groupName);
    const auto rc = NetLocalGroupGetMembers(nullptr, gn.c_str(), 0,
        reinterpret_cast<LPBYTE*>(&members), MAX_PREFERRED_LENGTH,
        &entries, &total, nullptr);
    if (rc != NERR_Success) {
        if (members) {
            NetApiBufferFree(members);
        }
        return false;
    }
    if (!members) {
        return false;
    }
    auto found = false;
    for (auto i = 0u; i < entries; ++i) {
        if (EqualSid(members[i].lgrmi0_sid, acctSid.get())) {
            found = true;
            break;
        }
    }
    if (members) {
        NetApiBufferFree(members);
    }
    return found;
}

std::string getLocalizedGroupName(wil::unique_sid&& sid) {
    auto nameSize = 0ul;
    auto domainSize = 0ul;
    SID_NAME_USE use;
    LookupAccountSid(nullptr, sid.get(), nullptr, &nameSize,
        nullptr, &domainSize, &use);
    std::string name(static_cast<size_t>(nameSize), '\0');
    std::string domain(static_cast<size_t>(domainSize), '\0');
    if (!LookupAccountSid(nullptr, sid.get(), name.data(), &nameSize,
        domain.data(), &domainSize, &use)) {
        return "Users";
    }
    name.erase(name.find('\0'));
    return name;
}

std::string getLocalizedUsersGroupName() {
    std::array<BYTE, SECURITY_MAX_SID_SIZE> buffer;
    DWORD size = SECURITY_MAX_SID_SIZE;
    if (!CreateWellKnownSid(WinBuiltinUsersSid, nullptr, &buffer,
        &size)) {
        return "Users";
    }

    wil::unique_sid sid(static_cast<PSID>(LocalAlloc(LMEM_FIXED, size)));
    CopySid(size, sid.get(), reinterpret_cast<PSID>(buffer.data()));

    return getLocalizedGroupName(std::move(sid));
}

std::string getLocalizedAdministratorsGroupName() {
    std::array<BYTE, SECURITY_MAX_SID_SIZE> buffer;
    DWORD size = SECURITY_MAX_SID_SIZE;
    if (!CreateWellKnownSid(WinBuiltinAdministratorsSid, nullptr, &buffer,
        &size)) {
        return "Administrators";
    }
    wil::unique_sid sid(static_cast<PSID>(LocalAlloc(LMEM_FIXED, size)));
    CopySid(size, sid.get(), reinterpret_cast<PSID>(buffer.data()));
    return getLocalizedGroupName(std::move(sid));
}

bool localGroupExists(const std::string& groupName) {
    LOCALGROUP_INFO_0* info = nullptr;
    const auto gn = boinc_ascii_to_wide(groupName);
    const auto rc = NetLocalGroupGetInfo(nullptr, gn.c_str(), 0,
        reinterpret_cast<LPBYTE*>(&info));
    if (info) {
        NetApiBufferFree(info);
    }
    return rc == NERR_Success;
}

bool createLocalGroup(const std::string& groupName) {
    auto gn = boinc_ascii_to_wide(groupName);
    LOCALGROUP_INFO_0 info;
    info.lgrpi0_name = gn.data();
    const auto rc = NetLocalGroupAdd(nullptr,
        0,
        reinterpret_cast<LPBYTE>(&info),
        nullptr);
    return rc == NERR_Success || rc == NERR_GroupExists;
}

bool deleteLocalGroup(const std::string& groupName) {
    const auto gn = boinc_ascii_to_wide(groupName);
    auto res = NetLocalGroupDel(nullptr, gn.c_str());
    return res == NERR_Success;
}

bool addUserToTheBuiltinAdministratorsGroup(wil::unique_sid&& userSid) {
    std::array<BYTE, SECURITY_MAX_SID_SIZE> adminSidBuf{};
    DWORD sidSize = SECURITY_MAX_SID_SIZE;
    if (!CreateWellKnownSid(WinBuiltinAdministratorsSid, nullptr,
        adminSidBuf.data(), &sidSize)) {
        return false;
    }

    wil::unique_sid sid(static_cast<PSID>(LocalAlloc(LMEM_FIXED, sidSize)));
    CopySid(sidSize, sid.get(), reinterpret_cast<PSID>(adminSidBuf.data()));
    auto adminGroupName =
        boinc_ascii_to_wide(getLocalizedGroupName(std::move(sid)));


    LOCALGROUP_MEMBERS_INFO_0 member{};
    member.lgrmi0_sid = reinterpret_cast<PSID>(userSid.get());
    const auto rc = NetLocalGroupAddMembers(
        nullptr,
        adminGroupName.c_str(),
        0,
        reinterpret_cast<LPBYTE>(&member),
        1
    );
    if (rc != NERR_Success && rc != ERROR_MEMBER_IN_ALIAS) {
        return false;
    }
    return true;
}

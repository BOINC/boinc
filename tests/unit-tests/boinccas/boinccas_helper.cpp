// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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

#include <iostream>


#include <filesystem>
#include "boinccas_helper.h"
#include <MsiQuery.h>
#include <Windows.h>
#include <stdexcept>

MsiHelper::MsiHelper() {
    cleanup();
    init();
}

MsiHelper::~MsiHelper() {
    cleanup();
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
        std::cerr << "Failed to remove existing MSI file: " << ex.what() << std::endl;
        std::rethrow_exception(std::current_exception());
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

    /*result = MsiCloseHandle(hSummaryInfo);
    if (result != ERROR_SUCCESS) {
        throw std::runtime_error("MsiCloseHandle failed:" +
            std::to_string(result));
    }*/
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

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

#include "msi_helper.h"

constexpr auto msiName = "test.msi";

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
        PMSIHANDLE hRecord = MsiCreateRecord(2);
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
    }

    result = MsiDatabaseCommit(hMsi);
    if (result != ERROR_SUCCESS) {
        throw std::runtime_error("MsiDatabaseCommit failed: " +
            std::to_string(result));
    }
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

static std::string generateGuid() {
    GUID guid;
    if (CoCreateGuid(&guid) != S_OK) {
        throw std::runtime_error("Failed to create GUID");
    }

    std::ostringstream oss;
    oss << std::uppercase << std::hex << std::setfill('0')
        << '{'
        << std::setw(8) << guid.Data1 << '-'
        << std::setw(4) << guid.Data2 << '-'
        << std::setw(4) << guid.Data3 << '-'
        << std::setw(2) << static_cast<int>(guid.Data4[0])
        << std::setw(2) << static_cast<int>(guid.Data4[1]) << '-'
        << std::setw(2) << static_cast<int>(guid.Data4[2])
        << std::setw(2) << static_cast<int>(guid.Data4[3])
        << std::setw(2) << static_cast<int>(guid.Data4[4])
        << std::setw(2) << static_cast<int>(guid.Data4[5])
        << std::setw(2) << static_cast<int>(guid.Data4[6])
        << std::setw(2) << static_cast<int>(guid.Data4[7])
        << '}';

    return oss.str();
}

void MsiHelper::insertComponents(
    const std::vector<std::pair<std::string, std::string>>& components) {
    createComponentTable();
    constexpr auto sql_insert = "INSERT INTO `Component` (`Component`, "
        "`ComponentId`, `Directory_`, `Attributes`, `Condition`, `KeyPath`) "
        "VALUES (?, ?, ?, ?, ?, ?)";
    PMSIHANDLE hView;
    auto result = MsiDatabaseOpenView(hMsi, sql_insert, &hView);
    if (result != ERROR_SUCCESS) {
        throw std::runtime_error("Error creating view: " +
            std::to_string(result));
    }
    for (const auto& record : components) {
        PMSIHANDLE hRecord = MsiCreateRecord(6);
        if (hRecord == 0) {
            throw std::runtime_error("Failed to create record");
        }
        result = MsiRecordSetString(hRecord, 1, record.first.c_str());
        if (result != ERROR_SUCCESS) {
            throw std::runtime_error("Failed to set record, errorcode: " +
                std::to_string(result));
        }
        const auto componentId = generateGuid();
        result = MsiRecordSetString(hRecord, 2, componentId.c_str());
        if (result != ERROR_SUCCESS) {
            throw std::runtime_error("Failed to set record, errorcode: " +
                std::to_string(result));
        }
        result = MsiRecordSetString(hRecord, 3, "INSTALLDIR");
        if (result != ERROR_SUCCESS) {
            throw std::runtime_error("Failed to set record, errorcode: " +
                std::to_string(result));
        }
        result = MsiRecordSetInteger(hRecord, 4, 256);
        if (result != ERROR_SUCCESS) {
            throw std::runtime_error("Failed to set record, errorcode: " +
                std::to_string(result));
        }
        result = MsiRecordSetString(hRecord, 5, "");
        if (result != ERROR_SUCCESS) {
            throw std::runtime_error("Failed to set record, errorcode: " +
                std::to_string(result));
        }
        result = MsiRecordSetString(hRecord, 6, record.second.c_str());
        if (result != ERROR_SUCCESS) {
            throw std::runtime_error("Failed to set record, errorcode: " +
                std::to_string(result));
        }

        result = MsiViewExecute(hView, hRecord);
        if (result != ERROR_SUCCESS) {
            throw std::runtime_error("Error inserting record: " +
                std::to_string(result));
        }
    }

    result = MsiDatabaseCommit(hMsi);
    if (result != ERROR_SUCCESS) {
        throw std::runtime_error("MsiDatabaseCommit failed: " +
            std::to_string(result));
    }
}

void MsiHelper::createComponentTable() {
    constexpr auto sql_create = "CREATE TABLE `Component` "
        "(`Component` CHAR(72) NOT NULL, `ComponentId` CHAR(38), "
        "`Directory_` CHAR(72) NOT NULL, "
        "`Attributes` SHORT NOT NULL, `Condition` CHAR(255), KeyPath CHAR(72) "
        "PRIMARY KEY `Component`)";
    createTable(sql_create);
}

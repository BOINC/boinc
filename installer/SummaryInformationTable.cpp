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

#include <chrono>
#include <iomanip>
#include <iostream>

#include "JsonHelper.h"
#include "GuidHelper.h"
#include "SummaryInformationTable.h"

SummaryInformationTable::SummaryInformationTable(const nlohmann::json& json,
    InstallerStrings& installerStrings, const std::string& platform)
{
    std::cout << "Loading SummaryInformationTable..." << std::endl;

    SYSTEMTIME systemTime;
    GetSystemTime(&systemTime);
    FILETIME fileTime;
    SystemTimeToFileTime(&systemTime, &fileTime);
    summary[1] = JsonHelper::get<int>(json, "codepage");
    summary[2] =
        JsonHelper::get<std::string>(json, "title", installerStrings);
    summary[3] =
        JsonHelper::get<std::string>(json, "subject", installerStrings);
    summary[4] =
        JsonHelper::get<std::string>(json, "author", installerStrings);
    summary[5] = JsonHelper::get<std::string>(json, "keywords");
    summary[6] =
        JsonHelper::get<std::string>(json, "comments", installerStrings);

    auto tmplt = JsonHelper::get<std::string>(json, "template");
    const std::string platform_template = "%%PLATFORM%%";
    const auto index = tmplt.find(platform_template);
    if (index != std::string::npos) {
        tmplt.replace(index, platform_template.size(),
            platform == "ARM64" ? "Arm64" : platform);
    }

    summary[7] = tmplt;
    summary[8] = JsonHelper::get<std::string>(json, "lastauthor");
    summary[9] = GuidHelper::generate_guid();
    summary[11] = fileTime;
    summary[12] = fileTime;
    summary[13] = fileTime;
    const auto pagecount = JsonHelper::get<std::string>(json, "pagecount");
    const auto colon = pagecount.find(':');
    if (colon != std::string::npos) {
        if (platform == "x64") {
            summary[14] = std::stoi(pagecount.substr(0, colon));
        }
        else {
            summary[14] = std::stoi(pagecount.substr(colon + 1));
        }
    }
    else {
        summary[14] = std::stoi(pagecount);
    }
    summary[15] = JsonHelper::get<int>(json, "wordcount");
    summary[16] = JsonHelper::get<int>(json, "charcount");
    summary[18] = JsonHelper::get<std::string>(json, "appname");
    summary[19] = JsonHelper::get<int>(json, "security");
}

bool SummaryInformationTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating SummaryInformationTable" << std::endl;

    MSIHANDLE hSummaryInfo;
    const auto updateCount = static_cast<UINT>(summary.size());

    auto result = MsiGetSummaryInformation(hDatabase, nullptr, updateCount,
        &hSummaryInfo);
    if (result != ERROR_SUCCESS) {
        std::cerr << "MsiGetSummaryInformation failed: " << result
            << std::endl;
        return false;
    }

    for (auto& [id, value] : summary) {
        try {
            std::cout << "Setting property " << id << std::endl;
            if (std::holds_alternative<int>(value)) {
                result = MsiSummaryInfoSetProperty(hSummaryInfo, id, VT_I4,
                    std::get<int>(value), nullptr, nullptr);
            }
            else if (std::holds_alternative<FILETIME>(value)) {
                result = MsiSummaryInfoSetProperty(hSummaryInfo, id,
                    VT_FILETIME, 0, std::get_if<FILETIME>(&value), nullptr);
            }
            else {
                result = MsiSummaryInfoSetProperty(hSummaryInfo, id,
                    VT_LPSTR, 0, nullptr,
                    std::get<std::string>(value).c_str());
            }
        }
        catch (const std::bad_variant_access& e) {
            std::cerr << "bad_variant_access: " << e.what() << std::endl;
            return false;
        }
        if (result != ERROR_SUCCESS) {
            std::cerr << "MsiSummaryInfoSetProperty failed: " << result
                << std::endl;
            return false;
        }
    }

    result = MsiSummaryInfoPersist(hSummaryInfo);
    if (result != ERROR_SUCCESS) {
        std::cerr << "MsiSummaryInfoPersist failed:" << result << std::endl;
        return false;
    }

    result = MsiCloseHandle(hSummaryInfo);
    if (result != ERROR_SUCCESS) {
        std::cerr << "MsiCloseHandle failed:" << result << std::endl;
        return false;
    }

    return true;
}

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

#include "MsiHelper.h"

#include <stdexcept>

MSIHANDLE MsiHelper::MsiRecordSet(const std::vector<std::variant<
    std::string, int, std::filesystem::path>>& values) {
    const auto hRecord = MsiCreateRecord(static_cast<UINT>(values.size()));
    if (hRecord == 0) {
        throw std::runtime_error("Failed to create record");
    }

    auto i = 0u;
    const auto error_message =
        std::string("Failed to set record, errorcode: ");
    for (const auto& value : values) {
        UINT result = 0;
        if (std::holds_alternative<std::string>(value)) {
            result = MsiRecordSetString(hRecord, ++i,
                std::get<std::string>(value).c_str());
        }
        else if (std::holds_alternative<int>(value)) {
            result = MsiRecordSetInteger(hRecord, ++i, std::get<int>(value));
        }
        else if (std::holds_alternative<std::filesystem::path>(value)) {
            result = MsiRecordSetStream(hRecord, ++i,
                std::get<std::filesystem::path>(value).string().c_str());
        }
        if (result != ERROR_SUCCESS) {
            throw std::runtime_error(error_message + std::to_string(result));
        }
    }

    return hRecord;
}

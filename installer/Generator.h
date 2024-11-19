// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2024 University of California
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

#include <string>
#include <vector>
#include <iostream>

#include "GeneratorTable.h"

template <typename V>
class Generator : public GeneratorTable {
public:
    Generator() = default;
    virtual ~Generator() override = default;

    bool generate(MSIHANDLE hDatabase, const std::string& sql_create,
        const std::string& sql_insert, const std::vector<V>& records) {
        MSIHANDLE hView;
        if (!sql_create.empty()) {
            auto result = MsiDatabaseOpenView(hDatabase, sql_create.c_str(),
                &hView);
            if (result != ERROR_SUCCESS) {
                std::cerr << "Error creating view: " << result << std::endl;
                return false;
            }
            result = MsiViewExecute(hView, 0);
            if (result != ERROR_SUCCESS) {
                std::cerr << "Error executing view: " << result << std::endl;
                return false;
            }
            result = MsiViewClose(hView);
            if (result != ERROR_SUCCESS) {
                std::cerr << "Error closing view: " << result << std::endl;
                return false;
            }
        }
        if (!sql_insert.empty() && !records.empty()) {
            auto result = MsiDatabaseOpenView(hDatabase, sql_insert.c_str(),
                &hView);
            if (result != ERROR_SUCCESS) {
                std::cerr << "Error creating view: " << result << std::endl;
                return false;
            }
            for (const auto& record : records) {
                const auto hRecord = record.getRecord();
                result = MsiViewExecute(hView, hRecord);
                if (result != ERROR_SUCCESS) {
                    std::cerr << "Error inserting record: " << result
                        << std::endl;
                    return false;
                }
                result = MsiCloseHandle(hRecord);
                if (result != ERROR_SUCCESS) {
                    std::cerr << "Error closing record: " << result
                        << std::endl;
                    return false;
                }
            }
            result = MsiViewClose(hView);
            if (result != ERROR_SUCCESS) {
                std::cerr << "Error closing view: " << result << std::endl;
                return false;
            }
        }
        return true;
    }

    virtual bool generate(MSIHANDLE hDatabase) override = 0;
};

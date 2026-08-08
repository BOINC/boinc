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

#pragma once

#include "Record.h"

class Validation : public Record {
public:
    explicit Validation(const std::string& table, const std::string& column,
        bool nullable, int minValue, int maxValue, const std::string& keyTable,
        int keyColumn, const std::string& category, const std::string& set,
        const std::string& description);
    ~Validation() = default;
    MSIHANDLE getRecord() const override;
private:
    std::string table{};
    std::string column{};
    bool nullable = false;
    int minValue = MSI_NULL_INTEGER;
    int maxValue = MSI_NULL_INTEGER;
    std::string keyTable{};
    int keyColumn = MSI_NULL_INTEGER;
    std::string category{};
    std::string set{};
    std::string description{};
};

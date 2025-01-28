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

#include "Validation.h"
#include "MsiHelper.h"

Validation::Validation(const std::string& table, const std::string& column,
    bool nullable, int minValue, int maxValue, const std::string& keyTable,
    int keyColumn, const std::string& category, const std::string& set,
    const std::string& description) : table(table), column(column),
    nullable(nullable), minValue(minValue), maxValue(maxValue),
    keyTable(keyTable), keyColumn(keyColumn), category(category), set(set),
    description(description) {
}

MSIHANDLE Validation::getRecord() const {
    const std::string yval = "Y";
    const std::string nval = "N";
    return MsiHelper::MsiRecordSet({ table, column, nullable ? yval : nval,
        minValue, maxValue, keyTable, keyColumn, category, set, description });
}

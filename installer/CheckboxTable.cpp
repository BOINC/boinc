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

#include "CheckboxTable.h"

CheckboxTable::CheckboxTable(const nlohmann::json& json) {
    std::cout << "Loading CheckboxTable..." << std::endl;
    for (const auto& item : json) {
        properties.emplace_back(item);
    }
}

bool CheckboxTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating CheckboxTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `Checkbox` "
        "(`Property` CHAR(72) NOT NULL, `Value` CHAR(64) "
        "PRIMARY KEY `Property`)";
    const auto sql_insert = "INSERT INTO `Checkbox` "
        "(`Property`, `Value`) VALUES (?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, properties);
}

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

#include "ComponentTable.h"

ComponentTable::ComponentTable(const std::vector<Directory>& directories) {
    for (const auto& directory : directories) {
        for (const auto& component : directory.getComponents()) {
            components.push_back(component);
        }
    }
}

bool ComponentTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating ComponentTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `Component` "
        "(`Component` CHAR(72) NOT NULL, `ComponentId` CHAR(38), "
        "`Directory_` CHAR(72) NOT NULL, "
        "`Attributes` SHORT NOT NULL, `Condition` CHAR(255), KeyPath CHAR(72) "
        "PRIMARY KEY `Component`)";
    const auto sql_insert = "INSERT INTO `Component` (`Component`, "
        "`ComponentId`, `Directory_`, `Attributes`, `Condition`, `KeyPath`) "
        "VALUES (?, ?, ?, ?, ?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, components);
}

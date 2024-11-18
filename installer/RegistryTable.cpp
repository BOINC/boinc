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

#include "RegistryTable.h"

RegistryTable::RegistryTable(const std::vector<Directory>& directories) {
    for (const auto& directory : directories) {
        for (const auto& component : directory.getComponents()) {
            for (const auto& registry : component.getRegistries()) {
                registries.push_back(registry);
            }
        }
    }
}

bool RegistryTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating RegistryTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `Registry` "
        "(`Registry` CHAR(72) NOT NULL, `Root` SHORT NOT NULL, "
        "`Key` CHAR(255) NOT NULL LOCALIZABLE, `Name` CHAR(255) LOCALIZABLE, "
        "`Value` LONGCHAR LOCALIZABLE, `Component_` CHAR(72) NOT NULL "
        "PRIMARY KEY `Registry`)";
    const auto sql_insert = "INSERT INTO `Registry` (`Registry`, `Root`, "
        "`Key`, `Name`, `Value`, `Component_`) VALUES (?, ?, ?, ?, ?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, registries);
}

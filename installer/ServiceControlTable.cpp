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

#include "ServiceControlTable.h"

ServiceControlTable::ServiceControlTable(
    const std::vector<Directory>& directories) {
    for (const auto& directory : directories) {
        for (const auto& component : directory.getComponents()) {
            for (const auto& serviceControl : component.getServiceControls()) {
                values.emplace_back(serviceControl);
            }
        }
    }
}

bool ServiceControlTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating ServiceControlTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `ServiceControl` "
        "(`ServiceControl` CHAR(72) NOT NULL, "
        "`Name` CHAR(255) NOT NULL LOCALIZABLE, `Event` SHORT NOT NULL, "
        "`Arguments` CHAR(255) LOCALIZABLE, `Wait` SHORT, "
        "`Component_` CHAR(72) NOT NULL PRIMARY KEY `ServiceControl`)";
    const auto sql_insert = "INSERT INTO `ServiceControl` (`ServiceControl`, "
        "`Name`, `Event`, `Arguments`, `Wait`, `Component_`) "
        "VALUES (?, ?, ?, ?, ?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, values);
}

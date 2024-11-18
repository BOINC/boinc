#include "ServiceInstallTable.h"
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

#include "ServiceInstallTable.h"

ServiceInstallTable::ServiceInstallTable(
    const std::vector<Directory>& directories) {
    for (const auto& directory : directories) {
        for (const auto& component : directory.getComponents()) {
            for (const auto& serviceInstall : component.getServiceInstalls()) {
                values.push_back(serviceInstall);
            }
        }
    }
}

bool ServiceInstallTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating ServiceInstallTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `ServiceInstall` "
        "(`ServiceInstall` CHAR(72) NOT NULL, `Name` CHAR(255) NOT NULL, "
        "`DisplayName` CHAR(255) LOCALIZABLE, `ServiceType` LONG NOT NULL, "
        "`StartType` LONG NOT NULL, `ErrorControl` LONG NOT NULL, "
        "`LoadOrderGroup` CHAR(255), `Dependencies` CHAR(255), "
        "`StartName` CHAR(255), `Password` CHAR(255), `Arguments` CHAR(255), "
        "`Component_` CHAR(72) NOT NULL, `Description` CHAR(255) LOCALIZABLE "
        "PRIMARY KEY `ServiceInstall`)";
    const auto sql_insert = "INSERT INTO `ServiceInstall` (`ServiceInstall`, "
        "`Name`, `DisplayName`, `ServiceType`, `StartType`, `ErrorControl`, "
        "`LoadOrderGroup`, `Dependencies`, `StartName`, `Password`, "
        "`Arguments`, `Component_`, `Description`) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, values);
}

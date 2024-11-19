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

#include "ShortcutTable.h"

ShortcutTable::ShortcutTable(const std::vector<Directory>& directories) {
    for (const auto& directory : directories) {
        for (const auto& component : directory.getComponents()) {
            for (const auto& shortcut : component.getShortcuts()) {
                values.push_back(shortcut);
            }
        }
    }
}

bool ShortcutTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating ShortcutTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `Shortcut` "
        "(`Shortcut` CHAR(72) NOT NULL, `Directory_` CHAR(72) NOT NULL, "
        "`Name` CHAR(128) NOT NULL LOCALIZABLE, "
        "`Component_` CHAR(72) NOT NULL, `Target` CHAR(255) NOT NULL, "
        "`Arguments` CHAR(255), `Description` CHAR(255) LOCALIZABLE, "
        "`Hotkey` SHORT, `Icon_` CHAR(72), `IconIndex` SHORT, "
        "`ShowCmd` SHORT, `WkDir` CHAR(72) NOT NULL, "
        "`DisplayResourceDll` CHAR(255), `DisplayResourceId` SHORT, "
        "`DescriptionResourceDll` CHAR(255), `DescriptionResourceId` SHORT "
        "PRIMARY KEY `Shortcut`)";

    const auto sql_insert = "INSERT INTO `Shortcut` (`Shortcut`, "
        "`Directory_`, `Name`, `Component_`, `Target`, `Arguments`, "
        "`Description`, `Hotkey`, `Icon_`, `IconIndex`, `ShowCmd`, `WkDir`, "
        "`DisplayResourceDll`, `DisplayResourceId`, `DescriptionResourceDll`, "
        "`DescriptionResourceId`) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, values);
}

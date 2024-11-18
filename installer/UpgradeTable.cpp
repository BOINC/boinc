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

#include "UpgradeTable.h"

UpgradeTable::UpgradeTable(const nlohmann::json& json) {
    std::cout << "Loading UpgradeTable..." << std::endl;

    for (const auto& item : json) {
        values.emplace_back(item);
    }
}

bool UpgradeTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating UpgradeTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `Upgrade` "
        "(`UpgradeCode` CHAR(38) NOT NULL, `VersionMin` CHAR(20), "
        "`VersionMax` CHAR(20), `Language` CHAR(255), "
        "`Attributes` LONG NOT NULL, `Remove` CHAR(255), "
        "`ActionProperty` CHAR(72) NOT NULL PRIMARY KEY `UpgradeCode`, "
        "`VersionMin`, `VersionMax`, `Language`, `Attributes`)";
    const auto sql_insert = "INSERT INTO `Upgrade` (`UpgradeCode`, "
        "`VersionMin`, `VersionMax`, `Language`, `Attributes`, `Remove`, "
        "`ActionProperty`) VALUES (?, ?, ?, ?, ?, ?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, values);
}

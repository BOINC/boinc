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

#include "RadioButtonTable.h"

RadioButtonTable::RadioButtonTable(const nlohmann::json& json,
    const InstallerStrings& installerStrings) {
    std::cout << "Loading RadioButtonTable..." << std::endl;
    for (const auto& item : json) {
        properties.emplace_back(item, installerStrings);
    }
}

bool RadioButtonTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating RadioButtonTable..." << std::endl;
    const auto sql_create = "CREATE TABLE `RadioButton` "
        "(`Property` CHAR(72) NOT NULL, `Order` SHORT NOT NULL, "
        "`Value` CHAR(64) NOT NULL, `X` SHORT NOT NULL, `Y` SHORT NOT NULL, "
        "`Width` SHORT NOT NULL, `Height` SHORT NOT NULL, "
        "`Text` CHAR(64) LOCALIZABLE, `Help` CHAR(50) LOCALIZABLE "
        "PRIMARY KEY `Property`, `Order`)";
    const auto sql_insert = "INSERT INTO `RadioButton` (`Property`, `Order`, "
        "`Value`, `X`, `Y`, `Width`, `Height`, `Text`, `Help`) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, properties);
}

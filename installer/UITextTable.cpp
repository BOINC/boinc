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

#include "UITextTable.h"

UITextTable::UITextTable(const nlohmann::json& json,
    InstallerStrings& installerStrings) {
    std::cout << "Loading UITextTable.." << std::endl;
    for (const auto& item : json) {
        uiTexts.emplace_back(item, installerStrings);
    }
}

bool UITextTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating UITextTable.." << std::endl;

    const auto sql_create = "CREATE TABLE `UIText` (`Key` CHAR(72) NOT NULL, "
        "`Text` CHAR(255) LOCALIZABLE PRIMARY KEY `Key`)";
    const auto sql_insert = "INSERT INTO `UIText` (`Key`, `Text`) "
        "VALUES (?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, uiTexts);
}

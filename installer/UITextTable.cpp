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

#include "UITextTable.h"

UITextTable::UITextTable(const nlohmann::json& json,
    InstallerStrings& installerStrings,
    std::shared_ptr<ValidationTable> validationTable) {
    std::cout << "Loading UITextTable.." << std::endl;
    for (const auto& item : json) {
        uiTexts.emplace_back(item, installerStrings);
    }

    const auto tableName = std::string("UIText");
    const auto url = "https://learn.microsoft.com/en-us/windows/win32/msi/uitext-table";
    if (validationTable != nullptr) {
        validationTable->add(Validation(
            tableName,
            "Key",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("A unique key that identifies the particular "
                "string.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Text",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryText,
            "",
            DescriptionWithUrl("The localized version of the string.", url)
        ));
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

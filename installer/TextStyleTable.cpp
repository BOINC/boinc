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

#include "TextStyleTable.h"

TextStyleTable::TextStyleTable(const nlohmann::json& json,
    std::shared_ptr<ValidationTable> validationTable) {
    std::cout << "Loading TextStyleTable..." << std::endl;

    for (const auto& textStyle : json) {
        textStyles.emplace_back(textStyle);
    }

    const auto tableName = std::string("TextStyle");
    const auto url = "https://learn.microsoft.com/en-us/windows/win32/msi/textstyle-table";
    if (validationTable != nullptr) {
        validationTable->add(Validation(
            tableName,
            "TextStyle",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("This column is the name of the font style.",
                url)
        ));
        validationTable->add(Validation(
            tableName,
            "FaceName",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryText,
            "",
            DescriptionWithUrl("A string indicating the name of the font.",
                url)
        ));
        validationTable->add(Validation(
            tableName,
            "Size",
            true,
            0,
            32767,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("The font size measured in points.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Color",
            true,
            0,
            16777215,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("This column specifies the text color "
                "displayed by a Text Control.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "StyleBits",
            true,
            0,
            15,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("A combination of bits indicating the "
                "formatting for the text.", url)
        ));
    }
}

bool TextStyleTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating TextStyleTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `TextStyle` "
        "(`TextStyle` CHAR(72) NOT NULL, `FaceName` CHAR(32) NOT NULL, "
        "`Size` SHORT NOT NULL, `Color` LONG, `StyleBits` SHORT "
        "PRIMARY KEY `TextStyle`)";
    const auto sql_insert = "INSERT INTO `TextStyle` (`TextStyle`, "
        "`FaceName`, `Size`, `Color`, `StyleBits`) VALUES (?, ?, ?, ?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, textStyles);
}

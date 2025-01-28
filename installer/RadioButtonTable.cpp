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

#include "RadioButtonTable.h"

RadioButtonTable::RadioButtonTable(const std::vector<Control>& controls,
    std::shared_ptr<ValidationTable> validationTable) {
    for (const auto& control : controls) {
        for (const auto& radioButton : control.get_radio_buttons()) {
            properties.emplace_back(radioButton);
        }
    }

    const auto tableName = std::string("RadioButton");
    const auto url = "https://learn.microsoft.com/en-us/windows/win32/msi/radiobutton-table";
    if (validationTable != nullptr) {
        validationTable->add(Validation(
            tableName,
            "Property",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("A named property to be tied to this radio "
                "button.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Order",
            false,
            1,
            32767,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("A positive integer used to determine the "
                "ordering of the items within one list.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Value",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryFormatted,
            "",
            DescriptionWithUrl("The value string associated with this button.",
                url)
        ));
        validationTable->add(Validation(
            tableName,
            "X",
            false,
            0,
            32767,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("The horizontal coordinate within the group of "
                "the upper-left corner of the bounding rectangle of the radio "
                "button.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Y",
            false,
            0,
            32767,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("The vertical coordinate within the group of "
                "the upper-left corner of the bounding rectangle of the radio "
                "button.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Width",
            false,
            0,
            32767,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("The width of the button.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Height",
            false,
            0,
            32767,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("The height of the button.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Text",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryFormatted,
            "",
            DescriptionWithUrl("The localizable, visible title to be assigned "
                "to the radio button.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Help",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryText,
            "",
            DescriptionWithUrl("The Help strings used with the button.", url)
        ));
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

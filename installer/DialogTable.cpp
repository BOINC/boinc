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

#include "DialogTable.h"
#include "ControlTable.h"
#include "Generator.h"

DialogTable::DialogTable(const nlohmann::json& json,
    InstallerStrings& installerStrings,
    std::shared_ptr<ValidationTable> validationTable) :
    validationTable(validationTable) {
    std::cout << "Loading DialogTable..." << std::endl;

    for (const auto& dialog : json) {
        dialogs.emplace_back(dialog, installerStrings);
    }

    const auto tableName = std::string("Dialog");
    const auto url = "https://learn.microsoft.com/en-us/windows/win32/msi/dialog-table";
    if (validationTable != nullptr) {
        validationTable->add(Validation(
            tableName,
            "Dialog",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("The primary key and name of the dialog box.",
                url)
        ));
        validationTable->add(Validation(
            tableName,
            "HCentering",
            false,
            0,
            100,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("The horizontal position of the dialog box.",
                url)
        ));
        validationTable->add(Validation(
            tableName,
            "VCentering",
            false,
            0,
            100,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("The vertical position of the dialog box.", url)
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
            DescriptionWithUrl("The width of the rectangular boundary of the "
                "dialog box.", url)
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
            DescriptionWithUrl("The height of the rectangular boundary of the "
                "dialog box.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Attributes",
            true,
            0,
            2147483647,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("A 32-bit word that specifies the attribute "
                "flags to be applied to this dialog box.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Title",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryFormatted,
            "",
            DescriptionWithUrl("A localizable text string specifying the "
                "title to be displayed in the title bar of the dialog box.",
                url)
        ));
        validationTable->add(Validation(
            tableName,
            "Control_First",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "Control",
            2,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("An external key to the second column of the "
                "Control Table.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Control_Default",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "Control",
            2,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("An external key to the second column of the "
                "Control Table.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Control_Cancel",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "Control",
            2,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("An external key to the second column of the "
                "Control Table.", url)
        ));
    }
}

bool DialogTable::generate(MSIHANDLE hDatabase) {
    if (!ControlTable(dialogs, validationTable).generate(hDatabase)) {
        std::cerr << "Failed to generate ControlTable" << std::endl;
        return false;
    }

    std::cout << "Generating DialogsTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `Dialog` "
        "(`Dialog` CHAR(72) NOT NULL, `HCentering` SHORT NOT NULL, "
        "`VCentering` SHORT NOT NULL, `Width` SHORT NOT NULL, "
        "`Height` SHORT NOT NULL, `Attributes` LONG, "
        "`Title` LONGCHAR LOCALIZABLE, `Control_First` CHAR(50) NOT NULL, "
        "`Control_Default` CHAR(50), `Control_Cancel` CHAR(50) "
        "PRIMARY KEY `Dialog`)";
    const auto sql_insert = "INSERT INTO `Dialog` (`Dialog`, `HCentering`, "
        "`VCentering`, `Width`, `Height`, `Attributes`, `Title`, "
        "`Control_First`, `Control_Default`, `Control_Cancel`) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, dialogs);
}



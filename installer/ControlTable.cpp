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

#include "Generator.h"
#include "Control.h"
#include "ControlConditionTable.h"
#include "ControlEventTable.h"
#include "EventMappingTable.h"
#include "RadioButtonTable.h"
#include "ControlTable.h"

ControlTable::ControlTable(const std::vector<Dialog>& dialogs,
    std::shared_ptr<ValidationTable> validationTable) : dialogs(dialogs),
    validationTable(validationTable) {
    const auto tableName = std::string("Control");
    const auto url = "https://learn.microsoft.com/en-us/windows/win32/msi/control-table";
    if (validationTable != nullptr) {
        validationTable->add(Validation(
            tableName,
            "Dialog_",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "Dialog",
            1,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("External key to the first column of the "
                "Dialog table, the name of the dialog box.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Control",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("Name of the control.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Type",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("The type of the control.", url)
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
            DescriptionWithUrl("Horizontal coordinate of the upper-left "
                "corner of the rectangular boundary of the control.", url)
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
            DescriptionWithUrl("Vertical coordinate of the upper-left corner "
                "of the rectangular boundary of the control.", url)
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
            DescriptionWithUrl("Width of the rectangular boundary of the "
                "control.", url)
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
            DescriptionWithUrl("Height of the rectangular boundary of the "
                "control.", url)
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
            DescriptionWithUrl("A 32-bit word that specifies the bit flags to "
                "be applied to this control.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Property",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("The name of a defined property to be linked "
                "to this control.",
                url)
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
            DescriptionWithUrl("A localizable string used to set the initial "
                "text contained in a control.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Control_Next",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "Control",
            2,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("The name of another control on the same "
                "dialog box and an external key to the second column of the "
                "Control table.", url)
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
            DescriptionWithUrl("Optional, localizable text strings that are "
                "used with the Help button.", url)
        ));
    }
}

bool ControlTable::generate(MSIHANDLE hDatabase)
{
    std::vector<Control> controls;
    for (const auto& dialog : dialogs) {
        for (const auto& control : dialog.get_controls()) {
            controls.emplace_back(control);
        }
    }

    if (!ControlConditionTable(controls, validationTable).generate(hDatabase)) {
        std::cerr << "Failed to generate ControlConditionTable" << std::endl;
        return false;
    }
    if (!ControlEventTable(controls, validationTable).generate(hDatabase)) {
        std::cerr << "Failed to generate ControlEventTable" << std::endl;
        return false;
    }
    if (!EventMappingTable(controls, validationTable).generate(hDatabase)) {
        std::cerr << "Failed to generate EventMappingTable" << std::endl;
        return false;
    }
    if (!RadioButtonTable(controls, validationTable).generate(hDatabase)) {
        std::cerr << "Failed to generate RadioButtonTable" << std::endl;
        return false;
    }

    std::cout << "Generating ControlTable..." << std::endl;

    const auto create_sql = "CREATE TABLE `Control` "
        "(`Dialog_` CHAR(72) NOT NULL, `Control` CHAR(50) NOT NULL, "
        "`Type` CHAR(20) NOT NULL, `X` SHORT NOT NULL, `Y` SHORT NOT NULL, "
        "`Width` SHORT NOT NULL, `Height` SHORT NOT NULL, `Attributes` LONG, "
        "`Property` CHAR(50), `Text` LONGCHAR LOCALIZABLE, "
        "`Control_Next` CHAR(50), `Help` CHAR(50) LOCALIZABLE "
        "PRIMARY KEY `Dialog_`, `Control`)";
    const auto insert_sql = "INSERT INTO `Control` (`Dialog_`, `Control`, "
        "`Type`, `X`, `Y`, `Width`, `Height`, `Attributes`, `Property`, "
        "`Text`, `Control_Next`, `Help`) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

    return Generator::generate(hDatabase, create_sql, insert_sql, controls);
}

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
#include "ControlConditionTable.h"

ControlConditionTable::ControlConditionTable(
    const std::vector<Control>& controls,
    std::shared_ptr<ValidationTable> validationTable) : controls(controls) {
    const auto tableName = std::string("ControlCondition");
    const auto url = "https://learn.microsoft.com/en-us/windows/win32/msi/controlcondition-table";
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
            DescriptionWithUrl("An external key to the first column of the "
                "Dialog table.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Control_",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "Control",
            2,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("An external key to the second column of the "
                "Control table.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Action",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryText,
            "Default;Disable;Enable;Hide;Show",
            DescriptionWithUrl("The action that is to be taken on the "
                "control.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Condition",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryCondition,
            "",
            DescriptionWithUrl("A conditional statement that specifies under "
                "which conditions the action should be triggered.", url)
        ));
    }
}

bool ControlConditionTable::generate(MSIHANDLE hDatabase)
{
    std::cout << "Generating ControlConditionTable..." << std::endl;

    std::vector<ControlCondition> conditions;
    for (const auto& control : controls) {
        for (const auto& condition : control.get_conditions()) {
            conditions.emplace_back(condition);
        }
    }

    const auto sql_create = "CREATE TABLE `ControlCondition` "
        "(`Dialog_` CHAR(72) NOT NULL, `Control_` CHAR(50) NOT NULL, "
        "`Action` CHAR(50) NOT NULL, "
        "`Condition` CHAR(255) NOT NULL "
        "PRIMARY KEY Dialog_, Control_, Action, Condition)";
    const auto sql_insert = "INSERT INTO `ControlCondition` "
        "(`Dialog_`, `Control_`, `Action`, `Condition`) VALUES (?, ?, ?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, conditions);
}

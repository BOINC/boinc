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

#include "ControlEventTable.h"

ControlEventTable::ControlEventTable(
    const std::vector<Control>& controls,
    std::shared_ptr<ValidationTable> validationTable) : controls(controls) {
    const auto tableName = std::string("ControlEvent");
    const auto url = "https://learn.microsoft.com/en-us/windows/win32/msi/controlevent-table";
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
            "Event",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryFormatted,
            "",
            DescriptionWithUrl("An identifier that specifies the type of "
                "event that should take place when the user interacts with "
                "the control specified by Dialog_ and Control_.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Argument",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryFormatted,
            "",
            DescriptionWithUrl("A value used as a modifier when triggering a "
                "particular event.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Condition",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryCondition,
            "",
            DescriptionWithUrl("A conditional statement that determines "
                "whether the installer activates the event in the Event "
                "column.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Ordering",
            true,
            0,
            2147483647,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("An integer used to order several events tied "
                "to the same control.", url)
        ));
    }
}

bool ControlEventTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating ControlEventTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `ControlEvent` "
        "(`Dialog_` CHAR(72) NOT NULL, `Control_` CHAR(50) NOT NULL, "
        "`Event` CHAR(50) NOT NULL, `Argument` CHAR(255) NOT NULL, "
        "`Condition` CHAR(255), `Ordering` SHORT "
        "PRIMARY KEY `Dialog_`, `Control_`, `Event`, `Argument`, `Condition`)";
    const auto sql_insert = "INSERT INTO `ControlEvent` (`Dialog_`, "
        "`Control_`, `Event`, `Argument`, `Condition`, `Ordering`) "
        "VALUES (?, ?, ?, ?, ?, ?)";

    std::vector<ControlEvent> controlEvents;
    for (const auto& control : controls) {
        for (const auto& controlEvent : control.get_events()) {
            controlEvents.emplace_back(controlEvent);
        }
    }

    return Generator::generate(hDatabase, sql_create, sql_insert,
        controlEvents);
}

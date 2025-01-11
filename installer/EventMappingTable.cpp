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

#include "EventMappingTable.h"

EventMappingTable::EventMappingTable(const std::vector<Control>& controls,
    std::shared_ptr<ValidationTable> validationTable) : controls(controls) {
    const auto tableName = std::string("EventMapping");
    const auto url = "https://learn.microsoft.com/en-us/windows/win32/msi/eventmapping-table";
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
                "Dialog Table.", url)
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
                "Control Table.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Event",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("This field is an identifier that specifies "
                "the type of event that is subscribed to by the control.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Attribute",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("The Argument of the event is passed as the "
                "argument of the attribute call to change this attribute of "
                "the control.", url)
        ));
    }
}

bool EventMappingTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating EventMappingTable..." << std::endl;

    std::vector<EventMapping> eventMappings;
    for (const auto& control : controls) {
        for (const auto& eventMapping : control.get_event_mappings()) {
            eventMappings.push_back(eventMapping);
        }
    }

    const auto sql_create = "CREATE TABLE `EventMapping` "
        "(`Dialog_` CHAR(72) NOT NULL, `Control_` CHAR(50) NOT NULL, "
        "`Event` CHAR(50) NOT NULL, `Attribute` CHAR(50) NOT NULL "
        "PRIMARY KEY Dialog_, Control_, Event)";
    const auto sql_insert = "INSERT INTO `EventMapping` "
        "(`Dialog_`, `Control_`, `Event`, `Attribute`) VALUES (?, ?, ?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, eventMappings);
}

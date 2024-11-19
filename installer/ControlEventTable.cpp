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
    const std::vector<Control>& controls) noexcept : controls(controls) {}

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

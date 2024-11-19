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

#include "AdvtExecuteSequenceTable.h"

AdvtExecuteSequenceTable::AdvtExecuteSequenceTable(
    const nlohmann::json& json) {
    std::cout << "Loading AdvtExecuteSequenceTable..." << std::endl;

    for (const auto& value : json) {
        actions.emplace_back(value);
    }
}

bool AdvtExecuteSequenceTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating AdvtExecuteSequenceTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `AdvtExecuteSequence` "
        "(`Action` CHAR(72) NOT NULL, `Condition` CHAR(255), "
        "`Sequence` SHORT PRIMARY KEY `Action`)";
    const auto sql_insert = "INSERT INTO `AdvtExecuteSequence` "
        "(`Action`, `Condition`, `Sequence`) VALUES (?, ?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, actions);
}

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

#include "Generator.h"
#include "Control.h"
#include "ControlConditionTable.h"
#include "ControlEventTable.h"
#include "EventMappingTable.h"
#include "RadioButtonTable.h"
#include "ControlTable.h"

ControlTable::ControlTable(const std::vector<Dialog>& dialogs) noexcept :
    dialogs(dialogs) {}

bool ControlTable::generate(MSIHANDLE hDatabase)
{
    std::vector<Control> controls;
    for (const auto& dialog : dialogs) {
        for (const auto& control : dialog.get_controls()) {
            controls.emplace_back(control);
        }
    }

    if (!ControlConditionTable(controls).generate(hDatabase)) {
        std::cerr << "Failed to generate ControlConditionTable" << std::endl;
        return false;
    }
    if (!ControlEventTable(controls).generate(hDatabase)) {
        std::cerr << "Failed to generate ControlEventTable" << std::endl;
        return false;
    }
    if (!EventMappingTable(controls).generate(hDatabase)) {
        std::cerr << "Failed to generate EventMappingTable" << std::endl;
        return false;
    }
    if (!RadioButtonTable(controls).generate(hDatabase)) {
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

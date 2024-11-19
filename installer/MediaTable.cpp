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

#include "MediaTable.h"

MediaTable::MediaTable(const std::vector<Media>& media) : media(media) {
}

bool MediaTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating MediaTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `Media` (`DiskId` SHORT NOT NULL, "
        "`LastSequence` SHORT NOT NULL, `DiskPrompt` CHAR(64) LOCALIZABLE, "
        "`Cabinet` CHAR(255), `VolumeLabel` CHAR(32), `Source` CHAR(72) "
        "PRIMARY KEY `DiskId`)";
    const auto sql_insert = "INSERT INTO `Media` (`DiskId`, `LastSequence`, "
        "`DiskPrompt`, `Cabinet`, `VolumeLabel`, `Source`) "
        "VALUES (?, ?, ?, ?, ?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, media);
}

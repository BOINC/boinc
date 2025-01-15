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

#include "MediaTable.h"

MediaTable::MediaTable(const std::vector<Media>& media,
    std::shared_ptr<ValidationTable> validationTable) : media(media) {
    const auto tableName = std::string("Media");
    const auto url = "https://learn.microsoft.com/en-us/windows/win32/msi/media-table";
    if (validationTable != nullptr) {
        validationTable->add(Validation(
            tableName,
            "DiskId",
            false,
            1,
            32767,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("Determines the sort order for the table.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "LastSequence",
            false,
            0,
            32767,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("File sequence number for the last file for "
                "this media.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "DiskPrompt",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryText,
            "",
            DescriptionWithUrl("The disk name, which is usually the visible "
                "text printed on the disk.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Cabinet",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryCabinet,
            "",
            DescriptionWithUrl("The name of the cabinet if some or all of the "
                "files stored on the media are compressed into a cabinet "
                "file.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "VolumeLabel",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryText,
            "",
            DescriptionWithUrl("The label attributed to the volume.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Source",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryProperty,
            "",
            DescriptionWithUrl("This field is only used by patching and is "
                "otherwise left blank.", url)
        ));
    }
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

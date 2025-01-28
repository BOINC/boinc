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

#include "DirectoryTable.h"
#include "ComponentTable.h"
#include "FeatureComponentsTable.h"
#include "CreateFolderTable.h"
#include "FileTable.h"
#include "FontTable.h"
#include "RegistryTable.h"
#include "RemoveFileTable.h"
#include "ServiceControlTable.h"
#include "ServiceInstallTable.h"
#include "ShortcutTable.h"

DirectoryTable::DirectoryTable(const nlohmann::json& json,
    const std::filesystem::path& root_path,
    const std::filesystem::path& output_path,
    InstallerStrings& installerStrings, const std::string& platform,
    const std::string& configuration,
    std::shared_ptr<ValidationTable> validationTable) :
    root_path(root_path), output_path(output_path), platform(platform),
    configuration(configuration), validationTable(validationTable) {
    std::cout << "Loading DirectoryTable..." << std::endl;
    for (const auto& directory : json) {
        directories.emplace_back(directory, "", installerStrings);
    }

    const auto tableName = std::string("Directory");
    const auto url = "https://learn.microsoft.com/en-us/windows/win32/msi/directory-table";
    if (validationTable != nullptr) {
        validationTable->add(Validation(
            tableName,
            "Directory",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("The Directory column contains a unique "
                "identifier for a directory or directory path.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Directory_Parent",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "Directory",
            1,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("This column is a reference to the directory's "
                "parent directory.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "DefaultDir",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryDefaultDir,
            "",
            DescriptionWithUrl("The DefaultDir column contains the "
                "directory's name (localizable)under the parent directory.",
                url)
        ));
    }
}

bool DirectoryTable::generate(MSIHANDLE hDatabase) {
    if (!ComponentTable(directories, validationTable).generate(hDatabase)) {
        std::cerr << "Failed to generate ComponentTable" << std::endl;
        return false;
    }
    if (!FeatureComponentsTable(directories,validationTable).generate(
        hDatabase)) {
        std::cerr << "Failed to generate FeatureComponentsTable" << std::endl;
        return false;
    }
    if (!CreateFolderTable(directories, validationTable).generate(hDatabase)) {
        std::cerr << "Failed to generate CreateFolderTable" << std::endl;
        return false;
    }
    if (!FileTable(directories, root_path,
        output_path, platform, configuration, validationTable).generate(
            hDatabase)) {
        std::cerr << "Failed to generate FileTable" << std::endl;
        return false;
    }
    if (!FontTable(directories, validationTable).generate(hDatabase)) {
        std::cerr << "Failed to generate FontTable" << std::endl;
        return false;
    }
    if (!RegistryTable(directories, validationTable).generate(hDatabase)) {
        std::cerr << "Failed to generate RegistryTable" << std::endl;
        return false;
    }
    if (!RemoveFileTable(directories, validationTable).generate(hDatabase)) {
        std::cerr << "Failed to generate RemoveFileTable" << std::endl;
        return false;
    }
    if (!ServiceControlTable(directories, validationTable).generate(hDatabase)) {
        std::cerr << "Failed to generate ServiceControlTable" << std::endl;
        return false;
    }
    if (!ServiceInstallTable(directories, validationTable).generate(hDatabase)) {
        std::cerr << "Failed to generate ServiceInstallTable" << std::endl;
        return false;
    }
    if (!ShortcutTable(directories, validationTable).generate(hDatabase)) {
        std::cerr << "Failed to generate ShortcutTable" << std::endl;
        return false;
    }

    std::vector<Directory> all;
    for (const auto& directory : directories) {
        all.push_back(directory);
        auto subdirs = directory.getDirectories();
        all.insert(all.end(), subdirs.begin(), subdirs.end());
    }

    std::cout << "Generating DirectoryTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `Directory` "
        "(`Directory` CHAR(72) NOT NULL, `Directory_Parent` CHAR(72), "
        "`DefaultDir` CHAR(255) NOT NULL LOCALIZABLE PRIMARY KEY `Directory`)";
    const auto sql_insert = "INSERT INTO `Directory` "
        "(`Directory`, `Directory_Parent`, `DefaultDir`) VALUES (?, ?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, all);
}

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

#include <fstream>
#include <iostream>

#include "JsonHelper.h"
#include "SummaryInformationTable.h"
#include "ActionTextTable.h"
#include "AdminExecuteSequenceTable.h"
#include "AdminUISequencetable.h"
#include "AdvtExecuteSequenceTable.h"
#include "BinaryTable.h"
#include "CheckboxTable.h"
#include "ControlTable.h"
#include "CustomActionTable.h"
#include "DialogTable.h"
#include "DirectoryTable.h"
#include "ErrorTable.h"
#include "FeatureTable.h"
#include "IconTable.h"
#include "InstallExecuteSequenceTable.h"
#include "InstallUISequenceTable.h"
#include "LaunchConditionTable.h"
#include "ListboxTable.h"
#include "PropertyTable.h"
#include "RadioButtonTable.h"
#include "TextStyleTable.h"
#include "UITextTable.h"
#include "UpgradeTable.h"
#include "ValidationTable.h"

#include "Installer.h"

#if defined(_DEBUG) && !defined(MSI_VALIDATE)
#define MSI_VALIDATE
#endif

const auto ActionTextTableName = std::string("ActionText");
const auto AdminExecuteSequenceTableName = std::string("AdminExecuteSequence");
const auto AdminUISequenceTableName = std::string("AdminUISequence");
const auto AdvtExecuteSequenceTableName = std::string("AdvtExecuteSequence");
const auto BinaryTableName = std::string("Binary");
const auto CheckboxTableName = std::string("Checkbox");
const auto CustomActionTableName = std::string("CustomAction");
const auto DialogTableName = std::string("Dialog");
const auto DirectoryTableName = std::string("Directory");
const auto ErrorTableName = std::string("Error");
const auto FeatureTableName = std::string("Feature");
const auto IconTableName = std::string("Icon");
const auto InstallExecuteSequenceTableName =
std::string("InstallExecuteSequence");
const auto InstallUISequenceTableName = std::string("InstallUISequence");
const auto LaunchConditionTableName = std::string("LaunchCondition");
const auto ListboxTableName = std::string("Listbox");
const auto PropertyTableName = std::string("Property");
const auto SummaryTableName = std::string("Summary");
const auto TextStyleTableName = std::string("TextStyle");
const auto UITextTableName = std::string("UIText");
const auto UpgradeTableName = std::string("Upgrade");
const auto ValidationTableName = std::string("Validation");

Installer::Installer(const std::filesystem::path& output_path,
    const std::string& platform, const std::string& configuration) :
    output_path(output_path), platform(platform),
    configuration(configuration) {
}

nlohmann::json Installer::load_json(const std::filesystem::path& json) {
    std::ifstream file(json);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file " + json.string());
    }

    nlohmann::json j;
    file >> j;

    if (file.fail()) {
        throw std::runtime_error("Failed reading the file " +
            json.string());
    }

    process_json(j, json.parent_path());

    return j;
}

void Installer::process_json(nlohmann::json& json,
    const std::filesystem::path& parent_file) {
    std::vector<nlohmann::json> include;
    if (json.is_object() || json.is_array()) {
        for (const auto& i : json.items()) {
            if (i.key() == "include") {
                if (i.value().is_array())
                    for (const auto& file : i.value().items()) {
                        include.push_back(load_json(parent_file /
                            file.value()));
                    }
                else if (i.value().is_string())
                    include.push_back(load_json(parent_file /
                        i.value()));
            }
            else {
                process_json(i.value(), parent_file);
            }
        }

    }
    for (const auto& i : include) {
        for (const auto& j : i.items()) {
            if (json.is_object()) {
                json[j.key()] = j.value();
            }
            else if (json.is_array()) {
                json.push_back(j.value());
            }
        }
    }
    if (!include.empty()) {
        json.erase("include");
    }
}

bool Installer::load(const std::filesystem::path& json) {
    try {
        return load_from_json(load_json(json), json.parent_path());
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}

bool Installer::load_from_json(const nlohmann::json& json,
    const std::filesystem::path& path)
{
    if (JsonHelper::exists(json, "Locale")) {
        if (!installer_strings.load(json["Locale"], path)) {
            return false;
        }
    }
    // should always be first
    std::shared_ptr<ValidationTable> validationTable =
#ifndef MSI_VALIDATE
        nullptr;
#else
        std::make_shared<ValidationTable>();
    tables[ValidationTableName] = validationTable;
#endif
    if (JsonHelper::exists(json, SummaryTableName)) {
        tables[SummaryTableName] =
            std::make_shared<SummaryInformationTable>(
                json[SummaryTableName], installer_strings, platform);
    }
    if (JsonHelper::exists(json, ActionTextTableName)) {
        tables[ActionTextTableName] = std::make_shared<ActionTextTable>(
            json[ActionTextTableName], installer_strings, validationTable);
    }
    if (JsonHelper::exists(json, AdminExecuteSequenceTableName)) {
        tables[AdminExecuteSequenceTableName] =
            std::make_shared<AdminExecuteSequenceTable>(
                json[AdminExecuteSequenceTableName], validationTable);
    }
    if (JsonHelper::exists(json, AdminUISequenceTableName)) {
        tables[AdminUISequenceTableName] =
            std::make_shared<AdminUISequenceTable>(
                json[AdminUISequenceTableName], validationTable);
    }
    if (JsonHelper::exists(json, AdvtExecuteSequenceTableName)) {
        tables[AdvtExecuteSequenceTableName] =
            std::make_shared<AdvtExecuteSequenceTable>(
                json[AdvtExecuteSequenceTableName], validationTable);
    }
    if (JsonHelper::exists(json, BinaryTableName)) {
        tables[BinaryTableName] = std::make_shared<BinaryTable>(
            json[BinaryTableName], path, platform, configuration,
            validationTable);
    }
    if (JsonHelper::exists(json, CheckboxTableName)) {
        tables[CheckboxTableName] = std::make_shared<CheckboxTable>(
            json[CheckboxTableName], validationTable);
    }
    if (JsonHelper::exists(json, CustomActionTableName)) {
        tables[CustomActionTableName] =
            std::make_shared<CustomActionTable>(
                json[CustomActionTableName], validationTable);
    }
    if (JsonHelper::exists(json, DialogTableName)) {
        tables[DialogTableName] =
            std::make_shared<DialogTable>(json[DialogTableName],
                installer_strings, validationTable);
    }
    if (JsonHelper::exists(json, DirectoryTableName)) {
        tables[DirectoryTableName] = std::make_shared<DirectoryTable>(
            json[DirectoryTableName], path, output_path, installer_strings,
            platform, configuration, validationTable);
    }
    if (JsonHelper::exists(json, ErrorTableName)) {
        tables[ErrorTableName] =
            std::make_shared<ErrorTable>(json[ErrorTableName],
                installer_strings, validationTable);
    }
    if (JsonHelper::exists(json, FeatureTableName)) {
        tables[FeatureTableName] =
            std::make_shared<FeatureTable>(json[FeatureTableName],
                installer_strings, validationTable);
    }
    if (JsonHelper::exists(json, IconTableName)) {
        tables[IconTableName] =
            std::make_shared<IconTable>(json[IconTableName], path,
                platform, configuration, validationTable);
    }
    if (JsonHelper::exists(json, InstallExecuteSequenceTableName)) {
        tables[InstallExecuteSequenceTableName] =
            std::make_shared<InstallExecuteSequenceTable>(
                json[InstallExecuteSequenceTableName], validationTable);
    }
    if (JsonHelper::exists(json, InstallUISequenceTableName)) {
        tables[InstallUISequenceTableName] =
            std::make_shared<InstallUISequenceTable>(
                json[InstallUISequenceTableName], validationTable);
    }
    if (JsonHelper::exists(json, LaunchConditionTableName)) {
        tables[LaunchConditionTableName] =
            std::make_shared<LaunchConditionTable>(
                json[LaunchConditionTableName], installer_strings,
                validationTable);
    }
    if (JsonHelper::exists(json, ListboxTableName)) {
        tables[ListboxTableName] = std::make_shared<ListboxTable>(
            json[ListboxTableName], installer_strings, validationTable);
    }
    if (JsonHelper::exists(json, PropertyTableName)) {
        tables[PropertyTableName] = std::make_shared<PropertyTable>(
            json[PropertyTableName], installer_strings, validationTable);
    }
    if (JsonHelper::exists(json, TextStyleTableName)) {
        tables[TextStyleTableName] = std::make_shared<TextStyleTable>(
            json[TextStyleTableName], validationTable);
    }
    if (JsonHelper::exists(json, UITextTableName)) {
        tables[UITextTableName] =
            std::make_shared<UITextTable>(json[UITextTableName],
                installer_strings, validationTable);
    }
    if (JsonHelper::exists(json, UpgradeTableName)) {
        tables[UpgradeTableName] = std::make_shared<UpgradeTable>(
            json[UpgradeTableName], validationTable);
    }
    return true;
}

bool Installer::forceCodePage(MSIHANDLE hDatabase) {
    const auto idt_path = output_path / "_ForceCodepage.idt";
    std::ofstream file(idt_path);
    file << std::endl << std::endl << "1252\t_ForceCodepage" << std::endl;
    file.close();
    const auto result = MsiDatabaseImport(hDatabase,
        output_path.string().c_str(), "_ForceCodepage.idt");
    std::filesystem::remove(idt_path);
    if (result != ERROR_SUCCESS) {
        std::cerr << "MsiDatabaseImport failed: " << result << std::endl;
        return false;
    }
    return true;
}

bool Installer::create_msi(const std::filesystem::path& msi) {
    MSIHANDLE hDatabase;

    try {
        if (std::filesystem::exists(msi)) {
            if (!std::filesystem::remove(msi)) {
                std::cerr << "Failed to remove existing file " << msi
                    << std::endl;
                return false;
            }
        }
        auto result = MsiOpenDatabase(msi.string().c_str(), MSIDBOPEN_CREATE,
            &hDatabase);
        if (result != ERROR_SUCCESS) {
            std::cerr << "MsiOpenDatabase failed with error " << result
                << std::endl;
            return false;
        }

        if (!forceCodePage(hDatabase)) {
            return false;
        }

        for (const auto& table : tables) {
            if (table.first == ValidationTableName) {
                continue;
            }
            if (!table.second->generate(hDatabase)) {
                std::cerr << "Failed to write table " << table.first
                    << std::endl;
                return false;
            }
        }

        // should always be last
        if (tables.find(ValidationTableName) != tables.end() &&
            tables[ValidationTableName] != nullptr) {
            if (!tables[ValidationTableName]->generate(hDatabase)) {
                std::cerr << "Failed to write table Validation" << std::endl;
                return false;
            }
        }

        result = MsiDatabaseCommit(hDatabase);
        if (result != ERROR_SUCCESS) {
            std::cerr << "MsiDatabaseCommit failed: " << result << std::endl;
            return false;
        }

        result = MsiCloseHandle(hDatabase);
        if (result != ERROR_SUCCESS) {
            std::cerr << "MsiCloseHandle failed: " << result << std::endl;
            return false;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }

    std::cout << "Created " << msi << std::endl;

    return true;
}

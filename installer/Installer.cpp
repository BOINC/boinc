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
#include "PropertyTable.h"
#include "RadioButtonTable.h"
#include "TextStyleTable.h"
#include "UITextTable.h"
#include "UpgradeTable.h"

#include "Installer.h"

Installer::Installer(const std::filesystem::path& output_path,
    const std::string& platform, const std::string& configuration) :
    output_path(output_path), platform(platform),
    configuration(configuration) {
}

bool Installer::load(const std::filesystem::path& json) {
    std::ifstream file(json);
    if (!file.is_open()) {
        std::cerr << "Could not open file " << json << std::endl;
        return false;
    }

    nlohmann::json j;
    try {
        file >> j;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }

    return load_from_json(j, json.parent_path());
}

bool Installer::load_from_json(const nlohmann::json& json,
    const std::filesystem::path& path)
{
    try {
        if (JsonHelper::exists(json, "Locale")) {
            if (!installer_strings.load(json["Locale"], path)) {
                return false;
            }
        }
        if (JsonHelper::exists(json, "Summary")) {
            tables["Summary"] = std::make_shared<SummaryInformationTable>(
                json["Summary"], installer_strings, platform);
        }
        if (JsonHelper::exists(json, "ActionText")) {
            tables["ActionText"] = std::make_shared<ActionTextTable>(
                json["ActionText"], installer_strings);
        }
        if (JsonHelper::exists(json, "AdminExecuteSequence")) {
            tables["AdminExecuteSequence"] =
                std::make_shared<AdminExecuteSequenceTable>(
                    json["AdminExecuteSequence"]);
        }
        if (JsonHelper::exists(json, "AdminUISequence")) {
            tables["AdminUISequence"] = std::make_shared<AdminUISequenceTable>(
                json["AdminUISequence"]);
        }
        if (JsonHelper::exists(json, "AdvtExecuteSequence")) {
            tables["AdvtExecuteSequence"] =
                std::make_shared<AdvtExecuteSequenceTable>(
                    json["AdvtExecuteSequence"]);
        }
        if (JsonHelper::exists(json, "Binary")) {
            tables["Binary"] = std::make_shared<BinaryTable>(
                json["Binary"], path, platform, configuration);
        }
        if (JsonHelper::exists(json, "Checkbox")) {
            tables["Checkbox"] = std::make_shared<CheckboxTable>(
                json["Checkbox"]);
        }
        if (JsonHelper::exists(json, "CustomAction")) {
            tables["CustomAction"] = std::make_shared<CustomActionTable>(
                json["CustomAction"]);
        }
        if (JsonHelper::exists(json, "Dialog")) {
            tables["Dialog"] = std::make_shared<DialogTable>(json["Dialog"],
                installer_strings);
        }
        if (JsonHelper::exists(json, "Directory")) {
            tables["Directory"] = std::make_shared<DirectoryTable>(
                json["Directory"], path, output_path, installer_strings,
                platform, configuration);
        }
        if (JsonHelper::exists(json, "Error")) {
            tables["Error"] = std::make_shared<ErrorTable>(json["Error"],
                installer_strings);
        }
        if (JsonHelper::exists(json, "Feature")) {
            tables["Feature"] = std::make_shared<FeatureTable>(json["Feature"],
                installer_strings);
        }
        if (JsonHelper::exists(json, "Icon")) {
            tables["Icon"] = std::make_shared<IconTable>(json["Icon"], path,
                platform, configuration);
        }
        if (JsonHelper::exists(json, "InstallExecuteSequence")) {
            tables["InstallExecuteSequence"] =
                std::make_shared<InstallExecuteSequenceTable>(
                    json["InstallExecuteSequence"]);
        }
        if (JsonHelper::exists(json, "InstallUISequence")) {
            tables["InstallUISequence"] =
                std::make_shared<InstallUISequenceTable>(
                    json["InstallUISequence"]);
        }
        if (JsonHelper::exists(json, "LaunchCondition")) {
            tables["LaunchCondition"] = std::make_shared<LaunchConditionTable>(
                json["LaunchCondition"], installer_strings);
        }
        if (JsonHelper::exists(json, "Property")) {
            tables["Property"] = std::make_shared<PropertyTable>(
                json["Property"], installer_strings);
        }
        if (JsonHelper::exists(json, "TextStyle")) {
            tables["TextStyle"] = std::make_shared<TextStyleTable>(
                json["TextStyle"]);
        }
        if (JsonHelper::exists(json, "UIText")) {
            tables["UIText"] = std::make_shared<UITextTable>(json["UIText"],
                installer_strings);
        }
        if (JsonHelper::exists(json, "Upgrade")) {
            tables["Upgrade"] = std::make_shared<UpgradeTable>(
                json["Upgrade"]);
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
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
            if (!table.second->generate(hDatabase)) {
                std::cerr << "Failed to write table " << table.first
                    << std::endl;
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

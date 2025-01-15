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

#include "Component.h"
#include "GuidHelper.h"
#include "MsiHelper.h"
#include "JsonHelper.h"

Component::Component(const nlohmann::json& json, const std::string& directory,
    const std::string& parent, InstallerStrings& installerStrings) :
    directory(directory) {
    JsonHelper::get(json, "Component", component);
    JsonHelper::get(json, "Attributes", attributes);
    JsonHelper::get(json, "Condition", condition);
    JsonHelper::get(json, "Feature_", feature);
    JsonHelper::get(json, "CreateFolder", create_folder);

    if (component.empty()) {
        component = parent + "_" + directory;
    }
    JsonHelper::handle(json, "Files", [&](const auto& file) {
        files.emplace_back(file, component, directory);
        });
    JsonHelper::handle(json, "Registry", [&](const auto& registry) {
        registries.emplace_back(registry, component);
        });
    JsonHelper::handle(json, "RemoveFile", [&](const auto& removeFile) {
        removeFiles.emplace_back(removeFile, component);
        });
    JsonHelper::handle(json, "ServiceControl",
        [&](const auto& serviceControl) {
        serviceControls.emplace_back(serviceControl, component);
        });
    JsonHelper::handle(json, "ServiceInstall",
        [&](const auto& serviceInstall) {
        serviceInstalls.emplace_back(serviceInstall, component,
            installerStrings);
        });
    JsonHelper::handle(json, "Shortcut", [&](const auto& shortcut) {
        shortcuts.emplace_back(shortcut, component, installerStrings);
        });

    componentId = GuidHelper::generate_guid();
    if (!files.empty()) {
        keyPath = files.front().getFileId();
    }
}

MSIHANDLE Component::getRecord() const {
    return MsiHelper::MsiRecordSet({ component, componentId, directory,
        attributes, condition, keyPath });
}

FeatureComponents Component::getFeatureComponent() const {
    return { feature, component };
}

std::tuple<bool, CreateFolder> Component::getCreateFolder() const {
    if (create_folder) {
        return { true, { directory, component } };
    }
    return { false, {} };
}

std::vector<File> Component::getFiles() const {
    return files;
}

std::vector<Registry> Component::getRegistries() const {
    return registries;
}

std::vector<RemoveFile> Component::getRemoveFiles() const {
    return removeFiles;
}

std::vector<ServiceControl> Component::getServiceControls() const {
    return serviceControls;
}

std::vector<ServiceInstall> Component::getServiceInstalls() const {
    return serviceInstalls;
}

std::vector<Shortcut> Component::getShortcuts() const {
    return shortcuts;
}

std::string Component::getComponentName() const {
    return component;
}

std::string Component::getDirectory() const {
    return directory;
}

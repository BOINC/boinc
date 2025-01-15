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

#pragma once

#include <nlohmann/json.hpp>

#include "Record.h"
#include "FeatureComponents.h"
#include "CreateFolder.h"
#include "File.h"
#include "Registry.h"
#include "RemoveFile.h"
#include "ServiceControl.h"
#include "ServiceInstall.h"
#include "Shortcut.h"
#include "InstallerStrings.h"

class Component : public Record {
public:
    explicit Component(const nlohmann::json& json,
        const std::string& directory, const std::string& parent,
        InstallerStrings& installerStrings);
    ~Component() = default;
    MSIHANDLE getRecord() const override;
    FeatureComponents getFeatureComponent() const;
    std::tuple<bool, CreateFolder> getCreateFolder() const;
    std::vector<File> getFiles() const;
    std::vector<Registry> getRegistries() const;
    std::vector<RemoveFile> getRemoveFiles() const;
    std::vector<ServiceControl> getServiceControls() const;
    std::vector<ServiceInstall> getServiceInstalls() const;
    std::vector<Shortcut> getShortcuts() const;
    std::string getComponentName() const;
    std::string getDirectory() const;
private:
    std::string component{};
    std::string componentId{};
    std::string directory{};
    int attributes = MSI_NULL_INTEGER;
    std::string condition{};
    std::string keyPath{};
    std::string feature{};
    bool create_folder = false;
    std::vector<File> files{};
    std::vector<Registry> registries{};
    std::vector<RemoveFile> removeFiles{};
    std::vector<ServiceControl> serviceControls{};
    std::vector<ServiceInstall> serviceInstalls{};
    std::vector<Shortcut> shortcuts{};
};

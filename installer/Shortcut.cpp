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

#include "Shortcut.h"
#include "JsonHelper.h"
#include "MsiHelper.h"

Shortcut::Shortcut(const nlohmann::json& json, const std::string& component,
    InstallerStrings& installerStrings) : component(component) {
    JsonHelper::get(json, "Shortcut", shortcut);
    JsonHelper::get(json, "Directory_", directory);
    JsonHelper::get(json, "Name", name, installerStrings);
    JsonHelper::get(json, "Target", target);
    JsonHelper::get(json, "Arguments", arguments);
    JsonHelper::get(json, "Description", description, installerStrings);
    JsonHelper::get(json, "Hotkey", hotkey);
    JsonHelper::get(json, "Icon_", icon);
    JsonHelper::get(json, "IconIndex", iconIndex);
    JsonHelper::get(json, "ShowCmd", showCmd);
    JsonHelper::get(json, "WkDir", wkDir);
    JsonHelper::get(json, "DisplayResourceDll", displayResourceDll);
    JsonHelper::get(json, "DisplayResourceId", displayResourceId);
    JsonHelper::get(json, "DescriptionResourceDll", descriptionResourceDll);
    JsonHelper::get(json, "DescriptionResourceId", descriptionResourceId);
}

MSIHANDLE Shortcut::getRecord() const {
    return MsiHelper::MsiRecordSet({ shortcut, directory, name, component,
        target, arguments, description, hotkey, icon, iconIndex, showCmd,
        wkDir, displayResourceDll, displayResourceId, descriptionResourceDll,
        descriptionResourceId });
}

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

#include "ServiceInstall.h"
#include "JsonHelper.h"
#include "MsiHelper.h"

ServiceInstall::ServiceInstall(const nlohmann::json& json,
    const std::string& component, InstallerStrings& installerStrings) :
    component(component) {
    JsonHelper::get(json, "ServiceInstall", serviceInstall);
    JsonHelper::get(json, "Name", name);
    JsonHelper::get(json, "DisplayName", displayName, installerStrings);
    JsonHelper::get(json, "ServiceType", serviceType);
    JsonHelper::get(json, "StartType", startType);
    JsonHelper::get(json, "ErrorControl", errorControl);
    JsonHelper::get(json, "LoadOrderGroup", loadOrderGroup);
    JsonHelper::get(json, "Dependencies", dependencies);
    JsonHelper::get(json, "StartName", startName);
    JsonHelper::get(json, "Password", password);
    JsonHelper::get(json, "Arguments", arguments);
    JsonHelper::get(json, "Description", description, installerStrings);
}

MSIHANDLE ServiceInstall::getRecord() const {
    return MsiHelper::MsiRecordSet({ serviceInstall, name, displayName,
        serviceType, startType, errorControl, loadOrderGroup, dependencies,
        startName, password, arguments, component, description });
}

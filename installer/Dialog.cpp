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

#include <sstream>

#include "Dialog.h"
#include "Control.h"
#include "MsiHelper.h"
#include "JsonHelper.h"

Dialog::Dialog(const nlohmann::json& json,
    InstallerStrings& installerStrings) {
    JsonHelper::get(json, "Dialog", dialog);
    JsonHelper::get(json, "HCentering", hcentering);
    JsonHelper::get(json, "VCentering", vcentering);
    JsonHelper::get(json, "Width", width);
    JsonHelper::get(json, "Height", height);
    JsonHelper::get(json, "Attributes", attributes);
    JsonHelper::get(json, "Title", title, installerStrings);
    JsonHelper::get(json, "Control_First", first);
    JsonHelper::get(json, "Control_Default", default);
    JsonHelper::get(json, "Control_Cancel", cancel);
    JsonHelper::handle(json, "Controls", [&](const auto& control) {
        controls.emplace_back(control, installerStrings, dialog);
        });
}

std::vector<Control> Dialog::get_controls() const {
    return controls;
}

MSIHANDLE Dialog::getRecord() const {
    return MsiHelper::MsiRecordSet({ dialog, hcentering, vcentering, width,
        height, attributes, title, first, default, cancel });
}

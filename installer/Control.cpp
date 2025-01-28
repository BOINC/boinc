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

#include "Control.h"
#include "MsiHelper.h"
#include "JsonHelper.h"

Control::Control(const nlohmann::json& json,
    InstallerStrings& installerStrings, const std::string& dialog) :
    dialog(dialog) {
    JsonHelper::get(json, "Control", control);
    JsonHelper::get(json, "Type", type);
    JsonHelper::get(json, "X", x);
    JsonHelper::get(json, "Y", y);
    JsonHelper::get(json, "Width", width);
    JsonHelper::get(json, "Height", height);
    JsonHelper::get(json, "Attributes", attributes);
    JsonHelper::get(json, "Property", property);
    JsonHelper::get(json, "Text", text, installerStrings);
    JsonHelper::get(json, "Control_Next", next);
    JsonHelper::get(json, "Help", help, installerStrings);
    JsonHelper::get(json, "Binary_", text);
    JsonHelper::handle(json, "Conditions", [&](const auto& condition) {
        conditions.emplace_back(condition, dialog, control);
        });
    JsonHelper::handle(json, "Events", [&](const auto& event) {
        events.emplace_back(event, dialog, control);
        });
    JsonHelper::handle(json, "EventMappings", [&](const auto& eventMapping) {
        eventMappings.emplace_back(eventMapping, dialog, control);
        });
    JsonHelper::handle(json, "RadioButtons", [&](const auto& radioButton) {
        radioButtons.emplace_back(radioButton, property, installerStrings);
        });
}

const std::vector<ControlCondition>& Control::get_conditions() const noexcept {
    return conditions;
}

const std::vector<ControlEvent>& Control::get_events() const noexcept {
    return events;
}

const std::vector<EventMapping>& Control::get_event_mappings() const noexcept {
    return eventMappings;
}

const std::vector<RadioButton>& Control::get_radio_buttons() const noexcept {
    return radioButtons;
}

MSIHANDLE Control::getRecord() const {
    return MsiHelper::MsiRecordSet({ dialog, control, type, x, y, width,
        height, attributes, property, text, next, help });
}

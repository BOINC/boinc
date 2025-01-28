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

#include <sstream>
#include <vector>
#include <nlohmann/json.hpp>


#include "ControlCondition.h"
#include "ControlEvent.h"
#include "EventMapping.h"
#include "RadioButton.h"
#include "InstallerStrings.h"

class Control : public Record {
public:
    explicit Control(const nlohmann::json& json,
        InstallerStrings& installerStrings, const std::string& dialog);
    ~Control() = default;
    const std::vector<ControlCondition>& get_conditions() const noexcept;
    const std::vector<ControlEvent>& get_events() const noexcept;
    const std::vector<EventMapping>& get_event_mappings() const noexcept;
    const std::vector<RadioButton>& get_radio_buttons() const noexcept;
    MSIHANDLE getRecord() const override;
private:
    std::string dialog{};
    std::string control{};
    std::string type{};
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    int attributes = MSI_NULL_INTEGER;
    std::string property{};
    std::string text{};
    std::string next{};
    std::string help{};
    std::vector<ControlCondition> conditions{};
    std::vector<ControlEvent> events{};
    std::vector<EventMapping> eventMappings{};
    std::vector<RadioButton> radioButtons{};
};

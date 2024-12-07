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

#include <vector>
#include <nlohmann/json.hpp>

#include "Record.h"
#include "Control.h"
#include "InstallerStrings.h"

class Dialog : public Record {
public:
    explicit Dialog(const nlohmann::json& json,
        InstallerStrings& installerStrings);
    ~Dialog() = default;
    std::vector<Control> get_controls() const;
    MSIHANDLE getRecord() const override;
private:
    std::string dialog{};
    int hcentering = 0;
    int vcentering = 0;
    int width = 0;
    int height = 0;
    int attributes = MSI_NULL_INTEGER;
    std::string title{};
    std::string first{};
    std::string default{};
    std::string cancel{};
    std::vector<Control> controls{};
};

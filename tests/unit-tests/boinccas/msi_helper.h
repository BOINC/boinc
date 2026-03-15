// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2026 University of California
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

class MsiHelper {
public:
    MsiHelper();
    ~MsiHelper();
    void insertProperties(
        const std::vector<std::pair<std::string, std::string>>& properties);
    std::tuple<unsigned int, std::string> getProperty(MSIHANDLE hMsiHandle,
        const std::string& propertyName);
    void setProperty(MSIHANDLE hMsiHandle, const std::string& propertyName,
        const std::string& propertyValue);

    std::string getMsiHandle() const {
        return "#" + std::to_string(hMsi);
    }

private:
    void init();
    void cleanup();
    void fillSummaryInformationTable();
    void createPropertiesTable();
    void createTable(const std::string_view& sql_create);
    MSIHANDLE hMsi = 0;
    INSTALLUILEVEL originalUiLevel;
};

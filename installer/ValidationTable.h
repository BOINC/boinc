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

#pragma once

#include "Generator.h"
#include "Validation.h"

const auto ValidationCategoryIdentifier = std::string("Identifier");
const auto ValidationCategoryText = std::string("Text");
const auto ValidationCategoryTemplate = std::string("Template");
const auto ValidationCategoryCondition = std::string("Condition");
const auto ValidationCategoryBinary = std::string("Binary");
const auto ValidationCategoryFormatted = std::string("Formatted");
const auto ValidationCategoryGuid = std::string("Guid");
const auto ValidationCategoryCustomSource = std::string("CustomSource");
const auto ValidationCategoryDefaultDir = std::string("DefaultDir");
const auto ValidationCategoryUpperCase = std::string("UpperCase");
const auto ValidationCategoryFilename = std::string("Filename");
const auto ValidationCategoryVersion = std::string("Version");
const auto ValidationCategoryLanguage = std::string("Language");
const auto ValidationCategoryCabinet = std::string("Cabinet");
const auto ValidationCategoryProperty = std::string("Property");
const auto ValidationCategoryRegPath = std::string("RegPath");
const auto ValidationCategoryWildCardFilename =
std::string("WildCardFilename");
const auto ValidationCategoryShortcut = std::string("Shortcut");

const auto DescriptionWithUrl = [](const std::string& description,
    const std::string& url) {
        return description + " For more information, see " + url;
    };

class ValidationTable : public Generator<Validation> {
public:
    explicit ValidationTable();
    ~ValidationTable() = default;
    bool generate(MSIHANDLE hDatabase) override;
    void add(const Validation& value);
private:
    std::vector<Validation> values{};
};

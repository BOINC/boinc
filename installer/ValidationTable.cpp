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

#include "ValidationTable.h"

ValidationTable::ValidationTable() {
    const auto tableName = std::string("_Validation");
    const auto url = "https://learn.microsoft.com/en-us/windows/win32/msi/-validation-table";
    values.emplace_back(
        tableName,
        "Table",
        false,
        MSI_NULL_INTEGER,
        MSI_NULL_INTEGER,
        "",
        MSI_NULL_INTEGER,
        ValidationCategoryIdentifier,
        "",
        DescriptionWithUrl("Used to identify a particular table.", url)
    );
    values.emplace_back(
        tableName,
        "Column",
        false,
        MSI_NULL_INTEGER,
        MSI_NULL_INTEGER,
        "",
        MSI_NULL_INTEGER,
        ValidationCategoryIdentifier,
        "",
        DescriptionWithUrl("Used to identify a particular column of the table",
            url)
    );
    values.emplace_back(
        tableName,
        "Nullable",
        false,
        MSI_NULL_INTEGER,
        MSI_NULL_INTEGER,
        "",
        MSI_NULL_INTEGER,
        "",
        "Y;N",
        DescriptionWithUrl("Identifies whether the column may contain a Null "
            "value.", url)
    );
    values.emplace_back(tableName,
        "MinValue",
        true,
        -2147483647,
        2147483647,
        "",
        MSI_NULL_INTEGER,
        "",
        "",
        DescriptionWithUrl("The field contains the minimum permissible value.",
            url)
    );
    values.emplace_back(tableName,
        "MaxValue",
        true,
        -2147483647,
        2147483647,
        "",
        MSI_NULL_INTEGER,
        "",
        "",
        DescriptionWithUrl("The field is the maximum permissible value.", url)
    );
    values.emplace_back(tableName,
        "KeyTable",
        true,
        MSI_NULL_INTEGER,
        MSI_NULL_INTEGER,
        "",
        MSI_NULL_INTEGER,
        ValidationCategoryIdentifier,
        "",
        DescriptionWithUrl("The table name with the foreign keys", url)
    );
    values.emplace_back(tableName,
        "KeyColumn",
        true,
        1,
        32,
        "",
        MSI_NULL_INTEGER,
        "",
        "",
        DescriptionWithUrl("This field applies to table columns that are "
            "external keys", url)
    );
    values.emplace_back(tableName,
        "Category",
        true,
        MSI_NULL_INTEGER,
        MSI_NULL_INTEGER,
        "",
        MSI_NULL_INTEGER,
        "",
        "Text;Formatted;Template;Condition;Guid;Path;Version;Language;"
        "Identifier;Binary;UpperCase;LowerCase;Filename;Paths;AnyPath;"
        "WildCardFilename;RegPath;KeyFormatted;CustomSource;Property;Cabinet;"
        "Shortcut;URL",
        DescriptionWithUrl("This is the type of data contained by the "
            "database field specified by the Table and Column columns of the "
            "_Validation table.", url)
    );
    values.emplace_back(tableName,
        "Set",
        true,
        MSI_NULL_INTEGER,
        MSI_NULL_INTEGER,
        "",
        MSI_NULL_INTEGER,
        ValidationCategoryText,
        "",
        DescriptionWithUrl("This is a list of permissible values for this "
            "field separated by semicolons.", url)
    );
    values.emplace_back(tableName,
        "Description",
        true,
        MSI_NULL_INTEGER,
        MSI_NULL_INTEGER,
        "",
        MSI_NULL_INTEGER,
        ValidationCategoryText,
        "",
        DescriptionWithUrl("A description of the data that is stored in the "
            "column.", url)
    );
}

bool ValidationTable::generate(MSIHANDLE hDatabase) {
    std::cout << "Generating ValidationTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `_Validation` "
        "(`Table` CHAR(32) NOT NULL, `Column` CHAR(32) NOT NULL, "
        "`Nullable` CHAR(4) NOT NULL, `MinValue` LONG, `MaxValue` LONG, "
        "`KeyTable` CHAR(255), `KeyColumn` SHORT, `Category` CHAR(32), "
        "`Set` CHAR(255), `Description` CHAR(255) "
        "PRIMARY KEY `Table`, `Column`)";
    const auto sql_insert = "INSERT INTO `_Validation` "
        "(`Table`, `Column`, `Nullable`, `MinValue`, `MaxValue`, `KeyTable`, "
        "`KeyColumn`, `Category`, `Set`, `Description`) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, values);
}

void ValidationTable::add(const Validation& value) {
    values.emplace_back(value);
}

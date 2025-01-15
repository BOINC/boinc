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

#include <sstream>
#include <iostream>

#include "FileTable.h"
#include "CabHelper.h"
#include "StreamTable.h"
#include "MediaTable.h"
#include "MsiFileHashTable.h"

int FileTable::GetFileLanguage(const std::string& filePath) {
    DWORD handle;
    const auto size = GetFileVersionInfoSize(filePath.c_str(), &handle);
    if (size == 0) {
        return 0;
    }

    std::vector<BYTE> buffer(size);
    if (!GetFileVersionInfo(filePath.c_str(), handle, size, buffer.data())) {
        return 0;
    }

    struct LANGANDCODEPAGE {
        WORD wLanguage;
        WORD wCodePage;
    } *lpTranslate;

    UINT cbTranslate;
    if (!VerQueryValue(buffer.data(), TEXT("\\VarFileInfo\\Translation"),
        reinterpret_cast<LPVOID*>(&lpTranslate), &cbTranslate)) {
        return 0;
    }

    if (cbTranslate < sizeof(LANGANDCODEPAGE)) {
        return 0;
    }

    return lpTranslate->wLanguage;
}

std::string FileTable::GetFileVersion(const std::string& filePath) {
    DWORD handle;
    const auto size = GetFileVersionInfoSize(filePath.c_str(), &handle);
    if (size == 0) {
        return {};
    }

    std::vector<BYTE> buffer(size);
    if (!GetFileVersionInfo(filePath.c_str(), handle, size, buffer.data())) {
        return {};
    }

    VS_FIXEDFILEINFO* fileInfo = nullptr;
    UINT fileInfoSize = 0;
    if (!VerQueryValue(buffer.data(), TEXT("\\"),
        reinterpret_cast<LPVOID*>(&fileInfo), &fileInfoSize) ||
        fileInfoSize == 0) {
        return {};
    }

    const DWORD majorVersion = HIWORD(fileInfo->dwFileVersionMS);
    const DWORD minorVersion = LOWORD(fileInfo->dwFileVersionMS);
    const DWORD buildNumber = HIWORD(fileInfo->dwFileVersionLS);
    const DWORD revisionNumber = LOWORD(fileInfo->dwFileVersionLS);

    std::stringstream ss;
    ss << majorVersion << "." << minorVersion << "." << buildNumber << "."
        << revisionNumber;
    return ss.str();
}

size_t FileTable::GetFileSize(const std::string& filePath) {
    const auto hFile = CreateFile(filePath.c_str(), GENERIC_READ,
        FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        return 0;
    }

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) {
        CloseHandle(hFile);
        return 0;
    }

    CloseHandle(hFile);
    return fileSize.QuadPart;
}

std::filesystem::path FileTable::GetAbsolutePath(
    const std::filesystem::path& filePath) {
    const std::string configuration_template = "%%CONFIGURATION%%";
    const std::string platform_template = "%%PLATFORM%%";

    auto p = filePath.string();
    auto index = p.find(configuration_template);
    if (index != std::string::npos) {
        p.replace(index, configuration_template.size(), configuration);
    }
    index = p.find(platform_template);
    if (index != std::string::npos) {
        p.replace(index, platform_template.size(), platform);
    }
    return root_path / p;
}

std::string FileTable::GetFileName(const std::filesystem::path& filePath) {
    char shortName[MAX_PATH];
    auto fileName = filePath.filename().string();
    if (GetShortPathName(filePath.string().c_str(), shortName, MAX_PATH)
        != 0) {
        const auto converted =
            std::filesystem::path(shortName).filename().string();
        if (converted != fileName) {
            return converted + "|" + fileName;
        }
    }
    else {
        std::cerr << "Failed to get short path name for " << filePath <<
            ": " << GetLastError() << std::endl;
    }
    return fileName;
}

FileTable::FileTable(const std::vector<Directory>& directories,
    const std::filesystem::path& root_path,
    const std::filesystem::path& output_path, const std::string& platform,
    const std::string& configuration,
    std::shared_ptr<ValidationTable> validationTable) : root_path(root_path),
    output_path(output_path), platform(platform),
    configuration(configuration), validationTable(validationTable) {
    int sequence = 0;
    for (const auto& directory : directories) {
        for (const auto& component : directory.getComponents()) {
            for (auto file : component.getFiles()) {
                file.setFilepath(GetAbsolutePath(file.getFilepath()));
                file.setAttributes(16384);
                file.setSequence(++sequence);
                const auto language = GetFileLanguage(
                    file.getFilepath().string());
                if (language > 0) {
                    file.setLanguage(std::to_string(language));
                }
                const auto version = GetFileVersion(
                    file.getFilepath().string());
                if (!version.empty()) {
                    file.setVersion(version);
                }
                file.setFilesize(static_cast<int>(
                    GetFileSize(file.getFilepath().string())));
                file.setFileName(GetFileName(file.getFilepath()));
                files.push_back(file);
            }
        }
    }

    const auto tableName = std::string("File");
    const auto url = std::string("https://learn.microsoft.com/en-us/windows/win32/msi/file-table");
    if (validationTable != nullptr) {
        validationTable->add(Validation(
            tableName,
            "File",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("A non-localized token that uniquely "
                "identifies the file.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Component_",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "Component",
            1,
            ValidationCategoryIdentifier,
            "",
            DescriptionWithUrl("The external key into the first column of the "
                "Component Table.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "FileName",
            false,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryFilename,
            "",
            DescriptionWithUrl("The file name used for installation.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "FileSize",
            false,
            0,
            2147483647,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("The size of the file in bytes.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Version",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "File",
            1,
            ValidationCategoryVersion,
            "",
            DescriptionWithUrl("This field is the version string for a "
                "versioned file.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Language",
            true,
            MSI_NULL_INTEGER,
            MSI_NULL_INTEGER,
            "",
            MSI_NULL_INTEGER,
            ValidationCategoryLanguage,
            "",
            DescriptionWithUrl("A list of decimal language IDs separated by "
                "commas.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Attributes",
            true,
            0,
            32767,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("The integer that contains bit flags that "
                "represent file attributes.", url)
        ));
        validationTable->add(Validation(
            tableName,
            "Sequence",
            false,
            1,
            32767,
            "",
            MSI_NULL_INTEGER,
            "",
            "",
            DescriptionWithUrl("Sequence position of this file on the media "
                "images.", url)
        ));
    }
}

bool FileTable::generate(MSIHANDLE hDatabase) {
    const auto cabname = std::string("boinc.cab");
    if (!CabHelper::create(output_path, cabname, files)) {
        return false;
    }

    if (!StreamTable(
        { Stream(cabname, output_path / cabname) }).generate(hDatabase)) {
        std::cerr << "Failed to generate StreamTable" << std::endl;
        return false;
    }

    std::filesystem::remove(output_path / cabname);

    if (!MediaTable({ Media(1, static_cast<int>(files.size()), "1",
        "#" + cabname, "DISK1", "") }, validationTable).generate(hDatabase)) {
        std::cerr << "Failed to generate MediaTable" << std::endl;
        return false;
    }
    if (!MsiFileHashTable(files, validationTable).generate(hDatabase)) {
        std::cerr << "Failed to generate MsiFileHashTable" << std::endl;
        return false;
    }

    std::cout << "Generating FileTable..." << std::endl;

    const auto sql_create = "CREATE TABLE `File` (`File` CHAR(72) NOT NULL, "
        "`Component_` CHAR(72) NOT NULL, "
        "`FileName` CHAR(255) NOT NULL LOCALIZABLE, `FileSize` LONG NOT NULL, "
        "`Version` CHAR(72), `Language` CHAR(20), `Attributes` SHORT, "
        "`Sequence` SHORT NOT NULL PRIMARY KEY `File`)";
    const auto sql_insert = "INSERT INTO `File` (`File`, `Component_`, "
        "`FileName`, `FileSize`, `Version`, `Language`, `Attributes`, "
        "`Sequence`) VALUES (?, ?, ?, ?, ?, ?, ?, ?)";

    return Generator::generate(hDatabase, sql_create, sql_insert, files);
}

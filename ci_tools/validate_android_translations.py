#!/usr/bin/env python3

# This file is part of BOINC.
# https://boinc.berkeley.edu
# Copyright (C) 2026 University of California
#
# BOINC is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
#
# BOINC is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

import pathlib
import sys
import xml.etree.ElementTree as ET


def parse_string_resources(xml_file: pathlib.Path) -> dict[str, str]:
    root = ET.parse(xml_file).getroot()
    strings: dict[str, str] = {}
    for child in root:
        if child.tag == "string" and "name" in child.attrib:
            strings[child.attrib["name"]] = "".join(child.itertext())
    return strings


def is_string_xml_in_values_folder(file: pathlib.Path, language: str | None = None) -> bool:
    folder = "values" if language is None else f"values-{language}"
    return file.parent.name == folder and file.name == "strings.xml"


def is_string_xml(file: pathlib.Path) -> bool:
    return file.name == "strings.xml" and not is_string_xml_in_values_folder(file)


def compare_two_strings_translations_maps(map1: dict[str, str], map2: dict[str, str]) -> bool:
    missed_keys = [k for k in map1 if k not in map2]
    different_values = [k for k in map1 if k in map2 and map1[k] != map2[k]]
    if missed_keys:
        print("The following keys are missed:")
        for k in missed_keys:
            print(k)
    if different_values:
        print("The following values are different:")
        for k in different_values:
            print(k)
    return not missed_keys and not different_values


def do_binary_validation(main_file: pathlib.Path, dependent_file: pathlib.Path) -> bool:
    if main_file.read_bytes() != dependent_file.read_bytes():
        dependent_file_path = dependent_file.parent
        print(f"Binary validation for {dependent_file} failed!")
        print(f"Consider copying {main_file} to {dependent_file_path}")
        print("or running next command:")
        print(f"cp {main_file} {dependent_file_path}")
        return False
    return True


def main() -> None:
    app_dir = pathlib.Path("android/BOINC/app")
    if not app_dir.is_dir():
        app_dir = pathlib.Path(__file__).resolve().parent.parent / "android/BOINC/app"
    res_dir = app_dir / "src/main/res"
    java_dir = app_dir / "src/main/java"
    manifest_file = app_dir / "src/main/AndroidManifest.xml"

    res_files = sorted(p for p in res_dir.rglob("*") if p.is_file())
    main_strings_file = next((p for p in res_files if is_string_xml_in_values_folder(p)), None)
    if main_strings_file is None:
        sys.exit("No strings.xml file found in values folder")

    translation_strings_files = [p for p in res_files if is_string_xml(p)]
    if not translation_strings_files:
        sys.exit("No translation strings files found")

    main_strings_map = parse_string_resources(main_strings_file)
    main_strings_usage = {k: False for k in main_strings_map}

    for source in res_files:
        if source.suffix != ".xml" or is_string_xml(source) or is_string_xml_in_values_folder(source):
            continue
        content = source.read_text(encoding="utf-8")
        for key, used in main_strings_usage.items():
            if not used and key in content:
                main_strings_usage[key] = True

    for source in sorted(java_dir.rglob("*.kt")):
        content = source.read_text(encoding="utf-8")
        for key, used in main_strings_usage.items():
            if not used and f"R.string.{key}" in content:
                main_strings_usage[key] = True

    manifest_content = manifest_file.read_text(encoding="utf-8")
    for key, used in main_strings_usage.items():
        if not used and key in manifest_content:
            main_strings_usage[key] = True

    unused_keys = [k for k, used in main_strings_usage.items() if not used]
    for k in unused_keys:
        print(f"{k} is not used in main strings file")
    if unused_keys:
        sys.exit("Redundant keys found in main strings file")

    parsed_translations = {file: parse_string_resources(file) for file in translation_strings_files}

    translation_files_contain_redundant_keys = False
    for file, strings_map in parsed_translations.items():
        redundant_keys = [k for k in strings_map if k not in main_strings_usage]
        if redundant_keys:
            print(f"{file.resolve()}: the following keys are not present in the main strings file: {redundant_keys}")
            translation_files_contain_redundant_keys = True
    if translation_files_contain_redundant_keys:
        sys.exit("The translation files contain redundant keys")

    en_file = next((p for p in translation_strings_files if is_string_xml_in_values_folder(p, "en")), None)
    if en_file is None:
        sys.exit("No strings.xml file found in values-en folder")
    if not compare_two_strings_translations_maps(main_strings_map, parsed_translations[en_file]):
        sys.exit("The main strings and en translations are not equal")

    for language_name, main_code, alias_code, required in [
        ("Hebrew", "he", "iw", True),
        ("Indonesian", "in", "id", True),
        ("Yiddish", "yi", "ji", False),
    ]:
        main_lang_file = next(
            (p for p in translation_strings_files if is_string_xml_in_values_folder(p, main_code)), None
        )
        alias_lang_file = next(
            (p for p in translation_strings_files if is_string_xml_in_values_folder(p, alias_code)), None
        )
        if not required and main_lang_file is None and alias_lang_file is None:
            continue
        if main_lang_file is None:
            sys.exit(f"No strings.xml file found in values-{main_code} folder")
        if alias_lang_file is None:
            sys.exit(f"No strings.xml file found in values-{alias_code} folder")
        if not compare_two_strings_translations_maps(
            parsed_translations[main_lang_file], parsed_translations[alias_lang_file]
        ):
            sys.exit(f"The {language_name} strings ({main_code} and {alias_code}) are not equal")
        if not do_binary_validation(main_lang_file, alias_lang_file):
            sys.exit(
                f"The {language_name} strings ({main_code} and {alias_code}) contain equal data but files are different"
            )


if __name__ == "__main__":
    main()

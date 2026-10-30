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
        if is_string_xml(source) or is_string_xml_in_values_folder(source):
            continue
        for line in source.read_text(encoding="utf-8", errors="replace").splitlines():
            for key, used in main_strings_usage.items():
                if not used and key in line:
                    main_strings_usage[key] = True

    for source in sorted(java_dir.rglob("*.kt")):
        for line in source.read_text(encoding="utf-8").splitlines():
            for key, used in main_strings_usage.items():
                if not used and f"R.string.{key}" in line:
                    main_strings_usage[key] = True

    for line in manifest_file.read_text(encoding="utf-8").splitlines():
        for key, used in main_strings_usage.items():
            if not used and key in line:
                main_strings_usage[key] = True

    unused_keys = [k for k, used in main_strings_usage.items() if not used]
    for k in unused_keys:
        print(f"{k} is not used in main strings file")
    if unused_keys:
        sys.exit("Redundant keys found in main strings file")

    translation_files_contain_redundant_keys = False
    for file in translation_strings_files:
        redundant_keys = [k for k in parse_string_resources(file) if k not in main_strings_usage]
        if redundant_keys:
            print(f"{file.resolve()}: the following keys are not present in the main strings file: {redundant_keys}")
            translation_files_contain_redundant_keys = True
    if translation_files_contain_redundant_keys:
        sys.exit("The translation files contain redundant keys")

    en_file = next((p for p in translation_strings_files if is_string_xml_in_values_folder(p, "en")), None)
    if en_file is None:
        sys.exit("No strings.xml file found in values-en folder")
    if not compare_two_strings_translations_maps(main_strings_map, parse_string_resources(en_file)):
        sys.exit("The main strings and en translations are not equal")

    he_file = next((p for p in translation_strings_files if is_string_xml_in_values_folder(p, "he")), None)
    if he_file is None:
        sys.exit("No strings.xml file found in values-he folder")
    iw_file = next((p for p in translation_strings_files if is_string_xml_in_values_folder(p, "iw")), None)
    if iw_file is None:
        sys.exit("No strings.xml file found in values-iw folder")
    if not compare_two_strings_translations_maps(parse_string_resources(he_file), parse_string_resources(iw_file)):
        sys.exit("The Hebrew strings (he and iw) are not equal")
    if not do_binary_validation(he_file, iw_file):
        sys.exit("The Hebrew strings (he and iw) contain equal data but files are different")

    in_file = next((p for p in translation_strings_files if is_string_xml_in_values_folder(p, "in")), None)
    if in_file is None:
        sys.exit("No strings.xml file found in values-in folder")
    id_file = next((p for p in translation_strings_files if is_string_xml_in_values_folder(p, "id")), None)
    if id_file is None:
        sys.exit("No strings.xml file found in values-id folder")
    if not compare_two_strings_translations_maps(parse_string_resources(in_file), parse_string_resources(id_file)):
        sys.exit("The Indonesian strings (in and id) are not equal")
    if not do_binary_validation(in_file, id_file):
        sys.exit("The Indonesian strings (in and id) contain equal data but files are different")

    yi_file = next((p for p in translation_strings_files if is_string_xml_in_values_folder(p, "yi")), None)
    ji_file = next((p for p in translation_strings_files if is_string_xml_in_values_folder(p, "ji")), None)
    if yi_file is not None or ji_file is not None:
        if yi_file is None:
            sys.exit("No strings.xml file found in values-yi folder")
        if ji_file is None:
            sys.exit("No strings.xml file found in values-ji folder")
        if not compare_two_strings_translations_maps(parse_string_resources(yi_file), parse_string_resources(ji_file)):
            sys.exit("The Yiddish strings (yi and ji) are not equal")
        if not do_binary_validation(yi_file, ji_file):
            sys.exit("The Yiddish strings (yi and ji) contain equal data but files are different")


if __name__ == "__main__":
    main()

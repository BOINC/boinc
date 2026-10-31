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

"""Run with: python3 -m unittest discover -s ci_tools -p 'test_*.py'."""

from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


SCRIPT = Path(__file__).with_name("validate_android_translations.py")


class TranslationValidationTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.app = self.root / "android/BOINC/app/src/main"
        self.res = self.app / "res"
        for locale in ("values", "values-en", "values-he", "values-iw", "values-in", "values-id"):
            self.write(locale)
        (self.res / "layout").mkdir()
        (self.res / "layout/main.xml").write_text('<TextView text="@string/hello"/>', encoding="utf-8")
        (self.app / "java").mkdir()
        (self.app / "AndroidManifest.xml").write_text('<manifest/>', encoding="utf-8")

    def write(self, locale, content='<resources><string name="hello">Hello</string></resources>'):
        directory = self.res / locale
        directory.mkdir(parents=True, exist_ok=True)
        (directory / "strings.xml").write_text(content, encoding="utf-8")

    def validate(self, expected_error=None):
        result = subprocess.run([sys.executable, "-B", str(SCRIPT)], cwd=self.root,
                                text=True, capture_output=True, timeout=15)
        if expected_error is None:
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        else:
            self.assertNotEqual(result.returncode, 0)
            self.assertIn(expected_error, result.stdout + result.stderr)

    def test_valid_resources_and_binary_assets(self):
        (self.res / "drawable").mkdir()
        (self.res / "drawable/image.png").write_bytes(b"\x89PNG\xff\x00")
        self.validate()

    def test_missing_default_or_required_locale(self):
        for locale in ("values", "values-en", "values-he", "values-iw", "values-in", "values-id"):
            with self.subTest(locale=locale):
                path = self.res / locale / "strings.xml"
                data = path.read_bytes()
                path.unlink()
                self.validate("No strings.xml file found")
                path.write_bytes(data)

    def test_unused_default_key(self):
        self.write("values", '<resources><string name="hello">Hello</string><string name="unused">Unused</string></resources>')
        self.validate("Redundant keys found in main strings file")

    def test_redundant_translation_key(self):
        self.write("values-fr", '<resources><string name="unknown">Inconnu</string></resources>')
        self.validate("The translation files contain redundant keys")

    def test_missing_or_different_english_string(self):
        for content in ('<resources/>', '<resources><string name="hello">Different</string></resources>'):
            with self.subTest(content=content):
                self.write("values-en", content)
                self.validate("The main strings and en translations are not equal")

    def test_optional_yiddish_pair_must_be_complete_and_equal(self):
        self.write("values-yi")
        self.validate("No strings.xml file found in values-ji folder")
        self.write("values-ji")
        self.validate()
        self.write("values-ji", '<resources><string name="hello">Different</string></resources>')
        self.validate("The Yiddish strings (yi and ji) are not equal")

    def test_alias_files_must_be_byte_identical(self):
        for locale in ("values-iw", "values-id"):
            with self.subTest(locale=locale):
                path = self.res / locale / "strings.xml"
                data = path.read_bytes()
                path.write_bytes(data + b"\n")
                self.validate("contain equal data but files are different")
                path.write_bytes(data)

    def test_inline_markup_is_compared_as_text(self):
        content = '<resources><string name="hello">Hello <b>world</b>!</string></resources>'
        self.write("values", content)
        self.write("values-en", '<resources><string name="hello">Hello world!</string></resources>')
        self.validate()

    def test_kotlin_and_manifest_references_count_as_usage(self):
        (self.res / "layout/main.xml").unlink()
        kotlin = self.app / "java/Main.kt"
        kotlin.write_text('val text = R.string.hello', encoding="utf-8")
        self.validate()
        kotlin.unlink()
        (self.app / "AndroidManifest.xml").write_text('<manifest label="@string/hello"/>', encoding="utf-8")
        self.validate()

    def test_malformed_xml_fails(self):
        self.write("values-fr", '<resources><string>')
        self.validate("ParseError")


if __name__ == "__main__":
    unittest.main()

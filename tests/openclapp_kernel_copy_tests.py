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

"""Exercise the real copy targets without requiring the C++ toolchain.

The callers' targets are extracted from the vcxproj files into small test
projects. This checks shared-target scheduling and file-copy behavior, not
native compilation or Visual Studio's up-to-date checks. The Windows workflow
also builds the complete solution with the real C++ targets.

Run with MSBuild on PATH, or use --msbuild dotnet msbuild.
"""

import argparse
import copy
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
import xml.etree.ElementTree as ET


NS = "http://schemas.microsoft.com/developer/msbuild/2003"
ET.register_namespace("", NS)
REPO = Path(__file__).resolve().parents[1]
KERNEL = "openclapp_kernels.cl"
TARGET = "CopyOpenCLAppKernels"
CALLERS = ("example_app_atiopencl", "example_app_nvopencl")
MARKER = "OPENCLAPP_KERNEL_COPIED"


def element(name, **attributes):
    return ET.Element(f"{{{NS}}}{name}", attributes)


def write_project(path, root):
    ET.ElementTree(root).write(path, encoding="utf-8", xml_declaration=True)


class KernelCopyTests(unittest.TestCase):
    msbuild = ["msbuild"]

    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="boinc-opencl-copy-")
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.build = self.root / "win_build"
        self.build.mkdir()
        self.source = self.root / "samples" / "openclapp" / KERNEL
        self.source.parent.mkdir(parents=True)
        shutil.copyfile(REPO / "samples" / "openclapp" / KERNEL, self.source)
        self.out = self.root / "output with spaces"
        self.destination = self.out / KERNEL

        shared = ET.parse(REPO / "win_build" / "openclapp_kernels.proj").getroot()
        target = shared.find(f"{{{NS}}}Target[@Name='{TARGET}']")
        self.assertIsNotNone(target, "Missing shared copy target")
        # Count successful executions without relying on localized Copy logs.
        target.append(element("Message", Text=MARKER, Importance="high"))
        write_project(self.build / "openclapp_kernels.proj", shared)

    def caller(self, name, template, out=None):
        original = ET.parse(REPO / "win_build" / f"{template}.vcxproj")
        target = original.find(f"{{{NS}}}Target[@Name='{TARGET}']")
        self.assertIsNotNone(target, f"Missing copy hook in {template}")
        self.assertEqual(target.get("AfterTargets"), "Build")
        root = element("Project", DefaultTargets="Build")
        properties = element("PropertyGroup")
        output = element("OutDir")
        output.text = str(out if out is not None else self.out) + os.sep
        properties.append(output)
        root.append(properties)
        root.append(element("Target", Name="Build"))
        root.append(copy.deepcopy(target))
        path = self.build / f"{name}.proj"
        write_project(path, root)
        return path

    def driver(self, callers):
        root = element("Project", DefaultTargets="Build")
        target = element("Target", Name="Build")
        target.append(element("MSBuild", Projects=";".join(map(str, callers)),
                              Targets="Build", BuildInParallel="true"))
        root.append(target)
        path = self.root / "parallel.proj"
        write_project(path, root)
        return path

    def run_msbuild(self, project, *options, success=True):
        env = dict(os.environ, DOTNET_CLI_UI_LANGUAGE="en-US", VSLANG="1033",
                   DOTNET_CLI_TELEMETRY_OPTOUT="1", MSBUILDDISABLENODEREUSE="1")
        result = subprocess.run(
            self.msbuild + [str(project), "-nologo", "-nr:false", "-m:4",
                            "-warnAsError", "-v:normal"] + list(options),
            cwd=self.root, env=env, stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT, text=True, errors="replace", timeout=120)
        if success:
            self.assertEqual(result.returncode, 0, result.stdout)
        else:
            self.assertNotEqual(result.returncode, 0, result.stdout)
        return result.stdout

    def assert_copied(self, output, count=1, destination=None):
        self.assertEqual(output.count(MARKER), count, output)
        path = destination if destination is not None else self.destination
        self.assertEqual(path.read_bytes(), self.source.read_bytes())

    def test_each_example_can_copy_independently(self):
        for template in CALLERS:
            with self.subTest(example=template):
                caller = self.caller(template, template)
                self.assert_copied(self.run_msbuild(caller))
                self.destination.unlink()

    def test_parallel_examples_copy_once(self):
        callers = [self.caller(name, name) for name in CALLERS]
        self.assert_copied(self.run_msbuild(self.driver(callers)))

    def test_solution_build_shares_child_project(self):
        original = ET.parse(REPO / "win_build" / "boinc.slnx")
        solution = ET.Element("Solution")
        solution.append(copy.deepcopy(original.find("Configurations")))
        for name in CALLERS:
            path = self.caller(name, name)
            native = path.with_suffix(".vcxproj")
            path.rename(native)
            entry = original.find(f"Project[@Path='{name}.vcxproj']")
            self.assertIsNotNone(entry)
            # Keep the real solution's native project IDs and configurations,
            # but omit library dependencies: these fixtures only test copying.
            attributes = dict(entry.attrib, Path=str(native.relative_to(self.root)))
            ET.SubElement(solution, "Project", attributes)
        path = self.root / "examples.slnx"
        ET.ElementTree(solution).write(path, encoding="utf-8")
        output = self.run_msbuild(path, "-p:Configuration=Debug", "-p:Platform=x64")
        self.assert_copied(output)

    def test_many_parallel_callers_copy_once_in_each_configuration(self):
        callers = [self.caller(f"caller-{i}", CALLERS[i % 2]) for i in range(32)]
        driver = self.driver(callers)
        for configuration in ("Debug", "Release"):
            for platform in ("x64", "ARM64"):
                with self.subTest(configuration=configuration, platform=platform):
                    output = self.run_msbuild(driver, f"-p:Configuration={configuration}",
                                              f"-p:Platform={platform}")
                    self.assert_copied(output)
                    self.destination.unlink()

    def test_different_output_directories_are_not_deduplicated(self):
        other = self.root / "another output"
        callers = [self.caller("ati", CALLERS[0]),
                   self.caller("nv", CALLERS[1], other)]
        output = self.run_msbuild(self.driver(callers))
        self.assert_copied(output, count=2)
        self.assert_copied(output, count=2, destination=other / KERNEL)

    def test_missing_source_fails_even_with_old_output(self):
        caller = self.caller("ati", CALLERS[0])
        self.source.unlink()
        for old_output in (False, True):
            with self.subTest(old_output=old_output):
                if old_output:
                    self.out.mkdir(exist_ok=True)
                    self.destination.write_bytes(b"old kernel")
                output = self.run_msbuild(caller, success=False)
                self.assertIn("MSB3030", output)

    def test_same_size_and_timestamp_do_not_hide_stale_output(self):
        caller = self.caller("ati", CALLERS[0])
        self.run_msbuild(caller)
        stat = self.destination.stat()
        stale = bytes(value ^ 1 for value in self.source.read_bytes())
        self.destination.write_bytes(stale)
        os.utime(self.destination, ns=(stat.st_atime_ns, stat.st_mtime_ns))
        self.assertEqual(self.destination.stat().st_size, self.source.stat().st_size)
        self.assertEqual(self.destination.stat().st_mtime_ns, self.source.stat().st_mtime_ns)
        self.assert_copied(self.run_msbuild(caller))

    def test_deleted_output_is_restored_on_next_build(self):
        caller = self.caller("nv", CALLERS[1])
        self.run_msbuild(caller)
        self.destination.unlink()
        self.assert_copied(self.run_msbuild(caller))

    def test_copy_failure_propagates_to_parent(self):
        caller = self.caller("ati", CALLERS[0])
        self.destination.mkdir(parents=True)
        output = self.run_msbuild(caller, success=False)
        self.assertIn("MSB3024", output)

    def test_output_directory_is_required(self):
        project = self.build / "openclapp_kernels.proj"
        output = self.run_msbuild(project, success=False)
        self.assertIn("OutDir must be set", output)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--msbuild", nargs="+", default=["msbuild"],
                        help="MSBuild command, e.g. --msbuild dotnet msbuild")
    KernelCopyTests.msbuild = parser.parse_args().msbuild
    unittest.main(argv=[__file__], verbosity=2)

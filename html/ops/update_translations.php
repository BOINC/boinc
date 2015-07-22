#!/usr/bin/env php
<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

$cli_only = true;
require_once("../inc/util_ops.inc");
require_once("../inc/translation.inc");

error_reporting(E_ALL);
ini_set('display_errors', true);
ini_set('display_startup_errors', true);

system("rm -f $lang_language_dir/$lang_compiled_dir/*");

$lang_log_level = 0;
if ($argc >= 3 && $argv[1] == '-d') {
    $lang_log_level = $argv[2];
}

// process the generic BOINC web site strings
//
build_translation_array_files(
    $lang_language_dir, $lang_translations_dir, $lang_compiled_dir
);

// process the project-specific strings
//
build_translation_array_files(
    $lang_language_dir, $lang_prj_translations_dir, $lang_compiled_dir
);

echo "update_translations finished\n";

?>

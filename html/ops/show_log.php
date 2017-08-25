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



// grep logs for a particular string

require_once("../inc/util_ops.inc");

$log_dir = parse_config(get_config(), "<log_dir>");
if (!$log_dir) {
    exit("Error: couldn't get log_dir from config file.");
}

if( isset($_GET["f"]) ){
    $f = $_GET["f"];
    $f = escapeshellcmd($f);
} else {
    $f = "";
}
if( isset($_GET["s"]) ){
    $s = $_GET["s"];
    $s = escapeshellcmd($s);
} else {
    $s = "";
}
if( isset($_GET["l"]) ){
    $l = (int)$_GET["l"];
} else {
    $l = 0;
}

if ($s) {
    admin_page_head("Grep logs for \"$s\"");
} else {
    admin_page_head("Show logs");
}

echo "<form action=\"show_log.php\">";
echo " Regexp: <input name=\"s\" value=\"$s\">";
echo " Files: <input name=\"f\" value=\"$f\">";
echo " Lines: <input name=\"l\" value=\"$l\"> (positive for head, negative for tail)";
echo " <input class=\"btn btn-default\" type=\"submit\" value=\"Grep\"></form>";

echo 'Hint: Example greps: "RESULT#106876", "26fe99aa_25636_00119.wu_1", "WU#8152", "too many errors", "2003-07-17", "CRITICAL" <br>';

if (strlen($f)) {
	$f = "../log*/". $f;
} else {
    $f = "../log*/*.log";
}

if ($s) {
    passthru("cd $log_dir && ../bin/grep_logs -html -l $l '$s' $f 2>&1");
}

admin_page_tail();
?>

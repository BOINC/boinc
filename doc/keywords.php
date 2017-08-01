<?php

// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2017 University of California
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

// show keywords as XML or as C or Python defines

require_once("../inc/xml.inc");
require_once("../inc/util_basic.inc");
require_once("keywords.inc");

function kw_xml($id, $kw) {
    echo "<keyword>
    <id>$id</id>
    <category>$kw->category</category>
    <level>$kw->level</level>
    <name>$kw->name</name>
    <symbol>$kw->symbol</symbol>
";
    if ($kw->level > 0) {
        echo "    <parent>$kw->parent</parent>\n";
    }
    echo "</keyword>\n";
}

function show_xml() {
    global $job_keywords;
    xml_header();
    echo "<keywords>\n";
    foreach ($job_keywords as $id=>$kw) {
        kw_xml($id, $kw);
    }
    echo "</keywords>\n";
}

function show_c() {
    global $job_keywords;
    header('Content-type:text/plain');
    foreach ($job_keywords as $id=>$kw) {
        echo "#define $kw->symbol $id\n";
    }
}

function show_python() {
    global $job_keywords;
    header('Content-type:text/plain');
    foreach ($job_keywords as $id=>$kw) {
        echo "$kw->symbol = $id\n";
    }
}

function show_bash() {
    global $job_keywords;
    header('Content-type:text/plain');
    foreach ($job_keywords as $id=>$kw) {
        echo "$kw->symbol='$id'\n";
    }
}

function show_php() {
    global $job_keywords;
    header('Content-type:text/plain');
    foreach ($job_keywords as $id=>$kw) {
        echo "define('$kw->symbol', $id);\n";
    }
}

$header = get_str('header', true);
if ($header == 'c') {
    show_c();
} else if ($header == 'python') {
    show_python();
} else if ($header == 'bash') {
    show_bash();
} else if ($header == 'php') {
    show_php();
} else {
    show_xml();
}

?>

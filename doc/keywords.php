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
require_once("../inc/util.inc");
require_once("../inc/keywords.inc");

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

function show_kw_html($id, $kw) {
    global $job_keywords;
    $x = "";
    for ($i=0; $i<$kw->level; $i++) {
        $x .= "&nbsp;&nbsp;&nbsp;&nbsp;";
    }
    row_array(array($x.$kw->name, $kw->symbol, $id));
    foreach ($job_keywords as $id2=>$kw2) {
        if ($kw2->parent == $id) {
            show_kw_html($id2, $kw2);
        }
    }
}

function show_category_html($category) {
    global $job_keywords;
    foreach ($job_keywords as $id=>$kw) {
        if ($kw->level) continue;
        if ($kw->category != $category) continue;
        show_kw_html($id, $kw);
    }
}

function show_html() {
    page_head("Keywords");
    start_table('table-striped');
    row_heading("Science Area");
    row_array(array("name", "symbol", "ID"));
    show_category_html(KW_CATEGORY_SCIENCE);
    row_heading("Location");
    row_array(array("name", "symbol", "ID"));
    show_category_html(KW_CATEGORY_LOC);
    end_table();
    page_tail();
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
} else if ($header == 'html') {
    show_html();
} else {
    show_xml();
}

?>

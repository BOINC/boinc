<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2011 University of California
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

// Utility functions for user file sandbox feature

require_once("../inc/util.inc");
require_once("../inc/dir_hier.inc");

// Return path of sandbox directory for the given user.
// Create dir if not present.
//
function sandbox_dir($user) {
    if (!is_dir("../../sandbox")) {
        mkdir("../../sandbox");
    }
    $d = "../../sandbox/$user->id";
    if (!is_dir($d)) {
        mkdir($d);
    }
    return $d;
}

function sandbox_write_link_file($path, $size, $md5) {
    file_put_contents($path, "sb $size $md5");
}

// parse a link file and return
// (error, size, md5)
//
function sandbox_parse_link_file($path) {
    $x = file_get_contents($path);
    $n = sscanf($x, "%s %d %s", $s, $size, $md5);
    if ($n != 3) return array(true, null, null);
    if ($s != 'sb') return array(true, null, null);
    return array(false, $size, $md5);
}

$fanout = parse_config(get_config(), "<uldl_dir_fanout>");

// return the path of the file in the download directory
//
function sandbox_physical_path($user, $md5) {
    global $fanout;
    $f = "sb_".$user->id."_".$md5;
    return dir_hier_path($f, "../../download", $fanout);
}

// return list of files in sandbox
//
function sandbox_file_names($user) {
    $d = opendir(sandbox_dir($user));
    $names = array();
    while (($f = readdir($d)) !== false) {
        if ($f == '.') continue;
        if ($f == '..') continue;
        $names[] = $f;
    }
    return $names;
}

// return a <select> for files in sandbox
//
function sandbox_file_select($user, $select_name) {
    $x = "<select name=$select_name>\n";
    $files = sandbox_file_names($user);
    foreach ($files as $f) {
        if(preg_match("/$select_name/",$f)){
            $x .= "<option value=\"$f\">$f</option>\n";
        }
    }
    $x .= "</select>\n";
    return $x;
}

?>

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
//
// In this system:
// - each user (job submitter) has a 'sandbox' where they can store files
//      on the BOINC server, via a web interface.
//      These files are mutable; you can modify a file w/ a given name.
// - files are stored in project/sandbox/<userid>/
// - When a file is uploaded, its size and MD5 are computed and stored
//      in an 'info file' in a parallel dir, project/sandbox/<userid>/.md5/
//
// Sandbox files can be used for web-based job submissions systems
// like BUDA and autodock on BOINC Central.
// Typically they are used as job input files or app files,
// in which case they are downloadable by BOINC clients.
// When a file is used in this way,
// it must be copied to the download hierarchy,
// and assigned a physical name that includes its MD5.
// The name can depend on the role of the file;
// e.g. it can include a batch ID or BUDA app name

require_once("../inc/util.inc");
require_once("../inc/dir_hier.inc");

// Return path of sandbox directory for the given user.
// Create dir if not present.
//
function sandbox_dir($user) {
    $dir = parse_config(get_config(), "<sandbox_dir>");
    if (!$dir) {
        $dir = "../../sandbox";
    }
    if (!is_dir($dir)) {
        mkdir($dir);
    }
    $d = "$dir/$user->id";
    if (!is_dir($d)) {
        mkdir($d);
    }
    if (!is_dir("$d/.md5")) {
        mkdir("$d/.md5");
    }
    return $d;
}

// return path of sandbox file
//
function sandbox_path($user, $fname) {
    $dir = sandbox_dir($user);
    return "$dir/$fname";
}

// parse a sandbox file's info file.
// If missing, create it.
//
function sandbox_parse_info_file($user, $name) {
    $dir = sandbox_dir($user);
    $info_path = "$dir/.md5/$name";
    $info = parse_info_file($info_path);
    if ($info) {
        return $info;
    }
    [$md5, $size] = get_file_info("$dir/$name");
    write_info_file($info_path, $md5, $size);
    return [$md5, $size];
}

// return list of files in sandbox
//
function sandbox_file_names($user) {
    $files = scandir(sandbox_dir($user));
    $names = array();
    foreach ($files as $f) {
        if ($f[0] == '.') continue;
        $names[] = $f;
    }
    natsort($names);
    return $names;
}

// return list of files matching given pattern,
// in the format used for form_select() and form_select_multiple()
//
function sandbox_select_items($user, $pattern=null) {
    $sbfiles = sandbox_file_names($user);
    $sbitems = [];
    foreach ($sbfiles as $f) {
        if ($pattern && !preg_match($pattern, $f)) continue;
        $sbitems[] = [$f, $f];
    }
    return $sbitems;
}

// return a <select> for files in sandbox
//
function sandbox_file_select(
    $user, $select_name, $regexp = null, $allow_none = false
) {
    $x = "<select class=\"form-control\" name=$select_name>\n";
    if ($allow_none) {
        $x .= "<option value=\"\">--- None</option>\n";
    }
    $files = sandbox_file_names($user);
    foreach ($files as $f) {
        if ($regexp && !preg_match("/$regexp/",$f)) continue;
        $x .= "<option value=\"$f\">$f</option>\n";
    }
    $x .= "</select>\n";
    return $x;
}

// Copy file and corresponding .md5 file from sandbox to $dir
// (which must have a subdir .md5/)
// Used for BUDA app files.
//
function copy_sandbox_file($user, $fname, $dir) {
    $sbdir = sandbox_dir($user);
    copy("$sbdir/$fname", "$dir/$fname");
    copy("$sbdir/.md5/$fname", "$dir/.md5/$fname");
}

//////////// tables with sortable columns ////////////////

// maybe this should go in util.inc

$g_field = null;

function field_compare($a, $b) {
    global $g_field;
    if ($a->$g_field == $b->$g_field) return 0;
    return $a->$g_field > $b->$g_field;
}

function column_sort(&$items, $field, $rev) {
    global $g_field;
    $g_field = $field;
    usort($items, 'field_compare');
    if ($rev) {
        $items = array_reverse($items);
    }
}

// column header for a given field.
// If it's the sort field, show up or down arrow to change order
// Otherwise name is a link to make it the sort field
//
function column_sort_header($field, $title, $url, $sort_field, $sort_rev) {
    if ($field == $sort_field) {
        return sprintf(
            '%s <a href="%s&sort_field=%s&sort_rev=%d">%s</a>',
            $title,
            $url, $sort_field,
            $sort_rev?0:1,
            $sort_rev?'&uarr;':'&darr;'
        );
    } else {
        return sprintf(
            '<a href="%s&sort_field=%s">%s</a>',
            $url, $field, $title
        );
    }
}

?>

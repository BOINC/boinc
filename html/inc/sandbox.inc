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
// - sandbox files live in the download hierarchy,
//   with names of the form sb_userid_md5
// - each user has a "sandbox dir" project/sandbox/userid.
//   The files in this have user-visible names, and contents of the form
//   sb file_size file_md5

require_once("../inc/util.inc");
require_once("../inc/submit_db.inc");
require_once("../inc/dir_hier.inc");

// Return path of sandbox directory for the given user.
// Create dir if not present.
//
if (!function_exists("sandbox_dir")){
function sandbox_dir($user) {
    $dir = parse_config(get_config(), "<sandbox_dir>");
    if (!$dir) {
        $dir = "../../sandbox/";
    }
    if (!is_dir($dir)) {
        mkdir($dir);
    }
    $d = "$dir/$user->id";
    if (!is_dir($d)) {
        mkdir($d);
    }
    return $d;
}
}

function sandbox_write_link_file($path, $size, $md5) {
    file_put_contents($path, "sb $size $md5");
}

// check if a link with the given md5 exists
//
function sandbox_lf_exists($user, $md5) {
    $exist = false;
    $elf = "";
    $dir = sandbox_dir($user);
    $files = sandbox_file_names($user);
    foreach ($files as $file) {
        $path = "$dir/$file";
        [$err, $file_size, $file_md5] = sandbox_parse_link_file($path);
        if (!$err){
            if (strcmp($md5, $file_md5) == 0) {
                $exist = true;
                $elf = $file;
                break;
            }
        }
    }
    return array($exist, $elf);
}

// parse a link file and return (error, size, md5)
//
function sandbox_parse_link_file($path) {
    if (!file_exists($path)) { return array(true, null, null); }
    $x = file_get_contents($path);
    $n = sscanf($x, "%s %d %s", $s, $size, $md5);
    if ($n != 3) return array(true, null, null);
    if ($s != 'sb') return array(true, null, null);
    return array(false, $size, $md5);
}

$fanout = parse_config(get_config(), "<uldl_dir_fanout>");

// return the physical name of the file
//
function sandbox_file_name($user, $md5) {
    $f = "sb_".$user->id."_".$md5;
    return $f;
}

// return the path of the file in the download directory
//
if (!function_exists('sandbox_physical_path')) {
function sandbox_physical_path($user, $md5) {
    global $fanout;
    $f = sandbox_file_name($user, $md5);
    return dir_hier_path($f, parse_config(get_config(), "<download_dir>"), $fanout);
}
}

// return list of files in sandbox
//
if (!function_exists('sandbox_file_names')) {
function sandbox_file_names($user) {
    $d = opendir(sandbox_dir($user));
    $names = array();
    while (($f = readdir($d)) !== false) {
        if ($f == '.') continue;
        if ($f == '..') continue;
        $names[] = $f;
    }
    natsort($names);
    return $names;
}
}

// return a <select> for files in sandbox
//
if (!function_exists('sandbox_file_select')) {
function sandbox_file_select($user, $select_name, $regexp = null, $allow_none = false) {
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
}

// convert logical (link) name to physical path
//
function sandbox_log_to_phys($user, $log_name) {
    $dir = sandbox_dir($user);
    [$err, $file_size, $file_md5] = sandbox_parse_link_file("$dir/$log_name");
    if ($err) return null;
    return sandbox_physical_path($user, $file_md5);
}

// check if a file is still being used by a unfinished batch
//
// TODO: this is a kludge.
// Should we use the job_file and batch_file_assoc tables instead?
//
function sandbox_file_in_use($user, $file){
    $ufiles = array();

    $pbatches = BoincBatch::enum(
        sprintf('user_id=%d and state!=%d and state!=%d',
            $user->id, BATCH_STATE_COMPLETE, BATCH_STATE_ABORTED
        )
    );
    if (!$pbatches) return false;

    foreach ($pbatches as $batch){
        $wus = BoincWorkUnit::enum("batch = $batch->id limit 1" );
        if ($wus == null){
            continue;
        }
        foreach($wus as $wu){
            $x = "<in>".$wu->xml_doc."</in>";
            $x = simplexml_load_string($x);
            global $fanout;
            foreach($x->workunit->file_ref as $fr){
                $pname = (string)$fr->file_name;
                $ufiles[] = $pname;
            }
        }
    }
    $dir = sandbox_dir($user);
    $path = $dir."/".$file;
    list($err, $size, $md5) = sandbox_parse_link_file($path);
    if (!$err){
        $f = sandbox_file_name($user, $md5);
        foreach($ufiles as $uf) {
            if (strcmp($f,$uf) == 0){
                return true;
            }

        }
    }
    return false;
}

?>

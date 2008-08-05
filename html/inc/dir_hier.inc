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

// PHP versions of functions in sched/sched_util.C

function filename_hash($filename, $fanout) {
    $m = md5($filename);
    $s = substr($m, 1, 7);
    sscanf($s, "%x", $n);
    return sprintf("%x", $n%$fanout);
}

function dir_hier_path($filename, $root, $fanout) {
    $dir = filename_hash($filename, $fanout);
    $dirpath = "$root/$dir";
    if (!is_dir($dirpath)) {
        mkdir($dirpath);
    }
    return "$dirpath/$filename";
}

function dir_hier_url($filename, $base, $fanout) {
    $dir = filename_hash($filename, $fanout);
    return "$base/$dir/$filename";
}

?>

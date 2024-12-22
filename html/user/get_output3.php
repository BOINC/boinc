<?php

// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2024 University of California
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

// get output files, individually or zipped groups
// Assumes the layout used by sample_assimilator.cpp and sample_assimilate.py:
// <project>/results/
//      <batchid>/   (0 if not in a batch)
//

require_once("../inc/util.inc");

// show or download a single output file,
// identified by result ID and file index
//
function get_file() {
    $path = get_str('path');
    if (strstr($path, '.')) error_page('bad path');
    $path = "../../$path";

    $download = get_str('download', true);
    if ($download) {
        do_download($path);
    } else {
        readfile($path);
    }
}

// download a zip of the given directory
//
function get_batch() {
    $batch_id = get_int('batch_id');
    $dir = "../../results/$batch_id";
    if (!is_dir($dir)) die('no batch dir');
    $name = "batch_$batch_id.zip";
    $cmd = "cd $dir; rm -f $name; zip -q $name *";
    system($cmd);
    do_download("$dir/$name");
}

$action = get_str('action');
switch ($action) {
case 'get_file': get_file(); break;
case 'get_batch': get_batch(); break;
}

?>

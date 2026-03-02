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

// get output files, individually or zipped groups,
// for apps that use the 'assim move' scheme
//
// args:
// action: get_file or get_batch
// get_file:
//      path: relative to project root dir
//      download: if set, download file; else show it in browser
// get_batch:
//      batch_id
//      downloads zip of batch's output files
//      Assumes the layout used by sample_assimilator.cpp
//      and sample_assimilate.py:
//      <project>/results/<batchid>/   (0 if not in a batch)

require_once("../inc/util.inc");
require_once("../inc/submit_util.inc");

// user is allowed to download output files from batch only if
// they own batch or have manage-all permissions
//
function check_auth($batch_id) {
    $user = get_logged_in_user();
    if (has_manage_access($user, 0)) {
        return;
    }
    $batch = BoincBatch::lookup_id($batch_id);
    if (!$batch || $user->id != $batch->user_id) {
        error_page('not owner');
    }
}

// show or download a single output file,
// identified by result ID and file index
//
function get_file() {
    $result_id = get_int('result_id');
    $index = get_int('index');
    $result = BoincResult::lookup_id($result_id);
    if (!$result) {
        error_page('no result');
    }
    $wu = BoincWorkunit::lookup_id($result->workunitid);
    if (!$wu) {
        error_page('no workunit');
    }
    check_auth($wu->batch);
    [$log_names, $gzip] = get_outfile_log_names($result);
    if ($index >= count($log_names)) {
        error_page('bad index');
    }
    $path = assim_move_outfile_path($wu, $index, $log_names, $gzip);

    if (get_str('download', true)) {
        do_download($path);
    } else {
        echo "<pre>\n";
        echo htmlspecialchars(file_get_contents($path));
        echo "</pre>\n";
    }
}

// download a zip of the given directory
//
function get_batch_zip() {
    $batch_id = get_int('batch_id');
    check_auth($batch_id);
    $dir = "../../results/$batch_id";
    if (!is_dir($dir)) {
        error_page('no batch dir');
    }
    $name = "batch_$batch_id.zip";
    $cmd = "cd $dir; rm -f $name; zip -q $name *";
    $line = system($cmd, $ret);
    if ($ret) {
        error_page("Zip failed: $line");
    }
    do_download("$dir/$name");
    unlink("$dir/$name");
}

function get_batch_tar() {
    $batch_id = get_int('batch_id');
    check_auth($batch_id);
    $dir = "../../results/$batch_id";
    if (!is_dir($dir)) {
        error_page('no batch dir');
    }

    $d = fopen($dir, 'r');
    if (!$d) {
        error_page('fopen() failed');
    }
    if (!flock($d, LOCK_EX|LOCK_NB)) {
        error_page(
            "A download of this batch is already in progress."
        );
    }

    // get the size of the tar file (fast - doesn't read files)
    //
    $cmd = "cd $dir; tar --totals -cf /dev/null * 2>&1";
    $f = popen($cmd, "r");
    if (!$f) {
        error_page('tar --totals failed');
    }
    $nbytes = -1;
    while (1) {
        $out = fgets($f);
        if (!$out) break;
        $x = sscanf($out, "Total bytes written: %d");
        if (count($x)) {
            $nbytes = $x[0];
            break;
        }
    }
    if ($nbytes<0) {
        error_page("tar --totals didn't produce result");
    }
    pclose($f);

    // generate tar file and stream to output
    //
    $name = "batch_$batch_id.tar";
    download_header($name, $nbytes);
    $cmd = "cd $dir; tar -cf - *";
    $f = popen($cmd, "r");
    if (!$f) {
        error_page('tar failed');
    }
    while (1) {
        $data = fread($f, 256*1024);
        if (!$data) {
            break;
        }
        echo $data;
        flush();
    }
    pclose($f);
    fclose($d);
}

$action = get_str('action');
switch ($action) {
case 'get_file':
    get_file();
    break;
case 'get_batch_zip':
    get_batch_zip();
    break;
case 'get_batch_tar':
    get_batch_tar();
    break;
}

?>

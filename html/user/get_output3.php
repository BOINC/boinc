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
//      <project>/results/
//      <batchid>/   (0 if not in a batch)
//

require_once("../inc/util.inc");
require_once("../inc/submit_util.inc");

// the path of an output file in the 'assim move' scheme.
// This must agree with the corresponding assimilators:
// tools/sample_assimilate.py
// sched/sample_assimilator.cpp
// and with tools/query_job
//
function outfile_path($wu, $index, $log_names) {
    if (!is_valid_filename($wu->name)) error_page("bad WU name");
    if (!is_valid_filename($log_names[$index])) error_page("bad logical name");
    return sprintf('results/%d/%s__file_%s',
        $wu->batch, $wu->name, $log_names[$index]
    );
}

// show or download a single output file,
// identified by result ID and file index
//
function get_file() {
    $result_id = get_int('result_id');
    $index = get_int('index');
    $result = BoincResult::lookup_id($result_id);
    if (!$result) error_page('no result');
    $wu = BoincWorkunit::lookup_id($result->workunitid);
    if (!$wu) error_page('no workunit');
    $log_names = get_outfile_log_names($result);
    if ($index >= count($log_names)) error_page('bad index');
    $path = sprintf('../../%s', outfile_path($wu, $index, $log_names));

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

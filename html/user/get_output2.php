<?php

// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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

// web API for fetching output files.
// This is for apps that don't use an assimilator
// that moves output files into project/results/...
// I.e., the output files are in the upload hierarchy with physical names
//
// For assim_move apps, use get_output3.php instead of this.

// This is an updated version of get_output.php;
// I didn't want to change that on the (unlikely) chance
// that someone is using it.

// args:
// cmd: batch, workunit, result or file
// auth: optional user authenticator (if not logged in)
// result:
//      result_id
// workunit:
//      wu_id
// batch:
//      batch_id
// file:
//      result_id
//      file_num
//
// action (see https://github.com/BOINC/boinc/issues/5262):
// result: if 1 output file, return it as resultname__logicalname
//          (that way you know what it is, and it has the right extension)
//      else return a zip file as resultname.zip,
//          containing the output files with their logical names
// workunit:
//      as above, for canonical instance
// batch:
//      as above for each workunit; return as batch_(batchid).zip
//
// In the result and workunit cases, they must be part of a batch
// (so that we know who the submitter is)

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/dir_hier.inc");
require_once("../inc/submit_util.inc");

function check_auth($auth, $batch) {
    $user = BoincUser::lookup_id($batch->user_id);
    if (!$user) die("no batch owner for $batch->id");
    if ($user->authenticator != $auth) die('bad auth');
}

function do_result_aux($result, $batch, $file_num=null) {
    $phys_names = get_outfile_phys_names($result);
    $log_names = get_outfile_log_names($result);
    if ($file_num !== null) {
        $path = upload_path($phys_names[$file_num]);
        do_download($path,
            sprintf("%s__%s", $result->name, $log_names[$file_num])
        );
    }
    if (count($phys_names) == 1) {
        $path = upload_path($phys_names[0]);
        do_download($path,
            sprintf("%s__%s", $result->name, $log_names[0])
        );
    } else {
        // make a temp dir in submit/batchid;
        // make symlinks there, and zip it
        $dir_path = "submit/$batch->id/$result->name";
        system("rm -r $dir_path");
        mkdir($dir_path);
        for ($i=0; $i<count($phys_names); $i++) {
            $cmd = sprintf('ln -s %s %s/%s',
                upload_path($phys_names[$i]),
                $dir_path,
                $log_names[$i]
            );
            system($cmd);
        }
        $cmd = sprintf('cd submit/%d; zip %s',
            $batch->id,
            $result->name
        );
        system($cmd);
        do_download("$dir_path.zip");
    }
}

function do_result($result_id, $auth, $file_num=null) {
    $result = BoincResult::lookup_id($result_id);
    if (!$result) die("no result $result_id");
    $workunit = BoincWorkunit::lookup_id($result->workunitid);
    if (!$workunit) die("no workunit for $result_id");
    $batch = BoincBatch::lookup_id($workunit->batch);
    if (!$batch) die("no batch for $result_id");
    check_auth($auth, $batch);
    do_result_aux($result, $batch, $file_num);
}

function do_wu($wu_id, $auth) {
    $workunit = BoincWorkunit::lookup_id($wu_id);
    if (!$workunit) die("no workunit for $result_id");
    $batch = BoincBatch::lookup_id($workunit->batch);
    if (!$batch) die("no batch for $result_id");
    $result = BoincResult::lookup_id($workunit->canonical_resultid);
    do_result_aux($result, $batch);
}

// make a temp dir in submit/batchid
// for each workunit in batch,
// put symlinks to its output file (or a dir of its output files) there.
// send the zipped temp dir.
//
function do_batch($batch_id, $auth) {
    $batch = BoincBatch::lookup_id($batch_id);
    if (!$batch) die("no batch $batch_id");
    $dir_path = sprintf('submit/%d/batch_%d', $batch_id, $batch_id);
    system("rm -r $dir_path");
    mkdir($dir_path);

    $wus = BoincWorkunit::enum("batch=$batch_id and canonical_resultid<>0");
    foreach ($wus as $wu) {
        $result = BoincResult::lookup_id($wu->canonical_resultid);
        $phys_names = get_outfile_phys_names($result);
        $log_names = get_outfile_log_names($result);
        if (count($phys_names) == 1) {
            $cmd = sprintf('ln -s %s %s/%s__%s',
                upload_path($phys_names[0]),
                $dir_path,
                $result->name,
                $log_names[0]
            );
            system($cmd);
        } else {
            mkdir(sprintf('%s/%s', $dir_path, $result->name));
            for ($i=0; $i<count($phys_names); $i++) {
                $cmd = sprintf('ln -s %s %s/%s/%s',
                    upload_path($phys_names[$i]),
                    $dir_path,
                    $result->name,
                    $log_names[$i]
                );
                system($cmd);
            }
        }
    }
    $cmd = sprintf('cd submit/%d/batch_%d; zip -q -r ../batch_%d.zip *',
        $batch_id,
        $batch_id,
        $batch_id
    );
    system($cmd);
    do_download(sprintf('submit/%d/batch_%d.zip', $batch_id, $batch_id));
    // todo: clean up
}

$cmd = get_str('cmd');
$user = get_logged_in_user(false);
if ($user) {
    $auth = $user->authenticator;
} else {
    $auth = get_str('auth');
}
switch ($cmd) {
case 'result':
    do_result(get_int('result_id'), $auth);
    break;
case 'workunit':
    do_wu(get_int('wu_id'), $auth);
    break;
case 'batch':
    do_batch(get_int('batch_id'), $auth);
    break;
case 'file':
    do_result(get_int('result_id'), $auth, get_int('file_num'));
    break;
default:
    die("bad cmd\n");
}
?>

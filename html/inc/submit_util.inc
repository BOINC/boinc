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

// server-side utility functions for remote job submission and control

require_once("../inc/submit_db.inc");

// The status of a workunit.
// Not stored in the DB;
// it's computed by get_batch_params() and added to the workunit object
//
define('WU_UNSENT', 1);
define('WU_IN_PROGRESS', 2);
define('WU_SUCCESS', 3);
define('WU_ERROR', 4);

// write status and error messages to log
//
function log_write($x) {
    static $enabled, $log_file;

    if (!isset($enabled)) {
        $enabled = false;
        $filename = parse_config(get_config(), "<remote_submit_log>");
        if (!$filename) {
            return;
        }
        $log_dir = parse_config(get_config(), "<log_dir>");
        if (!$log_dir) {
            return;
        }
        $log_file = fopen("$log_dir/$filename", "a");
        if (!$log_file) return;
        $enabled = true;
    }
    if (!$enabled) return;
    fwrite($log_file, sprintf("%s: %s\n", strftime("%c"), $x));
    fflush($log_file);
}

// return # of in-progress jobs for user
//
function n_jobs_in_progress($user_id) {
    $batches = BoincBatch::enum(
        sprintf(
            'user_id=%d and state=%d',
            $user_id, BATCH_STATE_IN_PROGRESS
        )
    );
    $ids = [];
    foreach ($batches as $batch) {
        $ids[] = $batch->id;
    }
    if (!$ids) return 0;
    $ids = implode(',', $ids);
    return BoincWorkunit::count(
        sprintf('error_mask=0 and canonical_resultid=0 and batch in (%s)',
            $ids
        )
    );
}

// in remote job submission,
// for input files of type local, semilocal, and inline,
// we need to give a unique physical name based on its content.
// Prepend the jf_ to make the origin of the file clear
//
function job_file_name($md5) {
    return "jf_$md5";
}

// can user upload files?
//
function has_file_access($user) {
    $us = BoincUserSubmit::lookup_userid($user->id);
    if (!$us) return false;
    return true;
}

// can user submit to given app?
//
function has_submit_access($user, $app_id) {
    $us = BoincUserSubmit::lookup_userid($user->id);
    if (!$us) return false;
    if ($us->submit_all) return true;
    $usa = BoincUserSubmitApp::lookup("user_id=$user->id and app_id=$app_id");
    if (!$usa) return false;
    return true;
}

// can user manage given app (or all apps if zero)?
//
function has_manage_access($user, $app_id) {
    $us = BoincUserSubmit::lookup_userid($user->id);
    if (!$us) return false;
    if ($us->manage_all) return true;
    $usa = BoincUserSubmitApp::lookup("user_id=$user->id and app_id=$app_id");
    if (!$usa) return false;
    return $usa->manage;
}

// check whether user has permissions for a remote job submission
// or job file request.
// $r is a request message that includes an 'authenticator' field
// $app is the app being submitted to (or null if file op)
// returns user, or give XML error and quit
//
function check_remote_submit_permissions($r, $app) {
    $auth = (string)$r->authenticator;
    if (!$auth) {
        log_write("no authenticator");
        xml_error(-1, "no authenticator");
    }
    $auth = BoincDb::escape_string($auth);
    $user = BoincUser::lookup("authenticator='$auth'");
    if (!$user) {
        log_write("bad authenticator");
        xml_error(-1, "bad authenticator");
    }

    // check access
    //
    if ($app) {
        if (!has_submit_access($user, $app->id)) {
            log_write("no submit access");
            xml_error(-1, "no submit access");
        }
    } else {
        if (!has_file_access($user)) {
            log_write("no file access");
            xml_error(-1, "no file access");
        }
    }
    return $user;
}

// remove all of user's permissions
//
function delete_remote_submit_user($user) {
    BoincUserSubmit::delete_user($user->id);
    BoincUserSubmitApp::delete_user($user->id);
}

// compute parameters of the batch:
//   credit_canonical: credit granted to canonical instances
//   fraction_done: frac of jobs that are done (success or failed)
//   state: whether complete (all jobs done)
//   completion_time: if newly complete
//   nerror_jobs: # of failed jobs
// Update the above in DB.
// Also compute (not in DB):
//   njobs_success: # of jobs with canonical instance
//   njobs_in_prog: # of jobs not success or fail,
//      and at least one result in progress
// return the batch object, with these values
//
// You need to pass in a list of the batch's WUs.
// These need to have the following fields:
//
// id
// rsc_fpops_est
// canonical_resultid
// canonical_credit
// error_mask
//
// add 'status' field (UNSENT, IN_PROGRESS, SUCCESS) to WUs
//
// TODO: update est_completion_time
//
function get_batch_params($batch, $wus) {
    if ($batch->state == BATCH_STATE_INIT) {
        // a batch in INIT state has no jobs
        //
        return $batch;
    }
    if (!$wus) {
        if ($batch->njobs) {
            $batch->update('njobs=0');
            $batch->njobs = 0;
        }
        return $batch;
    }

    // make list of WU IDs with an in-progress result
    $res_in_prog = BoincResult::enum_fields(
        'workunitid',
        sprintf('batch=%d and server_state in (%d, %d)',
            $batch->id,
            RESULT_SERVER_STATE_IN_PROGRESS, RESULT_SERVER_STATE_OVER
        )
    );
    $wus_in_prog = [];
    foreach ($res_in_prog as $res) {
        $wus_in_prog[$res->workunitid] = true;
    }
    unset($res_in_progress);    // does this do anything?

    $fp_total = 0;
    $fp_done = 0;
    $completed = true;
    $batch->nerror_jobs = 0;
    $batch->credit_canonical = 0;
    $njobs_success = 0;
    $njobs_in_prog = 0;
    foreach ($wus as $wu) {
        $fp_total += $wu->rsc_fpops_est;
        if ($wu->canonical_resultid) {
            $fp_done += $wu->rsc_fpops_est;
            $njobs_success++;
            $batch->credit_canonical += $wu->canonical_credit;
            $wu->status = WU_SUCCESS;
        } else if ($wu->error_mask) {
            $batch->nerror_jobs++;
            $wu->status = WU_ERROR;
        } else {
            $completed = false;
            if (array_key_exists($wu->id, $wus_in_prog)) {
                $njobs_in_prog++;
                $wu->status = WU_IN_PROGRESS;
            } else {
                $wu->status = WU_UNSENT;
            }
        }
    }
    $njobs = count($wus);
    $batch->njobs = $njobs;
    $batch->fraction_done = ($njobs_success + $batch->nerror_jobs)/$batch->njobs;
    if ($completed && $batch->state == BATCH_STATE_IN_PROGRESS) {
        $batch->state = BATCH_STATE_COMPLETE;
        $batch->completion_time = time();
    }
    $batch->update("fraction_done = $batch->fraction_done, nerror_jobs = $batch->nerror_jobs, state=$batch->state, completion_time = $batch->completion_time, credit_canonical = $batch->credit_canonical, njobs=$njobs");

    $batch->njobs_success = $njobs_success;
    $batch->njobs_in_prog = $njobs_in_prog;
    return $batch;
}

// get params of in-progress batches; they might not be in progress anymore.
//
function get_batches_params($batches) {
    $b = [];
    foreach ($batches as $batch) {
        if ($batch->state == BATCH_STATE_IN_PROGRESS) {
            $wus = BoincWorkunit::enum_fields(
                'id, name, rsc_fpops_est, canonical_credit, canonical_resultid, error_mask',
                "batch = $batch->id"
            );
            $b[] = get_batch_params($batch, $wus);
        } else {
            $b[] = $batch;
        }
    }
    return $b;
}

// get the physical names of a result's output files.
//
function get_outfile_phys_names($result) {
    $names = [];
    $xml = "<a>".$result->xml_doc_out."</a>";
    $r = simplexml_load_string($xml);
    if (!$r) return $names;
    foreach ($r->file_info as $fi) {
        $names[] = (string)($fi->name);
    }
    return $names;
}

function get_outfile_log_names($result) {
    $names = [];
    $xml = "<a>".$result->xml_doc_in."</a>";
    $r = simplexml_load_string($xml);
    if (!$r) return $names;
    foreach ($r->result->file_ref as $fr) {
        $names[] = (string)($fr->open_name);
    }
    return $names;
}

function get_outfile_paths($result) {
    $fanout = parse_config(get_config(), "<uldl_dir_fanout>");
    $upload_dir = parse_config(get_config(), "<upload_dir>");

    $paths = array();
    $xml = "<a>".$result->xml_doc_out."</a>";
    $r = simplexml_load_string($xml);
    if (!$r) return $paths;
    foreach ($r->file_info as $fi) {
        $path = dir_hier_path((string)($fi->name), $upload_dir, $fanout);
        $paths[] = $path;
    }
    return $paths;
}

function abort_workunit($wu) {
    BoincResult::update_aux(
        sprintf(
            'server_state=%d, outcome=%d where server_state=%d and workunitid=%d',
            RESULT_SERVER_STATE_OVER, RESULT_OUTCOME_DIDNT_NEED,
            RESULT_SERVER_STATE_UNSENT,
            $wu->id
        )
    );
    $wu->update(
        sprintf('error_mask=error_mask|%d, file_delete_state=%d',
            WU_ERROR_CANCELLED, FILE_DELETE_READY
        )
    );
}

function abort_batch($batch) {
    $wus = BoincWorkunit::enum_fields(
        'id',
        "batch=$batch->id"
    );
    $ids = [];
    foreach ($wus as $wu) {
        $ids[] = $wu->id;
    }
    if ($ids) {
        $ids = implode(',', $ids);
        // cancel unsent instances
        //
        BoincResult::update_aux(
            sprintf(
                'server_state=%d, outcome=%d where server_state=%d and workunitid in (%s)',
                RESULT_SERVER_STATE_OVER, RESULT_OUTCOME_DIDNT_NEED,
                RESULT_SERVER_STATE_UNSENT,
                $ids
            )
        );
        BoincWorkunit::update_aux(
            sprintf('error_mask=error_mask|%d, file_delete_state=%d where id in(%s)',
                WU_ERROR_CANCELLED, FILE_DELETE_READY, $ids
            )
        );
    }
    $batch->update(
        sprintf('state=%d, completion_time=%d', BATCH_STATE_ABORTED, time())
    );
    return 0;
}

// retire a batch:
// - mark the batch as retired (don't delete it; might be useful later)
// - delete output files (results/batchid/)
// - mark unsent WUs as cancelled
//
// Don't delete the batch; it may have in-progress jobs that
// we still need to file-delete and purge,
// and the batch needs to be present for this.
// Use ops/delete_batches.php to delete retired batches with no WUs
//
function retire_batch($batch) {
    system("rm -rf ../../results/$batch->id");

    // get list of unsent WUs in this batch
    //
    $wus = BoincWorkunit::enum_fields(
        'id, rsc_fpops_est, canonical_credit, canonical_resultid, error_mask',
        "batch = $batch->id"
    );
    get_batch_params($batch, $wus);
    $batch->update("state=".BATCH_STATE_RETIRED);
        // do this AFTER get_batch_params()
    $wu_ids = [];
    foreach ($wus as $wu) {
        if ($wu->status == WU_UNSENT) {
            $wu_ids[] = $wu->id;
        }
    }

    // cancel them
    //
    if ($wu_ids) {
        $wu_ids = implode(',', $wu_ids);
        BoincWorkunit::update_aux(
            sprintf('error_mask=error_mask|%d, file_delete_state=%d where id in (%s)',
                WU_ERROR_CANCELLED, FILE_DELETE_READY, $wu_ids
            )
        );
    }
}

function expire_batch($batch) {
    abort_batch($batch);
    retire_batch($batch);
    $batch->update("state=".BATCH_STATE_EXPIRED);
}

function batch_state_string($state) {
    switch ($state) {
    case BATCH_STATE_INIT: return "new";
    case BATCH_STATE_IN_PROGRESS: return "in progress";
    case BATCH_STATE_COMPLETE: return "completed";
    case BATCH_STATE_ABORTED: return "aborted";
    case BATCH_STATE_RETIRED: return "retired";
    }
    return "unknown state $state";
}
// get the total size of output files of a batch
//
function batch_output_file_size($batchid) {
    $batch_td_size=0;
    $wus = BoincWorkunit::enum_fields(
        'canonical_resultid',
        "batch=$batchid"
    );
    $fanout = parse_config(get_config(), "<uldl_dir_fanout>");
    $upload_dir = parse_config(get_config(), "<upload_dir>");
    foreach ($wus as $wu) {
        if (!$wu->canonical_resultid) continue;
        $result = BoincResult::lookup_id($wu->canonical_resultid);
        $names = get_outfile_phys_names($result);
        foreach ($names as $name) {
            $path = dir_hier_path($name, $upload_dir, $fanout);
            if (is_file($path)) {
                $batch_td_size += filesize($path);
            }
        }
    }
    return $batch_td_size;
}

function boinc_get_output_file_url($user, $result, $i) {
    $name = $result->name;
    $auth_str = md5($user->authenticator.$name);
    return "get_output.php?cmd=result_file&result_name=$name&file_num=$i&auth_str=$auth_str";
}

function boinc_get_output_files_url($user, $batch_id) {
    $auth_str = md5($user->authenticator.$batch_id);
    return "get_output.php?cmd=batch_files&batch_id=$batch_id&auth_str=$auth_str";
}

function boinc_get_wu_output_files_url($user, $wu_id) {
    $auth_str =  md5($user->authenticator.$wu_id);
    return "get_output.php?cmd=workunit_files&wu_id=$wu_id&auth_str=$auth_str";
}

////////////////// FILE INFO FILES //////////////

// these are used:
// 1) in user file sandbox
// 2) in BUDA app variant dirs
// in each case a file dir/foo has an info file dir/.md5/foo
// containing its md5 and size
// (same format as .md5 files in download hierarchy)

// get the MD5 and size of a file
//
function get_file_info($path) {
    $md5 = md5_file($path);
    $s = stat($path);
    $size = $s['size'];
    return [$md5, $size];
}

// write a "info file" containing MD5 and size
//
function write_info_file($path, $md5, $size) {
    file_put_contents($path, "$md5 $size");
}

// parse info file and return [md5, size]
//
function parse_info_file($path) {
    if (!file_exists($path)) return null;
    $x = file_get_contents($path);
    $n = sscanf($x, "%s %d", $md5, $size);
    if ($n != 2 || strlen($md5)!=32) {
        return null;
    }
    return [$md5, $size];
}

///////////////// TEMPLATE CREATION //////////////

function file_ref_in($fname) {
    return(sprintf(
'      <file_ref>
         <open_name>%s</open_name>
         <copy_file/>
      </file_ref>
',
        $fname
    ));
}
function file_info_out($i) {
    return sprintf(
'    <file_info>
        <name><OUTFILE_%d/></name>
        <generated_locally/>
        <upload_when_present/>
        <max_nbytes>5000000</max_nbytes>
        <url><UPLOAD_URL/></url>
    </file_info>
',
        $i
    );
}

function file_ref_out($i, $fname) {
    return sprintf(
'        <file_ref>
            <file_name><OUTFILE_%d/></file_name>
            <open_name>%s</open_name>
            <copy_file/>
        </file_ref>
',      $i, $fname
    );
}

////////////// ZIP FILES //////////////
// our job submission systems (BUDA and autodock)
// expect zip files that contain files.
// But some people are used to creating zip files
// that contain a (single) directory that contains files.

// if the given directory contains a single item, and it's a directory,
// return its name.
// Else return false.
//
function contains_single_dir($dir) {
    $files = scandir($dir);
    if (count($files) != 3) return false;
    foreach ($files as $file) {
        if ($file[0] == '.') continue;
        if (is_dir("$dir/$file")) return $file;
        return false;
    }
}

?>

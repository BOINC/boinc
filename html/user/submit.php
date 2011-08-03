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

// Handler for remote job submission.
// See http://boinc.berkeley.edu/trac/wiki/RemoteJobs

require_once("../inc/boinc_db.inc");
require_once("../inc/submit_db.inc");
require_once("../inc/xml.inc");
require_once("../inc/dir_hier.inc");
require_once("../inc/result.inc");

error_reporting(E_ALL);
ini_set('display_errors', true);
ini_set('display_startup_errors', true);

function error($s) {
    echo "<error>\n<message>$s</message>\n</error>\n";
    exit;
}

function authenticate_user($r, $app) {
    $auth = (string)$r->authenticator;
    if (!$auth) error("no authenticator");
    $user = BoincUser::lookup("authenticator='$auth'");
    if (!$user) error("bad authenticator");
    $user_submit = BoincUserSubmit::lookup_userid($user->id);
    if (!$user_submit) error("no submit access");
    if ($app && !$user_submit->all_apps) {
        $usa = BoincUserSubmitApp::lookup("user_id=$user->id and app_id=$app->id");
        if (!$usa) {
            error("no submit access");
        }
    }
    return array($user, $user_submit);
}

function get_app($r) {
    $name = (string)($r->batch->app_name);
    $app = BoincApp::lookup("name='$name'");
    if (!$app) error("no app");
    return $app;
}

function batch_flop_count($r) {
    $x = 0;
    foreach($r->batch->job as $job) {
        $x += (double)$job->rsc_fpops_est;
    }
    return $x;
}

// estimate project FLOPS based on recent average credit
//
function project_flops() {
    $x = BoincUser::sum("expavg_credit");
    $y = 1e9*$x/200;
    return $y;
}

function est_elapsed_time($r) {
    // crude estimate: batch FLOPs / project FLOPS
    //
    return batch_flop_count($r) / project_flops();
}

function estimate_batch($r) {
    $app = get_app($r);
    list($user, $user_submit) = authenticate_user($r, $app);

    $e = est_elapsed_time($r);
    echo "<estimate>\n<seconds>$e</seconds>\n</estimate>\n";
}

function read_input_template($app) {
    $path = "../../templates/$app->name"."_in";
    return simplexml_load_file($path);
}

function validate_batch($r, $template) {
    $i = 0;
    foreach($r->batch->job as $job) {
        $n = count($template->file_info);
        $m = count($job->input_file);
        if ($n != $m) {
            error("wrong # of input files for job $i: need $n, got $m");
        }
        $i++;
    }
}

$fanout = parse_config(get_config(), "<uldl_dir_fanout>");

function stage_file($file) {
    global $fanout;

    $source = (string)$file->source;
    $md5 = md5_file($source);
    if (!$md5) {
        error("Can't get MD5 of file $source");
    }
    $name = "batch_$md5";
    $path = dir_hier_path($name, "../../download", $fanout);
    if (file_exists($path)) return;
    if (!copy($source, $path)) {
        error("can't copy file from $source to $path");
    }
}

function stage_files($r, $template) {
    foreach($r->batch->job as $job) {
        foreach ($job->input_file as $file) {
            stage_file($file);
        }
    }
}

function submit_job($job, $template, $app, $batch_id, $i) {
    $cmd = "cd ../..; ./bin/create_work --appname $app->name --batch $batch_id";
    $cmd .= " --wu_name batch_".$batch_id."_".$i;
    foreach ($job->input_file as $file) {
        $name = (string)$file->name;
        $cmd .= " $name";
    }
    $ret = system($cmd);
    if ($ret === FALSE) {
        error("can't create job");
    }
}

function submit_batch($r) {
    $app = get_app($r);
    list($user, $user_submit) = authenticate_user($r, $app);
    $template = read_input_template($app);
    validate_batch($r, $template);
    stage_files($r, $template);
    $njobs = count($r->batch->job);
    $now = time();
    $batch_name = (string)($r->batch->batch_name);
    $batch_id = BoincBatch::insert(
        "(user_id, create_time, njobs, name, app_id) values ($user->id, $now, $njobs, '$batch_name', $app->id)"
    );
    $i = 0;
    foreach($r->batch->job as $job) {
        submit_job($job, $template, $app, $batch_id, $i++);
    }
    $batch = BoincBatch::lookup_id($batch_id);
    $batch->update("state=".BATCH_STATE_IN_PROGRESS);
    echo "<batch_id>$batch_id</batch_id>\n";
}

// compute and update params of a batch
// NOTE: eventually this should be done by server components
// (transitioner, validator etc.) as jobs complete or time out
//
// TODO: update est_completion_time
//
function get_batch_params($batch, $wus) {
    if ($batch->state > BATCH_STATE_IN_PROGRESS) return $batch;
    $fp_total = 0;
    $fp_done = 0;
    $completed = true;
    $nerror_jobs = 0;
    $credit_canonical = 0;
    foreach ($wus as $wu) {
        $fp_total += $wu->rsc_fpops_est;
        if ($wu->canonical_resultid) {
            $fp_done += $wu->rsc_fpops_est;
            $credit_canonical += $wu->canonical_credit;
        } else if ($wu->error_mask) {
            $nerror_jobs++;
        } else {
            $completed = false;
        }
    }
    if ($fp_total) {
        $batch->fraction_done = $fp_done / $fp_total;
    }
    if ($completed && $batch->state < BATCH_STATE_COMPLETE) {
        $batch->state = BATCH_STATE_COMPLETE;
        $batch->completion_time = time();
    }
    $batch->nerror_jobs = $nerror_jobs;
    $batch->update("fraction_done = $batch->fraction_done, nerror_jobs = $batch->nerror_jobs, state=$batch->state, completion_time = $batch->completion_time, credit_canonical = $batch->credit_canonical");
    return $batch;
}

function print_batch_params($batch) {
    $app = BoincApp::lookup_id($batch->app_id);
    echo "
        <id>$batch->id</id>
        <create_time>$batch->create_time</create_time>
        <est_completion_time>$batch->est_completion_time</est_completion_time>
        <njobs>$batch->njobs</njobs>
        <fraction_done>$batch->fraction_done</fraction_done>
        <nerror_jobs>$batch->nerror_jobs</nerror_jobs>
        <state>$batch->state</state>
        <completion_time>$batch->completion_time</completion_time>
        <credit_estimate>$batch->credit_estimate</credit_estimate>
        <credit_canonical>$batch->credit_canonical</credit_canonical>
        <credit_total>$batch->credit_total</credit_total>
        <name>$batch->name</name>
        <app_name>$app->name</app_name>
";
}

function query_batches($r) {
    list($user, $user_submit) = authenticate_user($r, null);
    $batches = BoincBatch::enum("user_id = $user->id");
    echo "<batches>\n";
    foreach ($batches as $batch) {
        $wus = BoincWorkunit::enum("batch = $batch->id");
        $batch = get_batch_params($batch, $wus);
        echo "    <batch>\n";
        print_batch_params($batch);
        echo "   </batch>\n";
    }
    echo "</batches>\n";
}

function n_outfiles($wu) {
    $path = "../../$wu->result_template_file";
    $r = simplexml_load_file($path);
    return count($r->file_info);
}

function query_batch($r) {
    list($user, $user_submit) = authenticate_user($r, null);
    $batch_id = (int)($r->batch_id);
    $batch = BoincBatch::lookup_id($batch_id);
    if (!$batch) {
        error("no such batch");
    }
    if ($batch->user_id != $user->id) {
        error("not owner");
    }

    $wus = BoincWorkunit::enum("batch = $batch_id");
    $batch = get_batch_params($batch, $wus);
    echo "<batch>\n";
    print_batch_params($batch);
    $n_outfiles = n_outfiles($wus[0]);
    foreach ($wus as $wu) {
        echo "    <job>
        <id>$wu->id</id>
        <canonical_instance_id>$wu->canonical_resultid</canonical_instance_id>
        <n_outfiles>$n_outfiles</n_outfiles>
        </job>
";
    }
    echo "</batch>\n";
}

function query_job($r) {
    list($user, $user_submit) = authenticate_user($r, null);
    $job_id = (int)($r->job_id);
    $wu = BoincWorkunit::lookup_id($job_id);
    if (!$wu) error("no such job");
    $batch = BoincBatch::lookup_id($wu->batch);
    if ($batch->user_id != $user->id) {
        error("not owner");
    }
    echo "<job>\n";
    $results = BoincResult::enum("workunitid=$job_id");
    foreach ($results as $result) {
        echo "    <instance>
        <name>$result->name</name>
        <id>$result->id</id>
        <state>".state_string($result)."</state>
";
        if ($result->server_state == 5) {   // over?
            $paths = get_outfile_paths($result);
            foreach($paths as $path) {
                if (is_file($path)) {
                    $size = filesize($path);
                    echo "        <outfile>
            <size>$size</size>
        </outfile>
";
                }
            }
        }
        echo "</instance>\n";
    }
    echo "</job>\n";
}

function abort_batch($r) {
    list($user, $user_submit) = authenticate_user($r, null);
    $batch_id = (int)($r->batch_id);
    $batch = BoincBatch::lookup_id($batch_id);
    if ($batch->user_id != $user->id) {
        error("not owner");
    }
    $wus = BoincWorkunit::enum("batch=$batch_id");
    foreach ($wus as $wu) {
        abort_workunit($wu);
    }
    $batch->update("state=".BATCH_STATE_ABORTED);
    echo "<success>1</success>";
}

function retire_batch($r) {
    list($user, $user_submit) = authenticate_user($r, null);
    $batch_id = (int)($r->batch_id);
    $batch = BoincBatch::lookup_id($batch_id);
    if ($batch->user_id != $user->id) {
        error("not owner");
    }
    $wus = BoincWorkunit::enum("batch=$batch_id");
    $now = time();
    foreach ($wus as $wu) {
        $wu->update("assimilate_state=2, transition_time=$now");
    }
    $batch->update("state=".BATCH_STATE_RETIRED);
    echo "<success>1</success>";
}

if (0) {
$r = simplexml_load_string(
    "<query_job>
    <authenticator>x</authenticator>
    <job_id>305613</job_id>
    </query_job>");
query_job($r);
exit;
}

xml_header();
$r = simplexml_load_string($_POST['request']);

if (!$r) {
    error("can't parse request message");
}

switch ($r->getName()) {
    case 'estimate_batch': estimate_batch($r); break;
    case 'submit_batch': submit_batch($r); break;
    case 'query_batches': query_batches($r); break;
    case 'query_batch': query_batch($r); break;
    case 'query_job': query_job($r); break;
    case 'abort_batch': abort_batch($r); break;
    case 'retire_batch': retire_batch($r); break;
    default: error("bad command");
}

?>

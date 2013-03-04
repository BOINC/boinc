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
require_once("../inc/submit_util.inc");

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
    if ($app && !$user_submit->submit_all) {
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
    if ($x == 0) $x = 200;
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

function validate_batch($jobs, $template) {
    $i = 0;
    $n = count($template->file_info);
    foreach($jobs as $job) {
        $m = count($job->input_files);
        if ($n != $m) {
            error("wrong # of input files for job $i: need $n, got $m");
        }
        $i++;
    }
}

$fanout = parse_config(get_config(), "<uldl_dir_fanout>");

// stage a file, and return the physical name
//
function stage_file($file) {
    global $fanout;

    $md5 = md5_file($file->source);
    if (!$md5) {
        error("Can't get MD5 of file $file->source");
    }
    $name = "batch_$md5";
    $path = dir_hier_path($name, "../../download", $fanout);
    if (file_exists($path)) return $name;
    if (!copy($file->source, $path)) {
        error("can't copy file from $file->source to $path");
    }
    return $name;
}

// stage all the files
//
function stage_files(&$jobs, $template) {
    foreach($jobs as $job) {
        foreach ($job->input_files as $file) {
            $file->name = stage_file($file);
        }
    }
}

function submit_job($job, $template, $app, $batch_id, $i) {
    $cmd = "cd ../..; ./bin/create_work --appname $app->name --batch $batch_id --rsc_fpops_est $job->rsc_fpops_est";
    if ($job->command_line) {
        $cmd .= " --command_line \"$job->command_line\"";
    }
    $cmd .= " --wu_name batch_".$batch_id."_".$i;
    foreach ($job->input_files as $file) {
        $cmd .= " $file->name";
    }
    $ret = system($cmd);
    if ($ret === FALSE) {
        error("can't create job");
    }
}

function xml_get_jobs($r) {
    $jobs = array();
    foreach($r->batch->job as $j) {
        $job = null;
        $job->input_files = array();
        $job->command_line = (string)$j->command_line;
        $job->rsc_fpops_est = (double)$j->rsc_fpops_est;
        foreach ($j->input_file as $f) {
            $file = null;
            $file->source = (string)$f->source;
            $job->input_files[] = $file;
        }
        $jobs[] = $job;
    }
    return $jobs;
}

function submit_batch($r) {
    $app = get_app($r);
    list($user, $user_submit) = authenticate_user($r, $app);
    $template = read_input_template($app);
    $jobs = xml_get_jobs($r);
    validate_batch($jobs, $template);
    stage_files($jobs, $template);
    $njobs = count($jobs);
    $now = time();
    $batch_name = (string)($r->batch->batch_name);
    $batch_id = BoincBatch::insert(
        "(user_id, create_time, njobs, name, app_id) values ($user->id, $now, $njobs, '$batch_name', $app->id)"
    );
    $i = 0;
    foreach($jobs as $job) {
        submit_job($job, $template, $app, $batch_id, $i++);
    }
    $batch = BoincBatch::lookup_id($batch_id);
    $batch->update("state=".BATCH_STATE_IN_PROGRESS);
    echo "<batch_id>$batch_id</batch_id>\n";
}

function print_batch_params($batch) {
    $app = BoincApp::lookup_id($batch->app_id);
    if (!$app) $app->name = "none";
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
        <name>$batch->name</name>
        <app_name>$app->name</app_name>
";
}

function query_batches($r) {
    list($user, $user_submit) = authenticate_user($r, null);
    $batches = BoincBatch::enum("user_id = $user->id");
    echo "<batches>\n";
    foreach ($batches as $batch) {
        if ($batch->state < BATCH_STATE_COMPLETE) {
            $wus = BoincWorkunit::enum("batch = $batch->id");
            $batch = get_batch_params($batch, $wus);
        }
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

function handle_abort_batch($r) {
    list($user, $user_submit) = authenticate_user($r, null);
    $batch_id = (int)($r->batch_id);
    $batch = BoincBatch::lookup_id($batch_id);
    if (!$batch) error("no such batch");
    if ($batch->user_id != $user->id) {
        error("not owner");
    }
    abort_batch($batch);
    echo "<success>1</success>";
}

function handle_retire_batch($r) {
    list($user, $user_submit) = authenticate_user($r, null);
    $batch_id = (int)($r->batch_id);
    $batch = BoincBatch::lookup_id($batch_id);
    if (!$batch) error("no such batch");
    if ($batch->user_id != $user->id) {
        error("not owner");
    }
    retire_batch($batch);
    echo "<success>1</success>";
}

if (0) {
$r = simplexml_load_string("
<query_batch>
    <authenticator>x</authenticator>
    <batch_id>54</batch_id>
</query_batch>
");
query_batch($r);
exit;
}

if (0) {
$r = simplexml_load_string("
<query_job>
    <authenticator>x</authenticator>
    <job_id>312173</job_id>
</query_job>
");
query_job($r);
exit;
}

if (0) {
$r = simplexml_load_string("
<submit_batch>
    <authenticator>x</authenticator>
    <batch>
    <app_name>remote_test</app_name>
    <batch_name>Aug 6 batch 2</batch_name>
    <job>
        <rsc_fpops_est>19000000000</rsc_fpops_est>
        <command_line>--t 19</command_line>
        <input_file>
            <source>http://google.com/</source>
        </input_file>
    </job>
    </batch>
</submit_batch>
");
submit_batch($r);
exit;
}

xml_header();
$r = simplexml_load_string($_POST['request']);

if (!$r) {
    error("can't parse request message");
}

switch ($r->getName()) {
    case 'abort_batch': handle_abort_batch($r); break;
    case 'estimate_batch': estimate_batch($r); break;
    case 'query_batch': query_batch($r); break;
    case 'query_batches': query_batches($r); break;
    case 'query_job': query_job($r); break;
    case 'retire_batch': handle_retire_batch($r); break;
    case 'submit_batch': submit_batch($r); break;
    default: error("bad command");
}

?>

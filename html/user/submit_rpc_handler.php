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

function get_wu($name) {
    $name = BoincDb::escape_string($name);
    $wu = BoincWorkunit::lookup("name='$name'");
    if (!$wu) xml_error(-1, "BOINC server: no job named $name was found");
    return $wu;
}

function get_app($name) {
    $name = BoincDb::escape_string($name);
    $app = BoincApp::lookup("name='$name'");
    if (!$app) xml_error(-1, "BOINC server: no app named $name was found");
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
    $app = get_app((string)($r->batch->app_name));
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
            xml_error(-1, "BOINC server: wrong # of input files for job $i: need $n, got $m");
        }
        $i++;
    }
}

$fanout = parse_config(get_config(), "<uldl_dir_fanout>");

// stage a file, and return the physical name
//
function stage_file($file) {
    global $fanout;

    switch ($file->mode) {
    case "semilocal":
    case "local":
        $md5 = md5_file($file->source);
        if (!$md5) {
            xml_error(-1, "BOINC server: Can't get MD5 of file $file->source");
        }
        $name = "jf_$md5";
        $path = dir_hier_path($name, "../../download", $fanout);
        if (file_exists($path)) return $name;
        if (!copy($file->source, $path)) {
            xml_error(-1, "BOINC server: can't copy file from $file->source to $path");
        }
        return $name;
    case "local_staged":
        return $file->source;
    }
    xml_error(-1, "BOINC server: unsupported file mode: $file->mode");
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

function submit_job($job, $template, $app, $batch_id, $i, $priority) {
    $cmd = "cd ../..; ./bin/create_work --appname $app->name --batch $batch_id --rsc_fpops_est $job->rsc_fpops_est --priority $priority";
    if ($job->command_line) {
        $cmd .= " --command_line \"$job->command_line\"";
    }
    $cmd .= " --wu_name batch_".$batch_id."_".$i;
    foreach ($job->input_files as $file) {
        $cmd .= " $file->name";
    }
    $ret = system($cmd);
    if ($ret === FALSE) {
        xml_error(-1, "BOINC server: can't create job");
    }
}

function xml_get_jobs($r) {
    $jobs = array();
    foreach($r->batch->job as $j) {
        $job = new StdClass;
        $job->input_files = array();
        $job->command_line = (string)$j->command_line;
        $job->rsc_fpops_est = (double)$j->rsc_fpops_est;
        foreach ($j->input_file as $f) {
            $file = new StdClass;
            $file->mode = (string)$f->mode;
            $file->source = (string)$f->source;
            $job->input_files[] = $file;
        }
        $jobs[] = $job;
    }
    return $jobs;
}

function submit_batch($r) {
    $app = get_app((string)($r->batch->app_name));
    list($user, $user_submit) = authenticate_user($r, $app);
    $template = read_input_template($app);
    $jobs = xml_get_jobs($r);
    validate_batch($jobs, $template);
    stage_files($jobs, $template);
    $njobs = count($jobs);
    $now = time();
    $batch_id = (int)($r->batch->batch_id);
    if ($batch_id) {
        $batch = BoincBatch::lookup_id($batch_id);
        if (!$batch) {
            xml_error(-1, "BOINC server: no batch $batch_id");
        }
        if ($batch->user_id != $user->id) {
            xml_error(-1, "BOINC server: not owner of batch");
        }
        if ($batch->state != BATCH_STATE_INIT) {
            xml_error(-1, "BOINC server: batch not in init state");
        }
    }

    // - compute batch FLOP count
    // - run adjust_user_priorities to increment user_submit.logical_start_time
    // - use that for batch logical end time and job priority
    //
    $total_flops = 0;
    foreach($jobs as $job) {
        if ($job->rsc_fpops_est) {
            $total_flops += $job->rsc_fpops_est;
        } else {
            $x = (double) $template->workunit->rsc_fpops_est;
            if ($x) {
                $total_flops += $x;
            } else {
                xml_error(-1, "BOINC server: no rsc_fpops_est given");
            }
        }
    }
    $cmd = "cd ../../bin; ./adjust_user_priority --user $user->id --flops $total_flops --app $app->name";
    $x = exec($cmd);
    if (!is_numeric($x) || (double)$x == 0) {
        xml_error(-1, "BOINC server: $cmd returned $x");
    }
    $let = (double)$x;

    if ($batch_id) {
        $njobs = count($jobs);
        $ret = $batch->update("njobs=$njobs, logical_end_time=$let, state= ".BATCH_STATE_IN_PROGRESS);
        if (!$ret) xml_error(-1, "BOINC server: batch->update() failed");
    } else {
        $batch_name = (string)($r->batch->batch_name);
        $batch_id = BoincBatch::insert(
            "(user_id, create_time, njobs, name, app_id, logical_end_time, state) values ($user->id, $now, $njobs, '$batch_name', $app->id, $let, ".BATCH_STATE_IN_PROGRESS.")"
        );
        if (!$batch_id) {
            xml_error(-1, "BOINC server: Can't create batch: ".mysql_error());
        }
    }
    $i = 0;
    foreach($jobs as $job) {
        submit_job($job, $template, $app, $batch_id, $i++, $let);
    }
    echo "<batch_id>$batch_id</batch_id>\n";
}

function create_batch($r) {
    $app = get_app((string)($r->batch->app_name));
    list($user, $user_submit) = authenticate_user($r, $app);
    $now = time();
    $batch_name = (string)($r->batch->batch_name);
    $batch_id = BoincBatch::insert(
        "(user_id, create_time, name, app_id, state) values ($user->id, $now, '$batch_name', $app->id, ".BATCH_STATE_INIT.")"
    );
    if (!$batch_id) {
        xml_error(-1, "BOINC server: Can't create batch: ".mysql_error());
    }
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

// return a batch specified by the command, using either ID or name
//
function get_batch($r) {
    if (!empty($r->batch_id)) {
        $batch_id = (int)($r->batch_id);
        $batch = BoincBatch::lookup_id($batch_id);
    } else if (!empty($r->batch_name)) {
        $batch_name = (string)($r->batch_name);
        $batch_name = BoincDb::escape_string($batch_name);
        $batch = BoincBatch::lookup_name($batch_name);
    } else {
        xml_error(-1, "BOINC server: batch not specified");
    }
    if (!$batch) xml_error(-1, "BOINC server: no such batch");
    return $batch;
}

function query_batch($r) {
    list($user, $user_submit) = authenticate_user($r, null);
    $batch = get_batch($r);
    if ($batch->user_id != $user->id) {
        xml_error(-1, "BOINC server: not owner of batch");
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

// variant for Condor, which doesn't care about job instances
// and refers to batches by name
//
function query_batch2($r) {
    list($user, $user_submit) = authenticate_user($r, null);
    $batch_names = $r->batch_name;
    $batches = array();
    foreach ($batch_names as $b) {
        $batch_name = (string)$b;
        $batch_name = BoincDb::escape_string($batch_name);
        $batch = BoincBatch::lookup_name($batch_name);
        if (!$batch) {
            xml_error(-1, "no batch named $batch_name");
        }
        if ($batch->user_id != $user->id) {
            xml_error(-1, "not owner of $batch_name");
        }
        $batches[] = $batch;
    }

    echo "<jobs>\n";
    foreach ($batches as $batch) {
        $wus = BoincWorkunit::enum("batch = $batch->id");
        foreach ($wus as $wu) {
            if ($wu->canonical_resultid) {
                $status = "DONE";
            } else if ($wu->error_mask) {
                $status = "ERROR";
            } else {
                $status = "IN_PROGRESS";
            }
            echo
"    <job>
        <job_name>$wu->name</job_name>
        <status>$status</status>
    </job>
";
        }
    }
    echo "</jobs>\n";
}

function query_job($r) {
    list($user, $user_submit) = authenticate_user($r, null);
    $job_id = (int)($r->job_id);
    $wu = BoincWorkunit::lookup_id($job_id);
    if (!$wu) xml_error(-1, "no such job");
    $batch = BoincBatch::lookup_id($wu->batch);
    if ($batch->user_id != $user->id) {
        xml_error(-1, "not owner");
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

// the following for Condor.
// If the job has a canonical instance, return info about it.
// Otherwise find an instance that completed
// (possibly crashed) and return its info.
//
function query_completed_job($r) {
    list($user, $user_submit) = authenticate_user($r, null);
    $job_name = (string)($r->job_name);
    $job_name = BoincDb::escape_string($job_name);
    $wu = BoincWorkunit::lookup("name='$job_name'");
    if (!$wu) xml_error(-1, "no such job");
    $batch = BoincBatch::lookup_id($wu->batch);
    if ($batch->user_id != $user->id) {
        xml_error(-1, "not owner");
    }

    echo "<completed_job>\n";
    echo "   <error_mask>$wu->error_mask</error_mask>\n";
    if ($wu->canonical_resultid) {
        $result = BoincResult::lookup_id($wu->canonical_resultid);
        echo "   <canonical_resultid>$wu->canonical_resultid</canonical_resultid>\n";
    } else {
        $results = BoincResult::enum("workunitid=$job_id");
        foreach ($results as $r) {
            switch($r->outcome) {
            case 1:
            case 3:
            case 6:
                $result = $r;
                break;
            }
        }
        if ($result) {
            echo "   <error_resultid>$result->id</error_resultid>\n";
        }
    }
    if ($result) {
        echo "   <exit_status>$result->exit_status</exit_status>\n";
        echo "   <elapsed_time>$result->elapsed_time</elapsed_time>\n";
        echo "   <cpu_time>$result->cpu_time</cpu_time>\n";
        echo "   <stderr_out><![CDATA[\n";
        echo htmlspecialchars($result->stderr_out);
        echo "   ]]></stderr_out>\n";
    }
    echo "</completed_job>\n";
}

function handle_abort_batch($r) {
    list($user, $user_submit) = authenticate_user($r, null);
    $batch = get_batch($r);
    if ($batch->user_id != $user->id) {
        xml_error(-1, "not owner");
    }
    abort_batch($batch);
    echo "<success>1</success>";
}

function handle_abort_jobs($r) {
    list($user, $user_submit) = authenticate_user($r, null);
    $batch = get_batch($r);
    if ($batch->user_id != $user->id) {
        xml_error(-1, "not owner");
    }
    foreach ($r->job_names as $job_name) {
        $job_name = BoincDb::escape_string($job_name);
        $wu = BoincWorkunit::lookup("name='$job_name'");
        if (!$wu) {
            xml_error(-1, "No job $job_name");
        }
        if ($wu->batch != $batch_id) {
            xml_error(-1, "Not owner of job $job_name");
        }
        abort_workunit($wu);
    }
    echo "<success>1</success>";
}

function handle_retire_batch($r) {
    list($user, $user_submit) = authenticate_user($r, null);
    $batch = get_batch($r);
    if ($batch->user_id != $user->id) {
        xml_error(-1, "not owner");
    }
    retire_batch($batch);
    echo "<success>1</success>";
}

function get_templates($r) {
    $app_name = (string)($r->app_name);
    if ($app_name) {
        $app = get_app($app_name);
    } else {
        $job_name = (string)($r->job_name);
        $wu = get_wu($job_name);
        $app = BoincApp::lookup_id($wu->appid);
    }

    list($user, $user_submit) = authenticate_user($r, $app);
    $in = file_get_contents("../../templates/".$app->name."_in");
    $out = file_get_contents("../../templates/".$app->name."_out");
    if ($in === false || $out === false) {
        xml_error(-1, "template file missing");
    }
    echo "<templates>\n$in\n$out\n</templates>\n";
}

function ping($r) {
    BoincDb::get();     // errors out if DB down or web disabled
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
    <batch_name>Aug 6 batch 4</batch_name>
    <job>
        <rsc_fpops_est>19000000000</rsc_fpops_est>
        <command_line>--t 19</command_line>
        <input_file>
            <mode>remote</mode>
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
    xml_error(-1, "can't parse request message");
}

switch ($r->getName()) {
    case 'abort_batch': handle_abort_batch($r); break;
    case 'abort_jobs': handle_abort_jobs($r); break;
    case 'create_batch': create_batch($r); break;
    case 'estimate_batch': estimate_batch($r); break;
    case 'get_templates': get_templates($r); break;
    case 'ping': ping($r); break;
    case 'query_batch': query_batch($r); break;
    case 'query_batch2': query_batch2($r); break;
    case 'query_batches': query_batches($r); break;
    case 'query_job': query_job($r); break;
    case 'query_completed_job': query_completed_job($r); break;
    case 'retire_batch': handle_retire_batch($r); break;
    case 'submit_batch': submit_batch($r); break;
    default: xml_error(-1, "bad command: ".$r->getName());
}

?>

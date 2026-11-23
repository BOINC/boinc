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

// web interface for submitting BUDA jobs

require_once('../inc/util.inc');
require_once('../inc/submit_util.inc');
require_once('../inc/sandbox.inc');
require_once('../inc/buda.inc');
require_once('../inc/kw_prefs.inc');

define('AVG_CPU_FPOPS', 4.3e9);

display_errors();

function submit_form($user) {
    $app = get_str('app');
    if (!is_valid_filename($app)) {
        error_page('bad app name');
    }
    $app_desc = get_buda_app_desc($app);
    if (!can_submit_to_buda_app($user, $app_desc)) {
        error_page('no permission');
    }

    if (empty(get_buda_variants($app))) {
        error_page(sprintf(
            'You must create an app variant before submitting jobs.
            <p>You can do this <a href=buda.php?action=variant_form&app=%s>here</a>.
            ',
            $app
        ));
    }

    $sbitems_zip = sandbox_select_items($user, '/.zip$/');
    if (!$sbitems_zip) {
        error_page("No .zip files in your sandbox.");
    }

    $desc = "<br><small>
        A zip file with one directory per job.
        Each directory contains the input file(s) for that job
        and an optional file <code>cmdline</code>
        containing command-line arguments.
        <a href=https://github.com/BOINC/boinc/wiki/BUDA-job-submission>Details</a></small>.
    ";
    page_head("BUDA: Submit jobs to $app");

    $us = BoincUserSubmit::lookup_userid($user->id);
    if ($us->max_jobs_in_progress) {
        $n = n_jobs_in_progress($user->id);
        echo sprintf(
            '<p>Note: you are limited to %d jobs in progress,
            and you currently have %d,
            so this batch can be at most %d jobs.</p>',
            $us->max_jobs_in_progress, $n,
            $us->max_jobs_in_progress - $n
        );
    }
    form_start('buda_submit.php');
    echo form_tokens();
    form_input_hidden('action', 'submit');
    form_input_hidden('app', $app);
    form_select("Batch zip file $desc", 'batch_file', $sbitems_zip);
    form_input_text(
        'Command-line arguments<br><small>Passed to all jobs in the batch</small>',
        'cmdline'
    );
    form_input_text(
        'Max job runtime (days) on a typical (4.3 GFLOPS) computer.
            <br><small>
            The runtime limit will be scaled for faster/slower computers.
            <br>
            Jobs that reach this limit will be aborted.
            </small>'
        ,
        'max_runtime_days', 1
    );
    form_input_text(
        'Expected job runtime (days) on a typical (4.3 GFLOPS) computer.
            <br><small>
            This determines how many jobs are sent to each host,
            and how "fraction done" is computed.
            </small>
        ',
        'exp_runtime_days', .5
    );
    form_checkbox(
        "Enable debugging output <br><small>Write Docker commands and output to stderr. Not recommended for long-running jobs.</small>",
        'wrapper_verbose'
    );
    form_submit('OK');
    form_end();
    page_tail();
}

// unzip batch file into a temp dir; return dir name
//
function unzip_batch_file($user, $batch_file) {
    @mkdir("../../buda_batches");
    for ($i=0; $i<1000; $i++) {
        $batch_dir = "../../buda_batches/$i";
        $batch_dir_name = $i;
        $ret = @mkdir($batch_dir);
        if ($ret) break;
    }
    if (!$ret) error_page("can't create batch dir");
    $sb_dir = sandbox_dir($user);
    if (!file_exists("$sb_dir/$batch_file")) {
        error_page("no batch file $batch_file");
    }
    $cmd = sprintf(
        'cd %s; unzip %s > /dev/null',
        $batch_dir,
        escapeshellarg("$sb_dir/$batch_file")
    );
    system($cmd, $ret);
    if ($ret) {
        error_page("unzip error: $ret");
    }
    return $batch_dir_name;
}

// Scan a batch dir.
// Check its validity:
// - optional dir 'shared_input_files' has shared input files
// - other dirs (jobs) can have only remaining infiles and possibly cmdline
//
// Return a structure describing the jobs and files
//

// helper function to handle an input file, shared or not.
// Returns an object with logical name, md5, size
//
function handle_file($dir, $fname) {
    $desc = new StdClass;
    $desc->sticky = false;
    $desc->url = '';
    if (str_ends_with($fname, '.toml')) {
        $desc->log_name = substr($fname, 0, -5);
        foreach (file("$dir/$fname") as $line) {
            $x = explode('=', $line);
            if (count($x) != 2) error_page("bad line $line");
            $a = trim($x[0]);
            $b = str_replace('"', '', trim($x[1]));
            switch ($a) {
            case 'sandbox_name':
                $desc->sandbox_name = $b;
                break;
            case 'sticky':
                $desc->sticky = ($b == 'true');
                break;
            case 'url':
                $desc->url = $b;
                break;
            default:
                error_page("bad line $line");
            }
            if (empty($desc->sandbox_name)) error_page('no sandbox name');
            $user = get_logged_in_user();
            $sbdir = sandbox_dir($user);
            $sbpath = "$sbdir/$desc->sandbox_name";
            if (!file_exists($sbpath)) error_page('no sandbox file');
            [$desc->md5, $desc->size] = sandbox_parse_info_file(
                $user, $desc->sandbox_name
            );
        }
    } else {
        $desc->log_name = $fname;
        [$desc->md5, $desc->size] = get_file_info("$dir/$fname");
    }
    return $desc;
}

function parse_batch_dir($batch_dir, $app_desc) {
    $input_file_names = $app_desc->input_file_names;
    $shared_files_names = [];
    $dir = "$batch_dir/shared_input_files";
    if (is_dir($dir)) {
        foreach (scandir($dir) as $fname) {
            if ($fname[0] == '.') continue;
            $desc = handle_file($dir, $fname);
            if (!in_array($desc->log_name, $input_file_names)) {
                error_page("$fname is not an input file name");
            }
            $shared_file_names[] = $desc->log_name;
            $shared_file_descs[] = $desc;
        }
    }
    $unshared_file_names = array_diff($input_file_names, $shared_file_names);
    $jobs = [];
    foreach (scandir($batch_dir) as $job_name) {
        if ($job_name[0] == '.') continue;
        if ($job_name == 'shared_input_files') continue;
        if (!is_dir("$batch_dir/$job_name")) {
            error_page("$batch_dir/$job_name is not a directory");
        }
        $job_file_names = [];
        $job_file_descs = [];
        $cmdline = '';
        $job_dir = "$batch_dir/$job_name";
        foreach (scandir($job_dir) as $fname) {
            if ($fname[0] == '.') continue;
            if ($fname == 'cmdline') {
                $cmdline = trim(file_get_contents("$job_dir/cmdline"));
                if (!is_valid_filename($cmdline)) {
                    error_page("Invalid cmdline: '$cmdline'");
                }
                continue;
            }
            $desc = handle_file($job_dir, $fname);
            if (!in_array($desc->log_name, $unshared_file_names)) {
                error_page("$fname/$desc->log_name is not an input file name");
            }
            $job_file_names[] = $desc->log_name;
            $job_file_descs[] = $desc;
        }
        if (array_values($job_file_names) != array_values($unshared_file_names)) {
            print_r($job_file_names);
            print_r($unshared_file_names);
            error_page("$job_name doesn't have all input files");
        }

        $job = new StdClass;
        $job->dir = $job_name;
        $job->cmdline = $cmdline;
        $job->file_descs = $job_file_descs;
        $jobs[] = $job;
    }
    $batch_desc = new StdClass;
    $batch_desc->shared_file_descs = $shared_file_descs;
    $batch_desc->jobs = $jobs;
    return $batch_desc;
}

function create_batch($user, $njobs, $app) {
    global $buda_boinc_app;
    $now = time();
    $batch_name = sprintf('buda_%d_%d', $user->id, $now);
    $description = "$app";
    $batch_id = BoincBatch::insert(sprintf(
        "(user_id, create_time, logical_start_time, logical_end_time, est_completion_time, njobs, fraction_done, nerror_jobs, state, completion_time, credit_estimate, credit_canonical, credit_total, name, app_id, project_state, description, expire_time) values (%d, %d, 0, 0, 0, %d, 0, 0, %d, 0, 0, 0, 0, '%s', %d, 0, '%s', 0)",
        $user->id, $now, $njobs, BATCH_STATE_INIT, $batch_name, $buda_boinc_app->id,
        $description
    ));
    return BoincBatch::lookup_id($batch_id);
}

// stage an input file, either from sandbox or batch dir.
// Fill in the physical name in desc.
//
function stage($desc, $dir, $batch_id) {
    $user = get_logged_in_user();
    if (!empty($desc->sandbox_name)) {
        $path = sandbox_path($user, $desc->sandbox_name);
    } else {
        $path = "$dir/$desc->log_name";
    }
    if ($desc->sticky) {
        $phys_name = "jf_$desc->md5";
        $sf = BoincStickyFile::lookup("md5='$desc->md5'");
        if (!$sf) {
            BoincStickyFile::insert(
                sprintf(
                    "(create_time, user_id, md5, size, physical_name, remote_url) values (%d, %d, '%s', %f, '%s', '%s')",
                    time(), $user->id, $desc->md5, $desc->size,
                    $phys_name, $desc->url
                )
            );
            $sf = BoincStickyFile::lookup("md5='$desc->md5'");
            if (!$sf) error_page("can't create sticky file record");
        }
        $ret = BoincStickyFileRef::insert(
            sprintf(
                "(create_time, sticky_file_id, user_id, batch_id, logical_name, sandbox_name) values (%d, %d, %d, %d, '%s', '%s')",
                time(), $sf->id, $user->id, $batch_id,
                $desc->log_name, $desc->sandbox_name
            )
        );
        if (!$ret) error_page("can't create sticky file ref");
    } else {
        $phys_name = sprintf('batch_%d_%s', $batch_id, $desc->md5);
    }
    stage_file_aux($path, $desc->md5, $desc->size, $phys_name);
    $desc->phys_name = $phys_name;
}

// stage the batch's input files:
// first shared files, then per-job files
//
function stage_input_files($batch_dir, $batch_desc, $batch_id) {
    foreach ($batch_desc->shared_file_descs as $desc) {
        stage($desc, "$batch_dir/shared_input_files", $batch_id);
    }
    foreach ($batch_desc->jobs as $job) {
        foreach ($job->file_descs as $desc) {
            stage($desc, "$batch_dir/$job->dir", $batch_id);
        }
    }
}

function lookup_file($log_name, $descs) {
    foreach ($descs as $desc) {
        if ($desc->log_name == $log_name) {
            return $desc;
        }
    }
    return null;
}

// run bin/create_work to create the jobs.
// Use --stdin, where each job is described by a line
//
function create_jobs(
    $user, $app, $app_desc, $batch_desc, $batch_id, $batch_dir_name,
    $wrapper_verbose, $cmdline, $max_fpops, $exp_fpops,
    $keywords
) {
    global $buda_boinc_app;

    // make per-job lines to pass as stdin to create_work
    //
    $job_cmds = '';
    foreach ($batch_desc->jobs as $job) {
        $job_cmd = sprintf('--wu_name batch_%d__job_%s', $batch_id, $job->dir);
        if ($job->cmdline) {
            $job_cmd .= sprintf(
                ' --command_line %s',
                escapeshellarg($job->cmdline)
            );
        }

        // files must be in same order as app desc
        //
        foreach ($app_desc->input_file_names as $log_name) {
            $d = lookup_file($log_name, $batch_desc->shared_file_descs);
            if (!$d) {
                $d = lookup_file($log_name, $job->file_descs);
            }
            if (!$d) {
                error_path("can't find $log_name in file descs");
            }
            if ($d->sticky) {
                if (!empty($d->url)) {
                    $job_cmd .= " --remote_sticky_file $d->url $d->size $d->md5";
                } else {
                    $job_cmd .= " --sticky_file $d->phys_name";
                }
            } else {
                if (!empty($d->url)) {
                    $job_cmd .= " --remote_file $d->url $d->size $d->md5";
                } else {
                    $job_cmd .= " $d->phys_name";
                }
            }
        }
        $job_cmds .= "$job_cmd\n";
    }
    $wrapper_cmdline = sprintf('%s%s',
        $wrapper_verbose?'--verbose ':'',
        $cmdline
    );
    $cmd = sprintf(
        'cd ../..; bin/create_work --appname %s --sub_appname %s --batch %d --stdin --command_line %s --wu_template %s --result_template %s --rsc_fpops_bound %f --rsc_fpops_est %f',
        $buda_boinc_app->name,
        escapeshellarg($app_desc->long_name),
        $batch_id,
        escapeshellarg($wrapper_cmdline),
        escapeshellarg("buda_apps/$app/template_in"),
        escapeshellarg("buda_apps/$app/template_out"),
        $max_fpops, $exp_fpops
    );
    if ($keywords) {
        $cmd .= " --keywords '$keywords'";
    }
    if ($user->seti_id) {
        $cmd .= " --target_user $user->id ";
    }
    $cmd .= sprintf(' > %s 2<&1', "buda_batches/errfile");

    $h = popen($cmd, "w");
    if (!$h) error_page('create_work launch failed');
    fwrite($h, $job_cmds);
    $ret = pclose($h);
    if ($ret) {
        echo "<pre>create_work failed.\n";
        echo "command: $cmd\n\n";
        echo "job lines:\n$job_cmds\n\n";
        echo "error file:\n";
        readfile("../../buda_batches/errfile");
        exit;
    }
}

function handle_submit($user) {
    global $buda_root;

    $app = get_str('app');
    if (!is_valid_filename($app)) {
        error_page('bad app name');
    }
    $app_desc = get_buda_app_desc($app);
    if (!can_submit_to_buda_app($user, $app_desc)) {
        error_page('no permission');
    }

    $batch_file = get_str('batch_file');
    if (!is_valid_filename($batch_file)) {
        error_page('bad batch file name');
    }
    $wrapper_verbose = get_str('wrapper_verbose', true);
    $cmdline = get_str('cmdline');

    $max_runtime_days = get_str('max_runtime_days');
    if (!is_numeric($max_runtime_days)) error_page('bad runtime limit');
    $max_runtime_days = (double)$max_runtime_days;
    if ($max_runtime_days <= 0) error_page('bad runtime limit');
    if ($max_runtime_days > 100) error_page('bad runtime limit');

    $max_fpops = $max_runtime_days * AVG_CPU_FPOPS * 86400;

    $exp_runtime_days = get_str('exp_runtime_days');
    if (!is_numeric($exp_runtime_days)) error_page('bad expected runtime');
    $exp_runtime_days = (double)$exp_runtime_days;
    if ($exp_runtime_days <= 0) error_page('bad expected runtime');
    if ($exp_runtime_days > 100) error_page('bad expected runtime');
    if ($exp_runtime_days > $max_runtime_days) {
        error_page('exp must be < max runtime');
    }
    $exp_fpops = $exp_runtime_days * AVG_CPU_FPOPS * 86400;

    // unzip batch file into temp dir
    $batch_dir_name = unzip_batch_file($user, $batch_file);
    $batch_dir = "../../buda_batches/$batch_dir_name";

    // scan batch dir; validate and return struct
    $batch_desc = parse_batch_dir($batch_dir, $app_desc);

    if (!$batch_desc->jobs) {
        system("rm -rf $batch_dir");
        page_head("No jobs created");
        echo "
            Your batch file (.zip) did not specify any jobs.
            See <a href=https://github.com/BOINC/boinc/wiki/BUDA-job-submission#batch-files>Instructions for creating batch files</a>.
        ";
        page_tail();
        return;
    }
    $njobs = count($batch_desc->jobs);
    if ($njobs > 10 && $user->seti_id) {
        system("rm -rf $batch_dir");
        error_page(
            "Batches with > 10 jobs are not allowed if 'use only my computers' is set"
        );
    }
    $us = BoincUserSubmit::lookup_userid($user->id);
    if ($us->max_jobs_in_progress) {
        $n = n_jobs_in_progress($user->id);
        if ($n + $njobs > $us->max_jobs_in_progress) {
            system("rm -rf $batch_dir");
            error_page(
                sprintf(
                    'This batch is %d jobs, and you already have %d in-progress jobs.
                    This would exceed your limit of %d in-progress jobs.
                    ',
                    $njobs, $n, $us->max_jobs_in_progress
                )
            );
        }
    }

    $batch = create_batch($user, count($batch_desc->jobs), $app);

    // stage input files and record the physical names
    //
    stage_input_files($batch_dir, $batch_desc, $batch->id);

    // get job keywords: user keywords plus BUDA app keywords
    //
    [$yes, $no] = read_kw_prefs($user);
    $keywords = array_merge($yes, $app_desc->sci_kw);
    $keywords = array_unique($keywords);
    $keywords = implode(' ', $keywords);

    create_jobs(
        $user, $app, $app_desc, $batch_desc, $batch->id, $batch_dir_name,
        $wrapper_verbose, $cmdline, $max_fpops, $exp_fpops, $keywords
    );

    // mark batch as in progress
    //
    $batch->update(sprintf('state=%d', BATCH_STATE_IN_PROGRESS));

    // clean up batch dir
    //
    system("rm -rf $batch_dir");

    header("Location: submit.php?action=query_batch&batch_id=$batch->id");
}

function show_app_list($user) {
    page_head('BUDA job submission');
    $apps = get_buda_apps();
    echo 'Select app:<p><br>';
    $found = false;
    foreach ($apps as $app) {
        $desc = get_buda_app_desc($app);
        if (!can_submit_to_buda_app($user, $desc)) {
            continue;
        }
        $found = true;
        echo sprintf('<p><a href=buda_submit.php?action=form&app=%s>%s</a>',
            $app, $desc->long_name
        );
    }
    if (!$found) {
        echo "You don't have submit permissions for any BUDA apps.";
    }
    page_tail();
}

$user = get_logged_in_user();
$buda_boinc_app = BoincApp::lookup("name='buda'");
if (!$buda_boinc_app) error_page('no buda app');
if (!has_submit_access($user, $buda_boinc_app->id)) {
    error_page('no access');
}
$action = get_str('action', true);
if ($action == 'submit') {
    check_tokens();
    handle_submit($user);
} else if ($action == 'form') {
    submit_form($user);
} else {
    show_app_list($user);
}

?>

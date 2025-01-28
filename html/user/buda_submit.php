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

display_errors();

function submit_form($user) {
    $sbitems_zip = sandbox_select_items($user, '/.zip$/');
    if (!$sbitems_zip) {
        error_page("No .zip files in your sandbox.");
    }
    $app = get_str('app');
    if (!is_valid_filename($app)) die('bad arg');
    $variant = get_str('variant');
    if (!is_valid_filename($variant)) die('bad arg');

    $desc = "<br><small>
        A zip file with one directory per job.
        Each directory contains the input file(s) for that job
        and an optional file <code>cmdline</code>
        containing command-line arguments.
        <a href=https://github.com/BOINC/boinc/wiki/BUDA-job-submission>Details</a></small>.
    ";
    page_head("Submit jobs to $app ($variant)");
    form_start('buda_submit.php');
    form_input_hidden('action', 'submit');
    form_input_hidden('app', $app);
    form_input_hidden('variant', $variant);
    form_select("Batch zip file $desc", 'batch_file', $sbitems_zip);
    form_input_text('Command-line arguments', 'cmdline');
    form_checkbox(
        "Enabled debugging output <br><small>Write Docker commands and output to stderr</small>.",
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
    system("cd $batch_dir; unzip $sb_dir/$batch_file > /dev/null", $ret);
    if ($ret) {
        error_page("unzip error: $ret");
    }
    return $batch_dir_name;
}

// Scan a batch dir.
// Check its validity:
// - optional dir 'shared_input_files' has shared input files
// - other dirs (job dirs) can have only remaining infiles and possibly cmdline
//
// Return a structure describing its contents, and the md5/size of files
//
function parse_batch_dir($batch_dir, $variant_desc) {
    $input_files = $variant_desc->input_file_names;
    sort($input_files);
    $shared_files = [];
    $shared_file_infos = [];
    if (is_dir("$batch_dir/shared_input_files")) {
        foreach (scandir("$batch_dir/shared_input_files") as $fname) {
            if ($fname[0] == '.') continue;
            if (!in_array($fname, $input_files)) {
                error_page("$fname is not an input file name");
            }
            $shared_files[] = $fname;
            $shared_file_infos[] = get_file_info("$batch_dir/shared_input_files/$fname");
        }
    }
    $unshared_files = array_diff($input_files, $shared_files);
    sort($unshared_files);
    $jobs = [];
    foreach (scandir($batch_dir) as $fname) {
        if ($fname[0] == '.') continue;
        if ($fname == 'shared_input_files') continue;
        if (!is_dir("$batch_dir/$fname")) {
            error_page("$batch_dir/$fname is not a directory");
        }
        $job_files = [];
        $cmdline = '';
        foreach(scandir("$batch_dir/$fname") as $f2) {
            if ($f2[0] == '.') continue;
            if ($f2 == 'cmdline') {
                $cmdline = trim(file_get_contents("$batch_dir/$fname/cmdline"));
                continue;
            }
            if (!in_array($f2, $unshared_files)) {
                error_page("$fname/$f2 is not an input file name");
            }
            $job_files[] = $f2;
        }
        if (sort($job_files) != $unshared_files) {
            error_page("$fname doesn't have all input files");
        }

        $file_infos = [];
        foreach ($unshared_files as $f2) {
            $file_infos[] = get_file_info("$batch_dir/$fname/$f2");
        }

        $job = new StdClass;
        $job->dir = $fname;
        $job->cmdline = $cmdline;
        $job->file_infos = $file_infos;
        $jobs[] = $job;
    }
    $batch_desc = new StdClass;
    $batch_desc->shared_files = $shared_files;
    $batch_desc->shared_file_infos = $shared_file_infos;
    $batch_desc->unshared_files = $unshared_files;
    $batch_desc->jobs = $jobs;
    return $batch_desc;
}

function create_batch($user, $njobs, $app, $variant) {
    global $buda_boinc_app;
    $now = time();
    $batch_name = sprintf('buda_%d_%d', $user->id, $now);
    $description = "$app ($variant)";
    $batch_id = BoincBatch::insert(sprintf(
        "(user_id, create_time, logical_start_time, logical_end_time, est_completion_time, njobs, fraction_done, nerror_jobs, state, completion_time, credit_estimate, credit_canonical, credit_total, name, app_id, project_state, description, expire_time) values (%d, %d, 0, 0, 0, %d, 0, 0, %d, 0, 0, 0, 0, '%s', %d, 0, '%s', 0)",
        $user->id, $now, $njobs, BATCH_STATE_INIT, $batch_name, $buda_boinc_app->id,
        $description
    ));
    return BoincBatch::lookup_id($batch_id);
}

function stage_input_files($batch_dir, $batch_desc, $batch_id) {
    $n = count($batch_desc->shared_files);
    $batch_desc->shared_files_phys_names = [];
    for ($i=0; $i<$n; $i++) {
        $path = sprintf('%s/%s', $batch_dir, $batch_desc->shared_files[$i]);
        [$md5, $size] = $batch_desc->shared_file_infos[$i];
        $phys_name = sprintf('batch_%d_%s', $batch_id, $md5);
        stage_file_aux($path, $md5, $size, $phys_name);
        $batch_desc->shared_files_phys_names[] = $phys_name;
    }
    foreach ($batch_desc->jobs as $job) {
        $n = count($batch_desc->unshared_files);
        $job->phys_names = [];
        for ($i=0; $i<$n; $i++) {
            $path = sprintf('%s/%s/%s',
                $batch_dir, $job->dir, $batch_desc->unshared_files[$i]
            );
            [$md5, $size] = $job->file_infos[$i];
            $phys_name = sprintf('batch_%d_%s', $batch_id, $md5);
            stage_file_aux($path, $md5, $size, $phys_name);
            $job->phys_names[] = $phys_name;
        }
    }
}

// run bin/create_work to create the jobs.
// Use --stdin, where each job is described by a line
//
function create_jobs(
    $app, $variant, $variant_desc,
    $batch_desc, $batch_id, $batch_dir_name,
    $wrapper_verbose, $cmdline
) {
    global $buda_boinc_app;

    // get list of physical names of app files
    //
    $app_file_names = $variant_desc->dockerfile_phys;
    foreach ($variant_desc->app_files_phys as $pname) {
        $app_file_names .= " $pname";
    }

    // make per-job lines to pass as stdin
    //
    $job_cmds = '';
    foreach ($batch_desc->jobs as $job) {
        $job_cmd = sprintf('--wu_name batch_%d__job_%s', $batch_id, $job->dir);
        if ($job->cmdline) {
            $job_cmd .= sprintf(' --command_line "%s"', $job->cmdline);
        }
        $job_cmd .= " $app_file_names";
        foreach ($batch_desc->shared_files_phys_names as $x) {
            $job_cmd .= " $x";
        }
        foreach ($job->phys_names as $x) {
            $job_cmd .= " $x";
        }
        $job_cmds .= "$job_cmd\n";
    }
    $cw_cmdline = sprintf('"--dockerfile %s %s %s"',
        $variant_desc->dockerfile,
        $wrapper_verbose?'--verbose':'',
        $cmdline
    );
    $cmd = sprintf(
        'cd ../..; bin/create_work --appname %s --batch %d --stdin --command_line %s --wu_template %s --result_template %s',
        $buda_boinc_app->name, $batch_id,
        $cw_cmdline,
        "buda_apps/$app/$variant/template_in",
        "buda_apps/$app/$variant/template_out"
    );
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
    $app = get_str('app');
    if (!is_valid_filename($app)) die('bad arg');
    $variant = get_str('variant');
    if (!is_valid_filename($variant)) die('bad arg');
    $batch_file = get_str('batch_file');
    if (!is_valid_filename($batch_file)) die('bad arg');
    $wrapper_verbose = get_str('wrapper_verbose', true);
    $cmdline = get_str('cmdline');

    $variant_dir = "../../buda_apps/$app/$variant";
    $variant_desc = json_decode(
        file_get_contents("$variant_dir/variant.json")
    );

    // unzip batch file into temp dir
    $batch_dir_name = unzip_batch_file($user, $batch_file);
    $batch_dir = "../../buda_batches/$batch_dir_name";

    // scan batch dir; validate and return struct
    $batch_desc = parse_batch_dir($batch_dir, $variant_desc);

    $batch = create_batch(
        $user, count($batch_desc->jobs), $app, $variant
    );

    // stage input files and record the physical names
    //
    stage_input_files($batch_dir, $batch_desc, $batch->id);

    create_jobs(
        $app, $variant, $variant_desc,
        $batch_desc, $batch->id, $batch_dir_name,
        $wrapper_verbose, $cmdline
    );

    // mark batch as in progress
    //
    $batch->update(sprintf('state=%d', BATCH_STATE_IN_PROGRESS));

    // clean up batch dir
    //
    //system("rm -rf $batch_dir");

    header("Location: submit.php?action=query_batch&batch_id=$batch->id");
}

$user = get_logged_in_user();
$buda_boinc_app = BoincApp::lookup("name='buda'");
if (!$buda_boinc_app) error_page('no buda app');
if (!has_submit_access($user, $buda_boinc_app->id)) {
    error_page('no access');
}
$action = get_str('action', true);
if ($action == 'submit') {
    handle_submit($user);
} else {
    submit_form($user);
}

?>

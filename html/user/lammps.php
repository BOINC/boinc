<?php

error_reporting(E_ALL);
ini_set('display_errors', true);
ini_set('display_startup_errors', true);

require_once("../inc/util.inc");
require_once("../inc/submit_db.inc");
require_once("../inc/sandbox.inc");

function show_submit_form() {
    page_head("Submit LAMMPS jobs");
    echo "
        <form action=lammps.php>
        <input type=hidden name=action value=prepare>
    ";
    start_table();
    row2("Structure file", "<input name=structure_file>");
    row2("Command file", "<input name=command_file>");
    row2("Command-line file", "<input name=cmdline_file>");
    row2("", "<input type=submit value=Prepare>");
    end_table();
    echo "</form>";
    page_tail();
}

// verify that an input file exists in sandbox, and return its physical path
//
function get_file_path($user, $name) {
    $fname = get_str($name);

    // verify that the files exist in sandbox
    //
    $sbdir = sandbox_dir($user);
    list($error, $size, $md5) = sandbox_parse_link_file("$sbdir/$fname");
    if ($error) error_page("no $name file");

    return sandbox_physical_path($user, $md5);
}

function project_flops() {
    $x = BoincUser::sum("expavg_credit");
    if ($x == 0) $x = 200;
    $y = 1e9*$x/200;
    return $y;
}

function prepare_batch($user) {
    $structure_file_path = get_file_path($user, 'structure_file');
    $command_file_path = get_file_path($user, 'command_file');
    $cmdline_file_path = get_file_path($user, 'cmdline_file');

    $info = null;
    $info->structure_file_path = $structure_file_path;
    $info->command_file_path = $command_file_path;
    $info->cmdline_file_path = $cmdline_file_path;

    // get FLOPS per job
    // TODO
    //
    $info->rsc_fpops_est = 1e12;

    // get disk space per job
    // TODO
    $info->rsc_disk_bound = 1e8;

    $tmpfile = tempnam("/tmp", "lammps_");
    file_put_contents($tmpfile, serialize($info));

    // get the # of jobs
    //
    $njobs = count(file($cmdline_file_path));
    $total_flops = $njobs * $info->rsc_fpops_est;
    $secs_est = $total_flops/project_flops();
    $hrs_est = number_format($secs_est/3600, 1);
    $client_mb = $info->rsc_disk_bound/1e6;
    $server_mb = $njobs*$client_mb;

    page_head("Batch prepared");
    echo "
        Your batch has $njobs jobs.
        <p>
        Estimated time to completion: $hrs_est hours.
        <p>
        Estimated client disk usage: $client_mb MB
        <p>
        Estimated server disk usage: $server_mb MB
        <p>
    ";
    show_button("lammps.php?action=submit&tmpfile=$tmpfile", "Submit Batch");
    page_tail();
}

function submit_job($app, $batch_id, $rsc_fpops_est, $cmdline, $i) {
    $cmd = "cd ../..; ./bin/create_work --appname $app->name --batch $batch_id --rsc_fpops_est $rsc_fpops_est";
    if ($cmdline) {
        $cmd .= " --command_line \"$cmdline\"";
    }
    $cmd .= " --wu_name batch_".$batch_id."_".$i;
    foreach ($job->input_files as $file) {
        $cmd .= " $file->name";
    }
    $ret = system($cmd);
    if ($ret === FALSE) {
        error_page("can't create job");
    }
}

function submit_batch($user, $app) {
    $tmpfile = get_str('tmpfile');
    $x = file_get_contents("/tmp/$tmpfile");
    $info = deserialize($x);

    $cmdlines = file($info->cmdline_file_path);
    $njobs = count($cmdlines);

    $now = time();
    $batch_name = time_str($now);
    $batch_id = BoincBatch::insert(
        "(user_id, create_time, njobs, name, app_id) values ($user->id, $now, $njobs, '$batch_name', $app->id)"
    );

    $i = 0;
    foreach ($cmdlines as $cmdline) {
        submit_job($app, $batch_id, $info->rsc_fpops_est, $cmdline, $i);
        $i++;
    }
}

$user = get_logged_in_user();
$user_submit = BoincUserSubmit::lookup_userid($user->id);
if (!$user_submit) error_page("no submit access");
$app = BoincApp::lookup("name='fake_cuda'");
if (!$app) error_page("no lammps app");

if (!$user_submit->submit_all) {
    $usa = BoincUserSubmitApp::lookup("user_id=$user->id and app_id=$app->id");
    if (!$usa) {
        error_page("no submit access");
    }
}

$action = get_str('action', true);
switch ($action) {
case '': show_submit_form(); break;
case 'prepare': prepare_batch($user); break;
case 'submit': submit_batch($user, $app); break;
default: error_page("no such action $action");
}
?>

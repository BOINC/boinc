#!/usr/local/bin/php -q
<?php {
    // $Id$

    // This tests whether the most basic mechanisms are working
    // Also whether stderr output is reported correctly
    // Also tests if water levels are working correctly

    include_once("test.inc");

    echo "-- Testing standard upper_case application ------------------------------------\n";

    $project = new Project;
    $project->add_core_and_version();
    $project->add_app_and_version("upper_case");

    $user = new User();
    $user->project_prefs = "<project_specific>\nfoobar\n</project_specific>\n";
    $user->global_prefs = "<venue name=\"home\">\n".
    "<work_buf_min_days>0</work_buf_min_days>\n".
    "<work_buf_max_days>2</work_buf_max_days>\n".
    "<disk_interval>1</disk_interval>\n".
    "<run_on_batteries/>\n".
    "<max_bytes_sec_down>400000</max_bytes_sec_down>\n".
    "</venue>\n";

    $project->add_user($user);
    $project->install();      // must install projects before adding to hosts
    $project->install_feeder();

    $host = new Host();
    $host->add_user($user, $project);
    $host->install();

    $work = new Work();
    $work->wu_template = "uc_wu";
    $work->result_template = "uc_result";
    $work->redundancy = 2;
    $work->delay_bound = 10;
    // Say that 1 WU takes 1 day on a ref comp
    $work->rsc_fpops = 86400*1e9/2;
    $work->rsc_iops = 86400*1e9/2;
    $work->rsc_disk = 10e8;
    array_push($work->input_files, "input");
    $work->install($project);

    $project->start_servers();
    $host->run("-exit_when_idle -skip_cpu_benchmarks");

    $project->validate(2);
    $result->server_state = RESULT_STATE_OVER;
    $result->stderr_out = "APP: upper_case: starting, argc 1";
    $result->exit_status = 0;
    $project->check_results(2, $result);
    $project->compare_file("uc_wu_0_0", "uc_correct_output");
    $project->compare_file("uc_wu_1_0", "uc_correct_output");

    $project->assimilate();
    $project->file_delete();

    $project->check_server_deleted("download/input");
    $project->check_server_deleted("upload/uc_wu_0_0");
    $project->check_server_deleted("upload/uc_wu_1_0");
    $project->stop();

    test_done();
} ?>

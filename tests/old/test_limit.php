#! /usr/local/bin/php
<?php
    // This tests whether the most basic mechanisms are working
    // Also whether stderr output is reported correctly
    // Also tests if water levels are working correctly

    include_once("test.inc");

    $retval = 0;

    $project = new Project;

    $app = new App("upper_case");
    $app_version = new App_Version($app);

    // the following is optional (makes client web download possible)
    $core_app = new App("core client");
    $core_app_version = new App_Version($core_app);
    $project->add_app($core_app);
    $project->add_app_version($core_app_version);

    $project->add_app($app);
    $project->add_app_version($app_version);

    $user = new User();
    $user->project_prefs = "<project_specific>\nfoobar\n</project_specific>\n";
    $user->global_prefs = "<venue name=\"home\">\n".
    "<work_buf_min_days>0</work_buf_min_days>\n".
    "<work_buf_max_days>2</work_buf_max_days>\n".
    "<run_on_batteries/>\n".
    "<max_bytes_sec_down>400000</max_bytes_sec_down>\n".
    "</venue>\n";

    $project->add_user($user);
    $project->install();      // must install projects before adding to hosts
    $project->install_feeder();

    $host = new Host();
    $host->log_flags = "log_flags.xml";
    $host->add_user($user, $project);
    $host->install();

    echo "adding work\n";

    $work = new Work($app);
    $work->wu_template = "ucs_wu";
    $work->result_template = "uc_result";
    $work->redundancy = 1;
    $work->delay_bound = 2;
    // Say that 1 WU takes 1 day on a ref comp
    $work->rsc_fpops = 86400*1e9/2;
    $work->rsc_iops = 86400*1e9/2;
    $work->rsc_memory = 1024*1024;
    $work->rsc_disk = 1;
    array_push($work->input_files, "input");
    $work->install($project);

    $project->start_servers();
    sleep(1);       // make sure feeder has a chance to run
    $host->run("-exit_when_idle -skip_cpu_benchmarks");

    $result->server_state = RESULT_SERVER_STATE_OVER;
    $result->stderr_out = "APP: upper_case: starting, argc 1";
    $result->exit_status = 0;
    $project->check_results(1, $result);

    $project->stop();

    exit($retval);
?>

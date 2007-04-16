#! /usr/local/bin/php
<?php
    // test whether CPU time is computed correctly across restarts

    include_once("test.inc");

    $project = new Project;
    $user = new User();
    $host = new Host($user);
    $app = new App("uc_cpu");
    $app_version = new App_Version($app);

    $project->add_user($user);
    $project->add_app($app);
    $project->add_app_version($app_version);
    $project->install();      // must install projects before adding to hosts

    $host->log_flags = "log_flags.xml";
    $host->add_project($project);
    $host->install();

    echo "adding work\n";

    $work = new Work($app);
    $work->wu_template = "uccpu_wu";
    $work->result_template = "uccpu_result";
    $work->redundancy = 1;
    array_push($work->input_files, "small_input");
    $work->install($project);

    $project->start_feeder();
    $app_time = 0;
    $host->run("-exit_after_app_start 400");
    $app_time += $host->read_cpu_time_file("app.time");
    $host->run("-exit_when_idle");
    $project->stop();

    $result->server_state = RESULT_SERVER_STATE_OVER;
    $project->check_results(1, $result);
    $project->compare_file("uccpu_wu_0_0", "uc_small_correct_output");
    $client_time = $host->read_cpu_time_file("client_time");
    $x = mysql_query("select cpu_time from result where name='uccpu_wu_0'");
    $result = mysql_fetch_object($x);
    $db_time = $result->cpu_time;

    if (abs($app_time-$client_time) > .01) echo "time mismatch\n";
    if (abs($app_time-$db_time) > .01) echo "time mismatch\n";

?>
